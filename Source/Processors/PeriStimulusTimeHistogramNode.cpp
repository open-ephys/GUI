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
#include "Visualization/SpikeDisplayCanvas.h"
#include "Channel.h"

#include <stdio.h>


PeriStimulusTimeHistogramNode::PeriStimulusTimeHistogramNode()
    : GenericProcessor("PSTH"), displayBufferSize(5),  redrawRequested(false), isRecording(false),
	  signalFilesShouldClose(false)
{
 

    spikeBuffer = new uint8_t[MAX_SPIKE_BUFFER_LEN]; // MAX_SPIKE_BUFFER_LEN defined in SpikeObject.h

}

PeriStimulusTimeHistogramNode::~PeriStimulusTimeHistogramNode()
{
    
}

AudioProcessorEditor* PeriStimulusTimeHistogramNode::createEditor()
{
    std::cout<<"Creating SpikeDisplayCanvas."<<std::endl;

    editor = new SpikeDisplayEditor(this);
    return editor;

}

void PeriStimulusTimeHistogramNode::updateSettings()
{
    //std::cout << "Setting num inputs on PeriStimulusTimeHistogramNode to " << getNumInputs() << std::endl;

    electrodes.clear();

    for (int i = 0; i < eventChannels.size(); i++)
    {
        if ((eventChannels[i]->eventType < 999) && (eventChannels[i]->eventType > SPIKE_BASE_CODE))
        {

            Electrode elec;
            elec.numChannels = eventChannels[i]->eventType - 100;
            elec.name = eventChannels[i]->name;
            elec.currentSpikeIndex = 0;
            elec.mostRecentSpikes.ensureStorageAllocated(displayBufferSize);

            for (int j = 0; j < elec.numChannels; j++)
            {
                elec.displayThresholds.add(0);
                elec.detectorThresholds.add(0);
            }
            
            electrodes.add(elec);
        }
    }

    recordNode = getProcessorGraph()->getRecordNode();
    diskWriteLock = recordNode->getLock();

}



bool PeriStimulusTimeHistogramNode::enable()
{
    std::cout << "PeriStimulusTimeHistogramNode::enable()" << std::endl;
    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
    editor->enable();
    return true;

}

bool PeriStimulusTimeHistogramNode::disable()
{
    std::cout << "PeriStimulusTimeHistogramNode disabled!" << std::endl;
    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
    editor->disable();
    return true;
}

int PeriStimulusTimeHistogramNode::getNumberOfChannelsForElectrode(int i)
{
    if (i > -1 && i < electrodes.size())
    {
        return electrodes[i].numChannels;
    } else {
        return 0;
    }
}

String PeriStimulusTimeHistogramNode::getNameForElectrode(int i)
{

    if (i > -1 && i < electrodes.size())
    {
        return electrodes[i].name;
    } else {
        return " ";
    }
}


int PeriStimulusTimeHistogramNode::getNumElectrodes()
{
    return electrodes.size();

}

void PeriStimulusTimeHistogramNode::startRecording()
{
    
    setParameter(1, 0.0f); // need to use the 'setParameter' method to interact with 'process'
}

void PeriStimulusTimeHistogramNode::stopRecording()
{
    setParameter(0, 0.0f); // need to use the 'setParameter' method to interact with 'process'
}


void PeriStimulusTimeHistogramNode::setParameter(int param, float val)
{
    //std::cout<<"PeriStimulusTimeHistogramNode got Param:"<< param<< " with value:"<<val<<std::endl;

    if (param == 0) // stop recording
    {
        isRecording = false;

    } else if (param == 1) // start recording
    {
        isRecording = true;
    } else if (param == 2) // redraw
    {
        redrawRequested = true;
    }

}



void PeriStimulusTimeHistogramNode::process(AudioSampleBuffer& buffer, MidiBuffer& events, int& nSamples)
{
	// Update internal statistics 
    checkForEvents(events); // automatically calls 'handleEvent
	
	// draw the PSTH
    if (redrawRequested)
    {
        redrawRequested = false;
    }

}

void PeriStimulusTimeHistogramNode::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{

    //std::cout << "Received event of type " << eventType << std::endl;

    if (eventType == SPIKE)
    {

        const uint8_t* dataptr = event.getRawData();
        int bufferSize = event.getRawDataSize();
            
        if (bufferSize > 0)
        {

            SpikeObject newSpike;

            bool isValid = unpackSpike(&newSpike, dataptr, bufferSize);

            if (isValid)
            {
                int electrodeNum = newSpike.source;

                Electrode& e = electrodes.getReference(electrodeNum);
               // std::cout << electrodeNum << std::endl;

             
               

            }
        
        }

    }

}
