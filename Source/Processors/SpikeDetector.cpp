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

#include <stdio.h>
#include "SpikeDetector.h"

#include "Channel.h"

SpikeDetector::SpikeDetector()
    : GenericProcessor("Spike Detector"),
      overflowBuffer(2,100), dataBuffer(nullptr),
      overflowBufferSize(100), currentElectrode(-1)
{
    //// the standard form:
    electrodeTypes.add("single electrode");
    electrodeTypes.add("stereotrode");
    electrodeTypes.add("tetrode");

    //// the technically correct form (Greek cardinal prefixes):
    // electrodeTypes.add("hentrode");
    // electrodeTypes.add("duotrode");
    // electrodeTypes.add("triode");
    // electrodeTypes.add("tetrode");
    // electrodeTypes.add("pentrode");
    // electrodeTypes.add("hextrode");
    // electrodeTypes.add("heptrode");
    // electrodeTypes.add("octrode");
    // electrodeTypes.add("enneatrode");
    // electrodeTypes.add("decatrode");
    // electrodeTypes.add("hendecatrode");
    // electrodeTypes.add("dodecatrode");
    // electrodeTypes.add("triskaidecatrode");
    // electrodeTypes.add("tetrakaidecatrode");
    // electrodeTypes.add("pentakaidecatrode");
    // electrodeTypes.add("hexadecatrode");
    // electrodeTypes.add("heptakaidecatrode");
    // electrodeTypes.add("octakaidecatrode");
    // electrodeTypes.add("enneakaidecatrode");
    // electrodeTypes.add("icosatrode");

    for (int i = 0; i < electrodeTypes.size()+1; i++)
    {
        electrodeCounter.add(0);
    }

    spikeBuffer = new uint8_t[MAX_SPIKE_BUFFER_LEN]; // MAX_SPIKE_BUFFER_LEN defined in SpikeObject.h

}

SpikeDetector::~SpikeDetector()
{

}


AudioProcessorEditor* SpikeDetector::createEditor()
{
    editor = new SpikeDetectorEditor(this, true);
    return editor;
}

void SpikeDetector::updateSettings()
{

    if (getNumInputs() > 0)
        overflowBuffer.setSize(getNumInputs(), overflowBufferSize);

    for (int i = 0; i < electrodes.size(); i++)
    {

        Channel* ch = new Channel(this, i);
        ch->isEventChannel = true;
        ch->eventType = SPIKE_BASE_CODE + electrodes[i]->numChannels;
        ch->name = electrodes[i]->name;

        eventChannels.add(ch);
    }

}

bool SpikeDetector::addElectrode(int nChans)
{

    std::cout << "Adding electrode with " << nChans << " channels." << std::endl;

    int firstChan;

    if (electrodes.size() == 0)
    {
        firstChan = 0;
    }
    else
    {
        Electrode* e = electrodes.getLast();
        firstChan = *(e->channels+(e->numChannels-1))+1;
    }

    if (firstChan + nChans > getNumInputs())
    {
        firstChan = 0; // make sure we don't overflow available channels
    }

    int currentVal = electrodeCounter[nChans];
    electrodeCounter.set(nChans,++currentVal);

    String electrodeName;

    // hard-coded for tetrode configuration
    if (nChans < 3)
        electrodeName = electrodeTypes[nChans-1];
    else
        electrodeName = electrodeTypes[nChans-2];

    String newName = electrodeName.substring(0,1);
    newName = newName.toUpperCase();
    electrodeName = electrodeName.substring(1,electrodeName.length());
    newName += electrodeName;
    newName += " ";
    newName += electrodeCounter[nChans];

    Electrode* newElectrode = new Electrode();

    newElectrode->name = newName;
    newElectrode->numChannels = nChans;
    newElectrode->prePeakSamples = 8;
    newElectrode->postPeakSamples = 32;
    newElectrode->thresholds = new double[nChans];
    newElectrode->isActive = new bool[nChans];
    newElectrode->channels = new int[nChans];
    newElectrode->isMonitored = false;

    for (int i = 0; i < nChans; i++)
    {
        *(newElectrode->channels+i) = firstChan+i;
        *(newElectrode->thresholds+i) = getDefaultThreshold();
        *(newElectrode->isActive+i) = true;
    }

    resetElectrode(newElectrode);

    electrodes.add(newElectrode);

    currentElectrode = electrodes.size()-1;

    return true;

}

float SpikeDetector::getDefaultThreshold()
{
    return 50.0f;
}

StringArray SpikeDetector::getElectrodeNames()
{
    StringArray names;

    for (int i = 0; i < electrodes.size(); i++)
    {
        names.add(electrodes[i]->name);
    }

    return names;
}

void SpikeDetector::resetElectrode(Electrode* e)
{
    e->lastBufferIndex = 0;
}

bool SpikeDetector::removeElectrode(int index)
{

    // std::cout << "Spike detector removing electrode" << std::endl;

    if (index > electrodes.size() || index < 0)
        return false;

    electrodes.remove(index);
    return true;
}

void SpikeDetector::setElectrodeName(int index, String newName)
{
    electrodes[index-1]->name = newName;
}

void SpikeDetector::setChannel(int electrodeIndex, int channelNum, int newChannel)
{

    std::cout << "Setting electrode " << electrodeIndex << " channel " << channelNum <<
              " to " << newChannel << std::endl;

    *(electrodes[electrodeIndex]->channels+channelNum) = newChannel;
}

int SpikeDetector::getNumChannels(int index)
{

    if (index < electrodes.size())
        return electrodes[index]->numChannels;
    else
        return 0;
}

int SpikeDetector::getChannel(int index, int i)
{
    return *(electrodes[index]->channels+i);
}

Array<Electrode*> SpikeDetector::getElectrodes()
{
    return electrodes;
}

Electrode* SpikeDetector::setCurrentElectrodeIndex(int i)
{
    jassert(i >= 0 & i < electrodes.size());
    currentElectrode = i;
    return electrodes[i];
}

Electrode* SpikeDetector::getActiveElectrode()
{
    if (electrodes.size() == 0)
      return nullptr;

    return electrodes[currentElectrode];
}

void SpikeDetector::setChannelActive(int electrodeIndex, int subChannel, bool active)
{


    currentElectrode = electrodeIndex;
    currentChannelIndex = subChannel;

    std::cout << "Setting channel active to " << active << std::endl;

    if (active)
        setParameter(98, 1);
    else
        setParameter(98, 0);

}

bool SpikeDetector::isChannelActive(int electrodeIndex, int i)
{
    return *(electrodes[electrodeIndex]->isActive+i);
}


void SpikeDetector::setChannelThreshold(int electrodeNum, int channelNum, float thresh)
{
    currentElectrode = electrodeNum;
    currentChannelIndex = channelNum;
    std::cout << "Setting electrode " << electrodeNum << " channel threshold " << channelNum << " to " << thresh << std::endl;
    setParameter(99, thresh);
}

double SpikeDetector::getChannelThreshold(int electrodeNum, int channelNum)
{
    return *(electrodes[electrodeNum]->thresholds+channelNum);
}

void SpikeDetector::setParameter(int parameterIndex, float newValue)
{
    //editor->updateParameterButtons(parameterIndex);

    if (parameterIndex == 99 && currentElectrode > -1)
    {
        *(electrodes[currentElectrode]->thresholds+currentChannelIndex) = newValue;
    }
    else if (parameterIndex == 98 && currentElectrode > -1)
    {
        if (newValue == 0.0f)
            *(electrodes[currentElectrode]->isActive+currentChannelIndex) = false;
        else
            *(electrodes[currentElectrode]->isActive+currentChannelIndex) = true;
    }
}


bool SpikeDetector::enable()
{

    useOverflowBuffer = false;
    return true;
}

bool SpikeDetector::disable()
{

    for (int n = 0; n < electrodes.size(); n++)
    {
        resetElectrode(electrodes[n]);
    }

    return true;
}

void SpikeDetector::addSpikeEvent(SpikeObject* s, MidiBuffer& eventBuffer, int peakIndex)
{

    // std::cout << "Adding spike event for index " << peakIndex << std::endl;

    s->eventType = SPIKE_EVENT_CODE;

    int numBytes = packSpike(s,                        // SpikeObject
                             spikeBuffer,              // uint8_t*
                             MAX_SPIKE_BUFFER_LEN);    // int

    if (numBytes > 0)
        eventBuffer.addEvent(spikeBuffer, numBytes, peakIndex);
    
    //std::cout << "Adding spike" << std::endl;
}

void SpikeDetector::addWaveformToSpikeObject(SpikeObject* s,
                                             int& peakIndex,
                                             int& electrodeNumber,
                                             int& currentChannel)
{
    int spikeLength = electrodes[electrodeNumber]->prePeakSamples +
                      + electrodes[electrodeNumber]->postPeakSamples;

    s->timestamp = timestamp + peakIndex;

    s->nSamples = spikeLength;

    int chan = *(electrodes[electrodeNumber]->channels+currentChannel);

    s->gain[currentChannel] = (int)(1.0f / channels[chan]->bitVolts)*1000;
    s->threshold[currentChannel] = (int) *(electrodes[electrodeNumber]->thresholds+currentChannel); // / channels[chan]->bitVolts * 1000;

    // cycle through buffer

    if (isChannelActive(electrodeNumber, currentChannel))
    {

        for (int sample = 0; sample < spikeLength; sample++)
        { 

            // warning -- be careful of bitvolts conversion
            s->data[currentIndex] = uint16(getNextSample(*(electrodes[electrodeNumber]->channels+currentChannel)) / channels[chan]->bitVolts + 32768);

            currentIndex++;
            sampleIndex++;

            //std::cout << currentIndex << std::endl;

        }
    }
    else
    {
        for (int sample = 0; sample < spikeLength; sample++)
        {

            // insert a blank spike if the
            s->data[currentIndex] = 0;
            currentIndex++;
            sampleIndex++;

            //std::cout << currentIndex << std::endl;

        }
    }


    sampleIndex -= spikeLength; // reset sample index


}

void SpikeDetector::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{

    if (eventType == TIMESTAMP)
    {
        const uint8* dataptr = event.getRawData();

        memcpy(&timestamp, dataptr + 4, 8); // remember to skip first four bytes
    }


}

void SpikeDetector::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events,
                            int& nSamples)
{

    // cycle through electrodes
    Electrode* electrode;
    dataBuffer = &buffer;

    checkForEvents(events); // need to find any timestamp events before extracting spikes

    //std::cout << dataBuffer.getMagnitude(0,nSamples) << std::endl;

    for (int i = 0; i < electrodes.size(); i++)
    {

        //  std::cout << "ELECTRODE " << i << std::endl;

        electrode = electrodes[i];

        // refresh buffer index for this electrode
        sampleIndex = electrode->lastBufferIndex - 1; // subtract 1 to account for
        // increment at start of getNextSample()

        // cycle through samples
        while (samplesAvailable(nSamples))
        {

            sampleIndex++;


            // cycle through channels
            for (int chan = 0; chan < electrode->numChannels; chan++)
            {

                // std::cout << "  channel " << chan << std::endl;

                if (*(electrode->isActive+chan))
                {

                    int currentChannel = *(electrode->channels+chan);

                    if (-getNextSample(currentChannel) > *(electrode->thresholds+chan)) // trigger spike
                    {

                        //std::cout << "Spike detected on electrode " << i << std::endl;
                        // find the peak
                        int peakIndex = sampleIndex;

                        while (-getCurrentSample(currentChannel) <
                               -getNextSample(currentChannel) &&
                               sampleIndex < peakIndex + electrode->postPeakSamples)
                        {
                            sampleIndex++;
                        }

                        peakIndex = sampleIndex;
                        sampleIndex -= (electrode->prePeakSamples+1);

                        SpikeObject newSpike;
                        newSpike.timestamp = peakIndex;
                        newSpike.source = i;
                        newSpike.nChannels = electrode->numChannels;

                        currentIndex = 0;

                        // package spikes;
                        for (int channel = 0; channel < electrode->numChannels; channel++)
                        {

                            addWaveformToSpikeObject(&newSpike,
                                                     peakIndex,
                                                     i,
                                                     channel);

                            // if (*(electrode->isActive+currentChannel))
                            // {

                            //     createSpikeEvent(peakIndex,       // peak index
                            //                      i,               // electrodeNumber
                            //                      currentChannel,  // channel number
                            //                      events);         // event buffer


                            // } // end if channel is active

                        }

                       //for (int xxx = 0; xxx < 1000; xxx++) // overload with spikes for testing purposes
                            addSpikeEvent(&newSpike, events, peakIndex);

                        // advance the sample index
                        sampleIndex = peakIndex + electrode->postPeakSamples;

                        break; // quit spike "for" loop
                    } // end spike trigger

                } // end if channel is active
            } // end cycle through channels on electrode

        } // end cycle through samples

        electrode->lastBufferIndex = sampleIndex - nSamples; // should be negative

        //jassert(electrode->lastBufferIndex < 0);

    } // end cycle through electrodes

    // copy end of this buffer into the overflow buffer

    //std::cout << "Copying buffer" << std::endl;
    // std::cout << "nSamples: " << nSamples;
    //std::cout << "overflowBufferSize:" << overflowBufferSize;

    //std::cout << "sourceStartSample = " << nSamples-overflowBufferSize << std::endl;
    // std::cout << "numSamples = " << overflowBufferSize << std::endl;
    // std::cout << "buffer size = " << buffer.getNumSamples() << std::endl;

    if (nSamples > overflowBufferSize)
    {

        for (int i = 0; i < overflowBuffer.getNumChannels(); i++)
        {

            overflowBuffer.copyFrom(i, 0,
                                    buffer, i,
                                    nSamples-overflowBufferSize,
                                    overflowBufferSize);

            useOverflowBuffer = true;
        }

    }
    else
    {

        useOverflowBuffer = false;
    }



}

float SpikeDetector::getNextSample(int& chan)
{



    //if (useOverflowBuffer)
    //{
    if (sampleIndex < 0)
    {
        // std::cout << "  sample index " << sampleIndex << "from overflowBuffer" << std::endl;
        int ind = overflowBufferSize + sampleIndex;

        if (ind < overflowBuffer.getNumSamples())
            return *overflowBuffer.getWritePointer(chan, ind);
        else
            return 0;

    }
    else
    {
        //  useOverflowBuffer = false;
        // std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;

        if (sampleIndex < dataBuffer->getNumSamples())
            return *dataBuffer->getWritePointer(chan, sampleIndex);
        else
            return 0;
    }
    //} else {
    //    std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;
    //     return *dataBuffer.getWritePointer(chan, sampleIndex);
    //}

}

float SpikeDetector::getCurrentSample(int& chan)
{

    // if (useOverflowBuffer)
    // {
    //     return *overflowBuffer.getWritePointer(chan, overflowBufferSize + sampleIndex - 1);
    // } else {
    //     return *dataBuffer.getWritePointer(chan, sampleIndex - 1);
    // }

    if (sampleIndex < 1)
    {
        //std::cout << "  sample index " << sampleIndex << "from overflowBuffer" << std::endl;
        return *overflowBuffer.getWritePointer(chan, overflowBufferSize + sampleIndex - 1);
    }
    else
    {
        //  useOverflowBuffer = false;
        // std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;
        return *dataBuffer->getWritePointer(chan, sampleIndex - 1);
    }
    //} else {

}


bool SpikeDetector::samplesAvailable(int& nSamples)
{

    if (sampleIndex > nSamples - overflowBufferSize/2)
    {
        return false;
    }
    else
    {
        return true;
    }

}


void SpikeDetector::saveCustomParametersToXml(XmlElement* parentElement)
{

    for (int i = 0; i < electrodes.size(); i++)
    {
        XmlElement* electrodeNode = parentElement->createNewChildElement("ELECTRODE");
        electrodeNode->setAttribute("name", electrodes[i]->name);
        electrodeNode->setAttribute("numChannels", electrodes[i]->numChannels);
        electrodeNode->setAttribute("prePeakSamples", electrodes[i]->prePeakSamples);
        electrodeNode->setAttribute("postPeakSamples", electrodes[i]->postPeakSamples);

        for (int j = 0; j < electrodes[i]->numChannels; j++)
        {
            XmlElement* channelNode = electrodeNode->createNewChildElement("SUBCHANNEL");
            channelNode->setAttribute("ch",*(electrodes[i]->channels+j));
            channelNode->setAttribute("thresh",*(electrodes[i]->thresholds+j));
            channelNode->setAttribute("isActive",*(electrodes[i]->isActive+j));

        }
    }


}

void SpikeDetector::loadCustomParametersFromXml()
{

   
    if (parametersAsXml != nullptr) // prevent double-loading
    {
        // use parametersAsXml to restore state

        SpikeDetectorEditor* sde = (SpikeDetectorEditor*) getEditor();

        int electrodeIndex = -1;

        forEachXmlChildElement(*parametersAsXml, xmlNode)
        {
            if (xmlNode->hasTagName("ELECTRODE"))
            {

                electrodeIndex++;

                std::cout << "ELECTRODE>>>" << std::endl;

                int channelsPerElectrode = xmlNode->getIntAttribute("numChannels");

                sde->addElectrode(channelsPerElectrode);

                setElectrodeName(electrodeIndex+1, xmlNode->getStringAttribute("name"));
                sde->refreshElectrodeList();

                int channelIndex = -1;

                forEachXmlChildElement(*xmlNode, channelNode)
                {
                    if (channelNode->hasTagName("SUBCHANNEL"))
                    {
                        channelIndex++;

                        std::cout << "Subchannel " << channelIndex << std::endl;

                        setChannel(electrodeIndex, channelIndex, channelNode->getIntAttribute("ch"));
                        setChannelThreshold(electrodeIndex, channelIndex, channelNode->getDoubleAttribute("thresh"));
                        setChannelActive(electrodeIndex, channelIndex, channelNode->getBoolAttribute("isActive"));
                    }
               }


            }
        }

        sde->checkSettings();
    }

}
