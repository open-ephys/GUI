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

#include "PeriStimulusTimeHistogramNode.h"
#include "RecordNode.h"
#include "Editors/PeriStimulusTimeHistogramEditor.h"
#include "Channel.h"

#include <stdio.h>


PeriStimulusTimeHistogramNode::PeriStimulusTimeHistogramNode()
    : GenericProcessor("PSTH"), displayBufferSize(5),  redrawRequested(false)
{
	trialCircularBuffer  = nullptr;
	
	saveTTLs = saveNetworkEvents = true;
	spikeSavingMode = 2;
}

PeriStimulusTimeHistogramNode::~PeriStimulusTimeHistogramNode()
{
    
}

AudioProcessorEditor* PeriStimulusTimeHistogramNode::createEditor()
{
    editor = new PeriStimulusTimeHistogramEditor(this,true);
    return editor;

}

void PeriStimulusTimeHistogramNode::updateSettings()
{
	recordNode = getProcessorGraph()->getRecordNode();
    diskWriteLock = recordNode->getLock();
}



bool PeriStimulusTimeHistogramNode::enable()
{
	
    std::cout << "PeriStimulusTimeHistogramNode::enable()" << std::endl;
	PeriStimulusTimeHistogramEditor* editor = (PeriStimulusTimeHistogramEditor*) getEditor();
    editor->enable();
		
    return true;

}

bool PeriStimulusTimeHistogramNode::disable()
{
	
    std::cout << "PeriStimulusTimeHistogramNode disabled!" << std::endl;
    PeriStimulusTimeHistogramEditor* editor = (PeriStimulusTimeHistogramEditor*) getEditor();
    editor->disable();
	
    return true;
}

void PeriStimulusTimeHistogramNode::toggleConditionVisibility(int cond)
{
	if (trialCircularBuffer  != nullptr)
	{
		trialCircularBuffer->toggleConditionVisibility(cond);
	}
}

void PeriStimulusTimeHistogramNode::process(AudioSampleBuffer& buffer, MidiBuffer& events, int& nSamples)
{
	if (trialCircularBuffer  == nullptr)
	{
		trialCircularBuffer = new TrialCircularBuffer(getNumInputChannels(),getSampleRate(),this);//
	}
	//trialCircularBuffer->reallocate(getNumInputChannels());

	// Update internal statistics 
    checkForEvents(events); 
	
	trialCircularBuffer->process(buffer,nSamples,hardware_timestamp,software_timestamp);
	// draw the PSTH
    if (redrawRequested)
    {
        redrawRequested = false;
    }

}

StringTS PeriStimulusTimeHistogramNode::unpackStringTS(MidiMessage &event) 
{
      const uint8* dataptr = event.getRawData();
		int bufferSize = event.getRawDataSize();
		int string_length = bufferSize-4-8; // -4 for initial event prefix, -8 for timestamp at the end
		int64 timestamp;
		memcpy(&timestamp, dataptr + 4+string_length, 8); // remember to skip first four bytes
		return StringTS((unsigned char *)dataptr+4,string_length,timestamp);
}


void PeriStimulusTimeHistogramNode::dumpEventToDisk(MidiMessage& event)
{
	diskWriteLock->enter();
	diskWriteLock->exit();
}

void PeriStimulusTimeHistogramNode::dumpSpikeEventToDisk(SpikeObject *s, bool dumpWave)
{
	diskWriteLock->enter();
	diskWriteLock->exit();
}

void PeriStimulusTimeHistogramNode::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{
    //std::cout << "Received event of type " << eventType << std::endl;
	if (eventType == NETWORK)
	{
		StringTS s = unpackStringTS(event);
  		trialCircularBuffer->parseMessage(s);
		if (isRecording && saveNetworkEvents)
		{
			   dumpEventToDisk(event);
		}

	}
	if (eventType == TIMESTAMP)
    {
          const uint8* dataptr = event.getRawData();
	      memcpy(&hardware_timestamp, dataptr + 4, 8); // remember to skip first four bytes
		  memcpy(&software_timestamp, dataptr + 12, 8); // remember to skip first four bytes
    } 
	if (eventType == TTL)
	{
		   const uint8* dataptr = event.getRawData();
		   bool ttl_raise = dataptr[2] > 0;
		   int channel = dataptr[3];
		   int64  ttl_timestamp_software,ttl_timestamp_hardware;
		   memcpy(&ttl_timestamp_software, dataptr+4, 8);
		   memcpy(&ttl_timestamp_hardware, dataptr+4, 8);
		   if (ttl_raise)
				trialCircularBuffer->addTTLevent(channel,ttl_timestamp_software);
		   if (isRecording && saveTTLs)
			   dumpEventToDisk(event);
	}

    if (eventType == SPIKE)
    {
        const uint8_t* dataptr = event.getRawData();
        int bufferSize = event.getRawDataSize();
        if (bufferSize > 0)
        {
            SpikeObject newSpike;
            unpackSpike(&newSpike, dataptr, bufferSize);
			if (newSpike.sortedId > 0) { // drop unsorted spikes
				trialCircularBuffer->addSpikeToSpikeBuffer(newSpike);
			}
			if (isRecording)
			{
				if  (spikeSavingMode == 1 && newSpike.sortedId > 0)
					dumpSpikeEventToDisk(&newSpike, false);
				else if (spikeSavingMode == 2 && newSpike.sortedId > 0)
					dumpSpikeEventToDisk(&newSpike, true);
				else if (spikeSavingMode == 3)
					dumpSpikeEventToDisk(&newSpike, true);
			}
        }
    }
}


void PeriStimulusTimeHistogramNode::startRecording()
{
	if (!isRecording)
	{
		isRecording = true;
        File dataDirectory = recordNode->getDataDirectory();

        if (dataDirectory.getFullPathName().length() == 0)
        {
            // temporary fix in case nothing is returned by the record node.
            dataDirectory = File::getSpecialLocation(File::userHomeDirectory); 
        }

        String baseDirectory = dataDirectory.getFullPathName();

/*        for (int i = 0; i < getNumElectrodes(); i++)
        {
            openFile(i);
        }*/
	}
}

void PeriStimulusTimeHistogramNode::stopRecording()
{
	if (isRecording)
	{
		// close files, etc.
		isRecording = false;
	}
}
