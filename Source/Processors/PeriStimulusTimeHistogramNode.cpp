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
}



bool PeriStimulusTimeHistogramNode::enable()
{
	/*
    std::cout << "PeriStimulusTimeHistogramNode::enable()" << std::endl;
    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
    editor->enable();
		*/
    return true;

}

bool PeriStimulusTimeHistogramNode::disable()
{
	/*
    std::cout << "PeriStimulusTimeHistogramNode disabled!" << std::endl;
    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
    editor->disable();
	*/
    return true;
}



void PeriStimulusTimeHistogramNode::process(AudioSampleBuffer& buffer, MidiBuffer& events, int& nSamples)
{
	if (trialCircularBuffer  == nullptr)
	{
		trialCircularBuffer = new TrialCircularBuffer(getNumInputChannels(),getSampleRate(),this);//
	}
	//trialCircularBuffer->reallocate(getNumInputChannels());

	// Update internal statistics 
    checkForEvents(events); // automatically calls 'handleEvent
	
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


void PeriStimulusTimeHistogramNode::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{

    //std::cout << "Received event of type " << eventType << std::endl;
	if (eventType == NETWORK)
	{
		StringTS s = unpackStringTS(event);
  		trialCircularBuffer->parseMessage(s);
	}
	if (eventType == TIMESTAMP)
    {
          const uint8* dataptr = event.getRawData();
	      memcpy(&hardware_timestamp, dataptr + 4, 8); // remember to skip first four bytes
		  memcpy(&software_timestamp, dataptr + 12, 8); // remember to skip first four bytes
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
        }
    }
}


