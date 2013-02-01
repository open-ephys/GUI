/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "SourceNode.h"
#include "Editors/SourceNodeEditor.h"
#include "Channel.h"
#include <stdio.h>

SourceNode::SourceNode(const String& name_)
	: GenericProcessor(name_),
	  dataThread(0), inputBuffer(0),
	  sourceCheckInterval(2000), wasDisabled(true), ttlState(0)
{

	std::cout << "creating source node." << std::endl;

	if (getName().equalsIgnoreCase("Intan Demo Board")) {
		dataThread = new IntanThread(this);
	} else if (getName().equalsIgnoreCase("Custom FPGA")) {
		dataThread = new FPGAThread(this);//FPGAThread(this);
	} else if (getName().equalsIgnoreCase("File Reader")) {
		dataThread = new FileReaderThread(this);
	}

	if (dataThread != 0)
	{
		if (!dataThread->foundInputSource())
		{
			enabledState(false);
		}

		numEventChannels = dataThread->getNumEventChannels();
		eventChannelState = new int[numEventChannels];
		for (int i = 0; i < numEventChannels; i++)
		{
			eventChannelState[i] = 0;
		}

	} else {
		enabledState(false);
		numEventChannels = 0;
	}

	// check for input source every few seconds
	startTimer(sourceCheckInterval); 

	timestamp = 0; 
	eventCodeBuffer = new int16[10000]; //10000 samples per buffer max?


}

SourceNode::~SourceNode() 
{
}

DataThread* SourceNode::getThread()
{
    return dataThread;
}

void SourceNode::updateSettings()
{
	if (inputBuffer == 0 && dataThread != 0)
	{

		inputBuffer = dataThread->getBufferAddress();
		std::cout << "Input buffer address is " << inputBuffer << std::endl;
	}

	for (int i = 0; i < dataThread->getNumEventChannels(); i++)
	{
		Channel* ch = new Channel(this, i);
		ch->eventType = TTL;
		ch->isEventChannel = true;
		eventChannels.add(ch);
	}

}

void SourceNode::actionListenerCallback(const String& msg)
{
    
    //std::cout << msg << std::endl;
    
    if (msg.equalsIgnoreCase("HI"))
    {
       // std::cout << "HI." << std::endl;
       // dataThread->setOutputHigh();
        ttlState = 1;
    } else if (msg.equalsIgnoreCase("LO"))
    {
       // std::cout << "LO." << std::endl;
       // dataThread->setOutputLow();
        ttlState = 0;
    }
}

int SourceNode::getTTLState()
{
    return ttlState;
}

float SourceNode::getSampleRate()
{

	if (dataThread != 0)
		return dataThread->getSampleRate();
	else
		return 44100.0;
}

float SourceNode::getDefaultSampleRate()
{
	if (dataThread != 0)
		return dataThread->getSampleRate();
	else
		return 44100.0;
}

int SourceNode::getDefaultNumOutputs()
{
	if (dataThread != 0)
		return dataThread->getNumChannels();
	else
		return 0;
}

float SourceNode::getDefaultBitVolts()
{
	if (dataThread != 0)
		return dataThread->getBitVolts();
	else
		return 1.0f;
}

void SourceNode::enabledState(bool t)
{
	if (t && !dataThread->foundInputSource())
	{
		isEnabled = false;
	} else {
		isEnabled = t;
	}

}

void SourceNode::setParameter (int parameterIndex, float newValue)
{
	//std::cout << "Got parameter change notification";
}

AudioProcessorEditor* SourceNode::createEditor()
{
	editor = new SourceNodeEditor(this, true);
	return editor;
}

void SourceNode::timerCallback()
{
	if (dataThread->foundInputSource())
	{
		if (!isEnabled) {
			std::cout << "Input source found." << std::endl;
			//stopTimer(); // check for input source every two seconds
			enabledState(true);
			GenericEditor* ed = getEditor();
			getEditorViewport()->makeEditorVisible(ed);
		}
	} else {
		if (isEnabled) {
			std::cout << "No input source found." << std::endl;
			enabledState(false);
			GenericEditor* ed = getEditor();
			getEditorViewport()->makeEditorVisible(ed);
		}
	}
}

bool SourceNode::isReady() {
	
	if (dataThread != 0) {
		return dataThread->foundInputSource();
	} else {
		return false;
	}
}

bool SourceNode::enable() {
	
	std::cout << "Source node received enable signal" << std::endl;

	wasDisabled = false;

	if (dataThread != 0)
	{
		dataThread->startAcquisition();
		return true;
	} else {
		return false;
	}

	stopTimer();

}

bool SourceNode::disable() {

	std::cout << "Source node received disable signal" << std::endl;

	if (dataThread != 0)
		dataThread->stopAcquisition();
	
	startTimer(2000);

	wasDisabled = true;

	std::cout << "SourceNode returning true." << std::endl;

	return true;
}

void SourceNode::acquisitionStopped()
{
	//if (!dataThread->foundInputSource()) {
		
		if (!wasDisabled) {
			std::cout << "Source node sending signal to UI." << std::endl;
			getUIComponent()->disableCallbacks();
			enabledState(false);
			GenericEditor* ed = (GenericEditor*) getEditor();
			getEditorViewport()->makeEditorVisible(ed);
		}
	//}
}


void SourceNode::process(AudioSampleBuffer &buffer, 
                            MidiBuffer &events,
                            int& nSamples)
{
	
	//std::cout << "SOURCE NODE" << std::endl;

	// clear the input buffers
	events.clear();
	buffer.clear();

	nSamples = inputBuffer->readAllFromBuffer(buffer, &timestamp, eventCodeBuffer, buffer.getNumSamples());
	
	 //std::cout << "TIMESTAMP: " << timestamp << std::endl;
    
    //std::cout << "Samples per buffer: " << nSamples << std::endl;

	uint8 data[4];
	memcpy(data, &timestamp, 4);

	 // generate timestamp
	 addEvent(events,    // MidiBuffer
	 		  TIMESTAMP, // eventType
	 		  0,         // sampleNum
	 		  nodeId,    // eventID
	 		  0,		 // eventChannel
	 		  4,         // numBytes
	 		  data   // data
	 		 );

	 // fill event buffer
	 for (int i = 0; i < nSamples; i++)
	 {
	 	for (int c = 0; c < numEventChannels; c++)
	 	{
	 		int state = eventCodeBuffer[i] & (1 << c);

	 		if (eventChannelState[c] != state)
	 		{
	 			if (state == 0) {

                    //std::cout << "OFF" << std::endl;
                    //std::cout << c << std::endl;
	 				// signal channel state is OFF
	 				addEvent(events, // MidiBuffer
	 						 TTL,    // eventType
	 						 i,      // sampleNum
	 						 0,	     // eventID
	 						 c		 // eventChannel
	 						 );
	 			} else {

                   // std::cout << "ON" << std::endl;
                   // std::cout << c << std::endl;
                    
	 				// signal channel state is ON
	 				addEvent(events, // MidiBuffer
	 						 TTL,    // eventType
	 						 i,      // sampleNum
	 						 1,		 // eventID
	 						 c		 // eventChannel
	 						 );
	 			

	 			}

	 			eventChannelState[c] = state;
	 		}
	 	}
	 }

}



