/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "OscilloscopeNode.h"
#include "RecordNode.h"
#include "Editors/OscilloscopeEditor.h"
#include "Channel.h"
#include "TrialCircularBuffer.h"
#include <stdio.h>


OscilloscopeNode::OscilloscopeNode() : GenericProcessor("Oscilloscope")
{
	trialCircularBuffer  = nullptr;
	triggerByTTL = true;
	triggerChannel = 0;
	triggerType = AUTO;
	prevValue = 0;
	triggerThresholduV = 50;
	numTTLchannels = 8;
	totalSamplesProcessed = 0;
	Time t;
	numTicksPerSec = t.getHighResolutionTicksPerSecond();
	fft = false;
}




/************************************************
* FFT code from the book Numerical Recipes in C *
* Visit www.nr.com for the licence.             *
************************************************/

// The following line must be defined before including math.h to correctly define M_PI
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


int OscilloscopeNode::getTriggerChannel()
{
	return triggerChannel;
}

void OscilloscopeNode::setFrequencyAnalyzerMode(bool state)
{
	fft = state;
}


bool OscilloscopeNode::getFrequencyAnalyzerMode()
{
	return fft;
}
bool OscilloscopeNode::isTTLtrigger()
{
	return triggerByTTL;
}


void OscilloscopeNode::setTriggerChannel(int channel, bool TTL)
{
	triggerChannel = channel;
	triggerByTTL = TTL;
}

void OscilloscopeNode::setTriggerThreshold(double thres)
{
	triggerThresholduV = thres;
}


double OscilloscopeNode::getTriggerThreshold()
{
	return triggerThresholduV;
}

void OscilloscopeNode::setTriggerType(TriggerTypes t)
{
	triggerType = t;
}

TriggerTypes OscilloscopeNode::getTriggerType()
{
	return triggerType;
}

void OscilloscopeNode::saveCustomParametersToXml(XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("OSCILLOSCOPE");
}



void OscilloscopeNode::loadCustomParametersFromXml()
{
	if (parametersAsXml != nullptr)
	{
		forEachXmlChildElement(*parametersAsXml, mainNode)
		{
			if (mainNode->hasTagName("OSCILLOSCOPE"))
			{
			}
		}
	}
}

OscilloscopeNode::~OscilloscopeNode()
{
    
}

AudioProcessorEditor* OscilloscopeNode::createEditor()
{
    editor = new OscilloscopeEditor(this,true);
    return editor;

}

void OscilloscopeNode::updateSettings()
{
	delete trialCircularBuffer;
	trialCircularBuffer = nullptr;
	if (trialCircularBuffer  == nullptr && getSampleRate() > 0 && getNumInputs() > 0)
	{

		allocateTrialCircularBuffer();

	}
}

bool OscilloscopeNode::enable()
{
    std::cout << "OscilloscopeNode::enable()" << std::endl;
	OscilloscopeEditor* editor = (OscilloscopeEditor*) getEditor();
    editor->enable();
    return true;
}

bool OscilloscopeNode::disable()
{
	
    std::cout << "OscilloscopeNode disabled!" << std::endl;
    OscilloscopeEditor* editor = (OscilloscopeEditor*) getEditor();
    editor->disable();
    return true;
}

void OscilloscopeNode::allocateTrialCircularBuffer()
{
		params.numChannels = getNumInputs();
		params.numTTLchannels = numTTLchannels;
		params.sampleRate = getSampleRate();
		params.preSec = 0.5;
		params.postSec = 0.5;
		params.maxTrialTimeSeconds = 0; 
		params.maxTrialsInMemory = 1;
		params.binResolutionMS = 1.0/params.sampleRate * 1e3;
		params.desiredSamplingRateHz = params.sampleRate;
		params.ttlSupressionTimeSec = params.postSec;
		params.ttlTrialLengthSec = 0;
		params.autoAddTTLconditions = false;
		params.buildTrialsPSTH = false;
		params.reconstructTTL = true;
		params.approximate = true;
		totalSamplesProcessed = 0;
		trialCircularBuffer = new TrialCircularBuffer(params);	

		int numCh = getNumInputs();
		int *dummyChannels = new int [numCh];

		for (int k=0;k<numCh;k++)
			dummyChannels[k]=k;

		Electrode *e = new Electrode(0,nullptr, "Dummy",numCh, dummyChannels, 0, 8, 32, getSampleRate());
		// build a single condition
		trialCircularBuffer->parseMessage(StringTS("addcondition name trigger trialtypes 1 visible 1"));
		trialCircularBuffer->addNewElectrode(e);

}

void OscilloscopeNode::process(AudioSampleBuffer& buffer, MidiBuffer& events, int& nSamples)
{
    checkForEvents(events); 

	if (trialCircularBuffer  == nullptr && getSampleRate() > 0 && getNumInputs() > 0)  
	{
		allocateTrialCircularBuffer();
	} else if (trialCircularBuffer != nullptr) {
		trialCircularBuffer->process(buffer,nSamples,hardware_timestamp,software_timestamp);
		totalSamplesProcessed+=nSamples;
		// add the first auto trigger only after we made sure we have enough data to display the "prev"
		if (triggerType == AUTO && totalSamplesProcessed > 2*params.preSec * params.sampleRate)
			{
				trialCircularBuffer->simulateHardwareTrial(software_timestamp, hardware_timestamp,1, 0);
			} else

		if (!triggerByTTL && (triggerType == RISING || triggerType == FALLING))
		{
			// check if the selected channel cross threshold
			// if so, simulate a trial.
			for (int k=0; k<nSamples;k++)
			{
				float currentValue = *(buffer.getSampleData(triggerChannel,k));
				// don't worry about values crossing the threshold multiple times. Trial Circular Buffer will drop those simulated trials
				// until postSec passed.
				
				if (triggerType == RISING)
				{
					if (prevValue < triggerThresholduV && currentValue >= triggerThresholduV)
					{
						trialCircularBuffer->simulateHardwareTrial(software_timestamp + float(k)/getSampleRate() * numTicksPerSec, hardware_timestamp+k,1, 0);
					}
				} else if (triggerType == FALLING)
				{
					if (prevValue > triggerThresholduV && currentValue <= triggerThresholduV)
					{
						trialCircularBuffer->simulateHardwareTrial(software_timestamp + float(k)/getSampleRate() * numTicksPerSec, hardware_timestamp+k,1, 0);
					}
				} 
				prevValue = currentValue;
			}
		}
	}
}

void OscilloscopeNode::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{
	if (eventType == TIMESTAMP)
    {
          const uint8* dataptr = event.getRawData();
	      memcpy(&hardware_timestamp, dataptr + 4, 8); // remember to skip first four bytes
		  memcpy(&software_timestamp, dataptr + 12, 8); // remember to skip first four bytes
    } 
	if (eventType == TTL)
	{
		   const uint8* dataptr = event.getRawData();
		   bool ttl_rise = dataptr[2] > 0;
		   int channel = dataptr[3];
		   int64  ttl_timestamp_software,ttl_timestamp_hardware;
		   memcpy(&ttl_timestamp_software, dataptr+4, 8);
		   memcpy(&ttl_timestamp_hardware, dataptr+12, 8);
		   // Tri
		   bool triggerTrial = triggerByTTL && triggerChannel == channel;
 		   trialCircularBuffer->addTTLevent(channel,ttl_timestamp_software,ttl_timestamp_hardware, ttl_rise, false);
		   if (triggerTrial)
		   {
			   trialCircularBuffer->simulateHardwareTrial(ttl_timestamp_software, ttl_timestamp_hardware,1, 0);
		   }
	}
}


