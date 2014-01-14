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
#include "SpikeSortBoxes.h"
#include "Visualization/SpikeDetectCanvas.h"
#include "Channel.h"
#include "SpikeDisplayNode.h"
#include "PeriStimulusTimeHistogramNode.h"
class spikeSorter;

SpikeDetector::SpikeDetector()
    : GenericProcessor("Spike Detector"),
      overflowBuffer(2,100), dataBuffer(overflowBuffer),
      overflowBufferSize(100), currentElectrode(-1),
	  numPreSamples(8),numPostSamples(32)
{
    //// the standard form:
	uniqueID = 0;
    //// the technically correct form (Greek cardinal prefixes):

	juce::Time timer;
	ticksPerSec = (float) timer.getHighResolutionTicksPerSecond();
	electrodeTypes.clear();
	electrodeCounter.clear();
    spikeBuffer = new uint8_t[MAX_SPIKE_BUFFER_LEN]; // MAX_SPIKE_BUFFER_LEN defined in SpikeObject.h
	channelBuffers=nullptr;
	PCAbeforeBoxes = true;
	autoDACassignment = true;
	syncThresholds = false;
}

int SpikeDetector::getNumPreSamples()
{
	return numPreSamples;
}

int SpikeDetector::getNumPostSamples()
{
	return numPostSamples;
}

bool SpikeDetector::getAutoDacAssignmentStatus()
{
	return autoDACassignment;
}

bool SpikeDetector::getThresholdSyncStatus()
{
	return syncThresholds;
}

void SpikeDetector::setThresholdSyncStatus(bool status)
{
	syncThresholds= status;
}


void SpikeDetector::seteAutoDacAssignment(bool status)
{
	autoDACassignment = status;
}

void SpikeDetector::setNumPreSamples(int numSamples)
{
	// we need to update all electrodes, and also inform other modules that this has happened....
	numPreSamples = numSamples;

	for (int k = 0; k < electrodes.size(); k++)
	{
		electrodes[k]->resizeWaveform(numPreSamples,numPostSamples);
	}
	
}

void SpikeDetector::setNumPostSamples(int numSamples)
{
	numPostSamples = numSamples;
	for (int k = 0; k < electrodes.size(); k++)
	{
		electrodes[k]->resizeWaveform(numPreSamples,numPostSamples);
	}
}


int SpikeDetector::getUniqueProbeID(String type)
{
    for (int i = 0; i < electrodeTypes.size(); i++)
    {
        if (electrodeTypes[i] == type)
		{
			return electrodeCounter[i];
		}
    }
	// if we reached here, we didn't find the type. Add it.
	electrodeTypes.push_back(type);
	electrodeCounter.push_back(1);
	return 1;
}


void SpikeDetector::increaseUniqueProbeID(String type)
{
    for (int i = 0; i < electrodeTypes.size(); i++)
    {
        if (electrodeTypes[i] == type)
		{
			electrodeCounter[i]++;
		}
    }
}



SpikeDetector::~SpikeDetector()
{
	delete spikeBuffer;
	spikeBuffer = nullptr;

	if (channelBuffers != nullptr)
		delete channelBuffers;

}



AudioProcessorEditor* SpikeDetector::createEditor()
{
    editor = new SpikeDetectorEditor(this, true);
	
    return editor;
}

void SpikeDetector::updateSettings()
{
	 
	mut.enter();
	int numChannels = getNumInputs();
    if (numChannels > 0)
        overflowBuffer.setSize(getNumInputs(), overflowBufferSize);

	if (channelBuffers != nullptr)
		delete channelBuffers;

	double SamplingRate = getSampleRate();;
	double ContinuousBufferLengthSec = 5;
	channelBuffers = new ContinuousCircularBuffer(numChannels,SamplingRate,1, ContinuousBufferLengthSec);
	 
	
	/** this code was used to sync SpikeDisplayNode  */
	
	/*
    for (int i = 0; i < electrodes.size(); i++)
    {

        Channel* ch = new Channel(this, i);
        ch->isEventChannel = true;
        ch->eventType = SPIKE_BASE_CODE + electrodes[i]->numChannels;
        ch->name = electrodes[i]->name;
        eventChannels.add(ch);
    } */

	// instead, we pass things now in this new form:
	for (int k = 0; k < electrodes.size(); k++)
	{
		String eventlog = "NewElectrode "+String(electrodes[k]->electrodeID) + " "+String(electrodes[k]->numChannels)+" ";
		for (int j=0;j<electrodes[k]->numChannels;j++)
			eventlog += String(electrodes[k]->channels[j])+ " "+electrodes[k]->name;

		addNetworkEventToQueue(StringTS(eventlog));
	}
	

	
	mut.exit();
}

/*
Electrode::Electrode(PCAcomputingThread *pth)
{
	spikePlot = nullptr;
	computingThread = pth;
}*/

Electrode::~Electrode()
{
	delete thresholds;
    delete isActive;

    delete channels;
	delete spikeSort;
}

Electrode::Electrode(int ID, PCAcomputingThread *pth, String _name, int _numChannels, int *_channels, float default_threshold, int pre, int post, float samplingRate )
{
	electrodeID = ID;
		computingThread = pth;

	name = _name;

    numChannels = _numChannels;
    prePeakSamples = pre;
    postPeakSamples = post;

    thresholds = new double[numChannels];
    isActive = new bool[numChannels];
    channels = new int[numChannels];
	depthOffsetMM = 0.0;

	advancerID = -1;

    for (int i = 0; i < numChannels; i++)
    {
        channels[i] = _channels[i];
		thresholds[i] = default_threshold;
		isActive[i] = true;
    }
	spikePlot = nullptr;
	spikeSort = new SpikeSortBoxes(computingThread,numChannels, samplingRate, pre+post);

    isMonitored = false;
}

void Electrode::resizeWaveform(int numPre, int numPost)
{
	// update electrode and all sorted units....
	// we can't keep pca space anymore, so we discard of all pca units (?)
    prePeakSamples = numPre;
    postPeakSamples = numPost;

	//spikePlot = nullptr;
	spikeSort->resizeWaveform(prePeakSamples+postPeakSamples);

}

void SpikeDetector::setElectrodeAdvancerOffset(int i, double v)
{
	mut.enter();
	if (i >= 0) 
	{
		electrodes[i]->depthOffsetMM = v;
		addNetworkEventToQueue(StringTS("NewElectrodeDepthOffset "+String(electrodes[i]->electrodeID)+" "+String(v,4)));
	}
	mut.exit();
}

void SpikeDetector::setElectrodeAdvancer(int i, int ID)
{
	mut.enter();
	if (i >= 0) 
	{
		electrodes[i]->advancerID = ID;
	}
	mut.exit();
}

void SpikeDetector::addNewUnit(int electrodeID, int newUnitID, uint8 r, uint8 g, uint8 b)
{
	String eventlog = "NewUnit "+String(electrodeID) + " "+String(newUnitID)+" "+String(r)+" "+String(g)+" "+String(b);
	addNetworkEventToQueue(StringTS(eventlog));
	updateSinks( electrodeID,  newUnitID, r,g,b,true);
}

void SpikeDetector::removeUnit(int electrodeID, int unitID)
{
	String eventlog = "RemoveUnit "+String(electrodeID) + " "+String(unitID);
	addNetworkEventToQueue(StringTS(eventlog));
	updateSinks( electrodeID,  unitID, 0,0,0,false);
	
}

RHD2000Thread* SpikeDetector::getRhythmAccess()
{

	ProcessorGraph *gr = getProcessorGraph();
	Array<GenericProcessor*> p = gr->getListOfProcessors();
	for (int k=0;k<p.size();k++)
	{
		if (p[k]->getName() == "Rhythm FPGA")
		{
			SourceNode* src = (SourceNode* )p[k];
			return (RHD2000Thread*)src->getThread();
		}
	}
	return nullptr;
}

void SpikeDetector::updateDACthreshold(int dacChannel, float threshold)
{
	RHD2000Thread* th = getRhythmAccess();
	if (th != nullptr)
	{
		th->setDACthreshold(dacChannel,threshold);
	}
}

Array<int> SpikeDetector::getDACassignments()
{
	Array<int> dacChannels ;
	RHD2000Thread* th = getRhythmAccess();
	if (th != nullptr)
	{
		dacChannels = th->getDACchannels();
	}
	return dacChannels;
}

int SpikeDetector::getDACassignment(int dacchannel)
{
	RHD2000Thread* th = getRhythmAccess();
	if (th != nullptr)
	{
		Array<int> dacChannels = th->getDACchannels();
		return dacChannels[dacchannel];
	}
	
	return -1; // not assigned
}

void SpikeDetector::assignDACtoChannel(int dacOutput, int channel)
{
	// inform sinks about a new unit
	//getSourceNode()
	RHD2000Thread* th = getRhythmAccess();
	if (th != nullptr)
	{
		th->setDACchannel(dacOutput, channel);
	}
}


void SpikeDetector::updateSinks(int electrodeID, int unitID, uint8 r, uint8 g, uint8 b, bool addRemove)
{
	// inform sinks about a new unit
	ProcessorGraph *gr = getProcessorGraph();
	Array<GenericProcessor*> p = gr->getListOfProcessors();
	for (int k=0;k<p.size();k++)
	{
		if (p[k]->getName() == "PSTH")
		{
			PeriStimulusTimeHistogramNode *node = (PeriStimulusTimeHistogramNode*)p[k];
			if (node->trialCircularBuffer != nullptr)
			{
				if (addRemove) 
				{
					// add electrode
					node->trialCircularBuffer->addNewUnit(electrodeID,unitID,r,g,b);
				} else
				{
					// remove electrode
					node->trialCircularBuffer->removeUnit(electrodeID,unitID);
				}
			}
		}
	}
}

void SpikeDetector::updateSinks(int electrodeID, int channelindex, int newchannel)
{
	// inform sinks about a channel change
	ProcessorGraph *g = getProcessorGraph();
	Array<GenericProcessor*> p = g->getListOfProcessors();
	for (int k=0;k<p.size();k++)
	{
		if (p[k]->getName() == "PSTH")
		{
			PeriStimulusTimeHistogramNode *node = (PeriStimulusTimeHistogramNode*)p[k];
			if (node->trialCircularBuffer != nullptr)
			{
				node->trialCircularBuffer->channelChange(electrodeID, channelindex,newchannel);
			}
		}
		if (p[k]->getName() == "Spike Viewer")
		{
			SpikeDisplayNode* node = (SpikeDisplayNode*)p[k];
			node->syncWithSpikeDetector();
		}
	}
}


void SpikeDetector::updateSinks(Electrode* electrode)
{
	// inform sinks about an electrode add 
	ProcessorGraph *g = getProcessorGraph();
	Array<GenericProcessor*> p = g->getListOfProcessors();
	for (int k=0;k<p.size();k++)
	{
		String s = p[k]->getName();
		if (p[k]->getName() == "PSTH")
		{
			PeriStimulusTimeHistogramNode *node = (PeriStimulusTimeHistogramNode*)p[k];
			if (node->trialCircularBuffer != nullptr)
			{
					// add electrode
					node->trialCircularBuffer->addNewElectrode(electrode);
			}
		}
		if (p[k]->getName() == "Spike Viewer")
		{
			SpikeDisplayNode* node = (SpikeDisplayNode*)p[k];
			node->syncWithSpikeDetector();
		}
	}
}


void SpikeDetector::updateSinks(int electrodeID)
{
	// inform sinks about an electrode removal
	ProcessorGraph *g = getProcessorGraph();
	Array<GenericProcessor*> p = g->getListOfProcessors();
	for (int k=0;k<p.size();k++)
	{
		String s = p[k]->getName();
		if (p[k]->getName() == "PSTH")
		{
			PeriStimulusTimeHistogramNode *node = (PeriStimulusTimeHistogramNode*)p[k];
			if (node->trialCircularBuffer != nullptr)
			{
				// remove electrode
				node->trialCircularBuffer->removeElectrode(electrodeID);
			}
		}
		if (p[k]->getName() == "Spike Viewer")
		{
			SpikeDisplayNode* node = (SpikeDisplayNode*)p[k];
			node->syncWithSpikeDetector();
		}
	}
}

void SpikeDetector::addElectrode(Electrode* newElectrode)
{
	mut.enter();
    resetElectrode(newElectrode);
    electrodes.add(newElectrode);
	// inform PSTH sink, if it exists, about this new electrode.
	updateSinks(newElectrode);
	mut.exit();
}

bool SpikeDetector::addElectrode(int nChans, String name, double Depth)
{

	mut.enter();
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
		mut.exit();
        return false;
    }
	
	int *channels = new int[nChans];
	for (int k=0;k<nChans;k++)
		channels[k] = firstChan+k;

	Electrode* newElectrode = new Electrode(++uniqueID,&computingThread,name, nChans,channels, getDefaultThreshold(), 
		numPreSamples,numPostSamples, getSampleRate());

	newElectrode->depthOffsetMM = Depth;
	String log = "Added electrode (ID "+String(uniqueID)+") with " + String(nChans) + " channels." ;
    std::cout <<log << std::endl;
	String eventlog = "NewElectrode "+String(uniqueID) + " "+String(nChans)+" ";
	for (int k=0;k<nChans;k++)
		eventlog += String(channels[k])+ " " + name;

	addNetworkEventToQueue(StringTS(eventlog));

    resetElectrode(newElectrode);
    electrodes.add(newElectrode);
	updateSinks(newElectrode);
	setCurrentElectrodeIndex(electrodes.size()-1);
	mut.exit();
    return true;

}

float SpikeDetector::getDefaultThreshold()
{
    return -20.0f;
}

StringArray SpikeDetector::getElectrodeNames()
{
    StringArray names;
	mut.enter();
    for (int i = 0; i < electrodes.size(); i++)
    {
        names.add(electrodes[i]->name);
    }
	mut.exit();
    return names;
}

void SpikeDetector::resetElectrode(Electrode* e)
{
    e->lastBufferIndex = 0;
}

bool SpikeDetector::removeElectrode(int index)
{
	mut.enter();
    // std::cout << "Spike detector removing electrode" << std::endl;

    if (index > electrodes.size() || index < 0) {
        mut.exit();
		return false;
	}

	
	String log = "Removing electrode (ID " + String(electrodes[index]->electrodeID)+")";
	std::cout << log <<std::endl;

	String eventlog = "RemoveElectrode " + String(electrodes[index]->electrodeID);
	addNetworkEventToQueue(StringTS(eventlog));
	
	int idToRemove = electrodes[index]->electrodeID;
    electrodes.remove(index);

	updateSinks(idToRemove);

	if (electrodes.size() > 0)
		currentElectrode = electrodes.size()-1;
	else
		currentElectrode = -1;
	
	mut.exit();
    return true;
}

void SpikeDetector::setElectrodeName(int index, String newName)
{
	mut.enter();
    electrodes[index-1]->name = newName;
	mut.exit();
}

void SpikeDetector::setChannel(int electrodeIndex, int channelNum, int newChannel)
{
	mut.enter();
	String log = "Setting electrode " + String(electrodeIndex) + " channel " + String( channelNum )+
              " to " + String( newChannel );
    std::cout << log<< std::endl;


	
	String eventlog = "ChanelElectrodeChannel " + String(electrodes[electrodeIndex]->electrodeID) + " " + String(channelNum) + " " + String(newChannel);
	addNetworkEventToQueue(StringTS(eventlog));
	
	updateSinks(electrodes[electrodeIndex]->electrodeID, channelNum,newChannel);

    *(electrodes[electrodeIndex]->channels+channelNum) = newChannel;
	mut.exit();
}

int SpikeDetector::getNumChannels(int index)
{
	mut.enter();
    int i=electrodes[index]->numChannels;
	mut.exit();
	return i;
}

int SpikeDetector::getChannel(int index, int i)
{
	mut.enter();
    int ii=*(electrodes[index]->channels+i);
	mut.exit();
	return ii;
}


void SpikeDetector::setChannelActive(int electrodeIndex, int subChannel, bool active)
{

    currentElectrode = electrodeIndex;
    currentChannelIndex = subChannel;

    if (active)
        setParameter(98, 1);
    else
        setParameter(98, 0);
	
	//getEditorViewport()->makeEditorVisible(this, true, true);
}

bool SpikeDetector::isChannelActive(int electrodeIndex, int i)
{
	mut.enter();
	bool b= *(electrodes[electrodeIndex]->isActive+i);
	mut.exit();
	return b;
}


void SpikeDetector::setChannelThreshold(int electrodeNum, int channelNum, float thresh)
{
	mut.enter();
    currentElectrode = electrodeNum;
    currentChannelIndex = channelNum;
	electrodes[electrodeNum]->thresholds[channelNum] = thresh;
	if (electrodes[electrodeNum]->spikePlot != nullptr)
		electrodes[electrodeNum]->spikePlot->setDisplayThresholdForChannel(channelNum,thresh);

	if (syncThresholds)
	{
		for (int k=0;k<electrodes.size();k++)
		{
			for (int i=0;i<electrodes[k]->numChannels;i++)
			{
				electrodes[k]->thresholds[i] = thresh;
			}
		}
	}

	mut.exit();
    setParameter(99, thresh);
}

double SpikeDetector::getChannelThreshold(int electrodeNum, int channelNum)
{
    mut.enter();
	double f= *(electrodes[electrodeNum]->thresholds+channelNum);
	mut.exit();
	return f;
}

void SpikeDetector::setParameter(int parameterIndex, float newValue)
{
    //editor->updateParameterButtons(parameterIndex);
	mut.enter();
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
	mut.exit();
}


bool SpikeDetector::enable()
{

    useOverflowBuffer = false;
	SpikeDetectorEditor* editor = (SpikeDetectorEditor*) getEditor();
	 editor->enable();
	
    return true;
}


bool SpikeDetector::isReady()
{
    return true;
}


bool SpikeDetector::disable()
{
	mut.enter();
    for (int n = 0; n < electrodes.size(); n++)
    {
        resetElectrode(electrodes[n]);
    }
	//editor->disable();
	mut.exit();
    return true;
}

Electrode* SpikeDetector::getActiveElectrode()
{
    if (electrodes.size() == 0)
    	return nullptr;

    return electrodes[currentElectrode];
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
	mut.enter();
    int spikeLength = electrodes[electrodeNumber]->prePeakSamples +
                      + electrodes[electrodeNumber]->postPeakSamples;
	
    s->timestamp = hardware_timestamp + peakIndex;

	// convert sample offset to software ticks
	float samplesPerSec = getSampleRate();
	s->timestamp_software = software_timestamp + int64( ticksPerSec*float(peakIndex)/samplesPerSec);
    s->nSamples = spikeLength;

    int chan = *(electrodes[electrodeNumber]->channels+currentChannel);

    s->gain[currentChannel] = (int)(1.0f / channels[chan]->bitVolts)*1000;
    s->threshold[currentChannel] = (int) electrodes[electrodeNumber]->thresholds[currentChannel]; // / channels[chan]->bitVolts * 1000;

    // cycle through buffer

    if (isChannelActive(electrodeNumber, currentChannel))
    {

        for (int sample = 0; sample < spikeLength; sample++)
        { 

            // warning -- be careful of bitvolts conversion
            s->data[currentIndex] = uint16(getNextSample(electrodes[electrodeNumber]->channels[currentChannel]) / channels[chan]->bitVolts + 32768);

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
	mut.exit();

}


int64 SpikeDetector::getExtrapolatedHardwareTimestamp(int64 softwareTS)
{
	Time timer;
	// this is the case in which messages arrived before the data stream started....
	if (hardware_timestamp == 0) 
		return 0;

	// compute how many ticks passed since the last known software-hardware pair
	int64 ticksPassed = software_timestamp-softwareTS;
	float secondPassed = (float)ticksPassed / timer.getHighResolutionTicksPerSecond();
	// adjust hardware stamp accordingly
	return hardware_timestamp + secondPassed*getSampleRate();
}




void SpikeDetector::postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events)
{
	uint8* msg_with_ts = new uint8[s.len+8]; // for the two timestamps
	memcpy(msg_with_ts, s.str, s.len);	
	memcpy(msg_with_ts+s.len, &s.timestamp, 8);

	addEvent(events,           // eventBuffer
             (uint8) NETWORK,          // type
             0,                // sampleNum
             0,                // eventId
             (uint8) GENERIC_EVENT,    // eventChannel
             (uint8) s.len+8,  // numBytes
             msg_with_ts);     // eventData

	delete msg_with_ts;
}

void SpikeDetector::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
    if (eventType == TIMESTAMP)
	{
        const uint8* dataptr = event.getRawData();
	      memcpy(&hardware_timestamp, dataptr + 4, 8); // remember to skip first four bytes
		  memcpy(&software_timestamp, dataptr + 12, 8); // remember to skip first four bytes
    }
}

void SpikeDetector::addNetworkEventToQueue(StringTS S)
{
	StringTS copy(S);
	getUIComponent()->getLogWindow()->addLineToLog(copy.getString());
	eventQueue.push(copy);
}


void SpikeDetector::postEventsInQueue(MidiBuffer& events)
{
	while (eventQueue.size() > 0)
	{
		StringTS msg = eventQueue.front();
		postTimestamppedStringToMidiBuffer(msg,events);
		eventQueue.pop();
	}
}

void SpikeDetector::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events,
                            int& nSamples)
{
	mut.enter();
	uint16_t samplingFrequencyHz = getSampleRate();//buffer.getSamplingFrequency();
    // cycle through electrodes
    Electrode* electrode;
    dataBuffer = buffer;
	
	
    checkForEvents(events); // find latest's packet timestamps
	
	postEventsInQueue(events);

	channelBuffers->update(buffer,  hardware_timestamp,software_timestamp,nSamples);
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

                    int currentChannel = electrode->channels[chan];

					bool bSpikeDetectedPositive  = electrode->thresholds[chan] > 0 &&
						(-getNextSample(currentChannel) > electrode->thresholds[chan]); // rising edge
					bool bSpikeDetectedNegative = electrode->thresholds[chan] < 0 &&
						(-getNextSample(currentChannel) < electrode->thresholds[chan]); // falling edge

                    if  (bSpikeDetectedPositive || bSpikeDetectedNegative)
                    { 

                        //std::cout << "Spike detected on electrode " << i << std::endl;
                        // find the peak
                        int peakIndex = sampleIndex;

						if (bSpikeDetectedPositive) 
						{
							// find localmaxima
							while (-getCurrentSample(currentChannel) <
								-getNextSample(currentChannel) &&
								   sampleIndex < peakIndex + electrode->postPeakSamples)
							 {
							 sampleIndex++;
							}
						} else {
							// find local minimum
							
							while (-getCurrentSample(currentChannel) >
								-getNextSample(currentChannel) &&
								   sampleIndex < peakIndex + electrode->postPeakSamples)
							 {
							 sampleIndex++;
							}
						}

                        peakIndex = sampleIndex;
                        sampleIndex -= (electrode->prePeakSamples+1);

                        SpikeObject newSpike;
                        newSpike.timestamp = peakIndex;
						newSpike.electrodeID = electrode->electrodeID;
                        newSpike.source = i;
                        newSpike.nChannels = electrode->numChannels;
						newSpike.samplingFrequencyHz = samplingFrequencyHz;
						newSpike.color[0] = newSpike.color[1] = newSpike.color[2] = 127;
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
						electrode->spikeSort->projectOnPrincipalComponents(&newSpike);

						// Add spike to drawing buffer....
						electrode->spikeSort->sortSpike(&newSpike, PCAbeforeBoxes);

						  // transfer buffered spikes to spike plot
						if (electrode->spikePlot != nullptr) {
							if (electrode->spikeSort->isPCAfinished()) 
							{
								electrode->spikeSort->resetJobStatus();
								float p1min,p2min, p1max,  p2max;
								electrode->spikeSort->getPCArange(p1min,p2min, p1max,  p2max);
								electrode->spikePlot->setPCARange(p1min,p2min, p1max,  p2max);
							}


							electrode->spikePlot->processSpikeObject(newSpike);
						}

//						editor->addSpikeToBuffer(newSpike);
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


	mut.exit();
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
            return *overflowBuffer.getSampleData(chan, ind);
        else
            return 0;

    }
    else
    {
        //  useOverflowBuffer = false;
        // std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;

        if (sampleIndex < dataBuffer.getNumSamples())
            return *dataBuffer.getSampleData(chan, sampleIndex);
        else
            return 0;
    }
    //} else {
    //    std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;
    //     return *dataBuffer.getSampleData(chan, sampleIndex);
    //}

}

float SpikeDetector::getCurrentSample(int& chan)
{

    // if (useOverflowBuffer)
    // {
    //     return *overflowBuffer.getSampleData(chan, overflowBufferSize + sampleIndex - 1);
    // } else {
    //     return *dataBuffer.getSampleData(chan, sampleIndex - 1);
    // }

    if (sampleIndex < 1)
    {
        //std::cout << "  sample index " << sampleIndex << "from overflowBuffer" << std::endl;
        return *overflowBuffer.getSampleData(chan, overflowBufferSize + sampleIndex - 1);
    }
    else
    {
        //  useOverflowBuffer = false;
        // std::cout << "  sample index " << sampleIndex << "from regular buffer" << std::endl;
        return *dataBuffer.getSampleData(chan, sampleIndex - 1);
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

void SpikeDetector::addProbes(String probeType,int numProbes, int nElectrodesPerProbe, int nChansPerElectrode,  double firstContactOffset, double interelectrodeDistance)
{
	for (int probeIter=0;probeIter<numProbes;probeIter++)
	{
		int probeCounter = getUniqueProbeID(probeType);
		for (int electrodeIter=0;electrodeIter<nElectrodesPerProbe;electrodeIter++)
		{
			double depth = firstContactOffset - electrodeIter*interelectrodeDistance;
			String name;
			if (nElectrodesPerProbe > 1)
				 name = probeType + " " + String(probeCounter) + " ["+String(electrodeIter+1)+"/"+String(nElectrodesPerProbe)+"]";
			else
				 name = probeType + " " + String(probeCounter);

			bool successful = addElectrode(nChansPerElectrode, name,  depth);
			if (!successful) {
                sendActionMessage("Not enough channels to add electrode.");
				return;
			}
		}
		increaseUniqueProbeID(probeType);
	}
}
Array<Electrode*> SpikeDetector::getElectrodes()
{
	return electrodes;
}

double SpikeDetector::getAdvancerPosition(int advancerID)
{
	ProcessorGraph *g = getProcessorGraph();
	Array<GenericProcessor*> p = g->getListOfProcessors();
	for (int k=0;k<p.size();k++)
	{
		if (p[k] != nullptr)
		{
			if (p[k]->getName() == "Advancers")
			{
				AdvancerNode *node = (AdvancerNode*)p[k];
				return node->getAdvancerPosition(advancerID);
			}
		}
	}
	return 0.0;
}

double SpikeDetector::getElectrodeDepth(int electrodeID)
{
	for (int k=0;k<electrodes.size();k++)
	{
		if (electrodes[k]->electrodeID == electrodeID)
		{
			double currentAdvancerPos = getAdvancerPosition(electrodes[k]->advancerID);
			return electrodes[k]->depthOffsetMM + currentAdvancerPos;
		}
	}
	return 0.0;
}


double SpikeDetector::getSelectedElectrodeDepth()
{
	if (electrodes.size() == 0)
	return 0.0;

	double currentAdvancerPos = getAdvancerPosition(electrodes[currentElectrode]->advancerID);
	return electrodes[currentElectrode]->depthOffsetMM + currentAdvancerPos;
}

void SpikeDetector::saveCustomParametersToXml(XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("SPIKEDETECTOR");
    mainNode->setAttribute("numElectrodes", electrodes.size());

	SpikeDetectorEditor* ed = (SpikeDetectorEditor*) getEditor();
	
	mainNode->setAttribute("activeElectrode", ed->getSelectedElectrode()-1);
	mainNode->setAttribute("numPreSamples", numPreSamples);
	mainNode->setAttribute("numPostSamples", numPostSamples);
	mainNode->setAttribute("autoDACassignment",	autoDACassignment);
	mainNode->setAttribute("syncThresholds",syncThresholds);
	mainNode->setAttribute("uniqueID",uniqueID);


    XmlElement* countNode = mainNode->createNewChildElement("ELECTRODE_COUNTER");

	countNode->setAttribute("numElectrodeTypes",  (int)electrodeTypes.size());
	for (int k=0;k<electrodeTypes.size();k++)
	{
		XmlElement* countNode2 = countNode->createNewChildElement("ELECTRODE_TYPE");
		countNode2->setAttribute("type", electrodeTypes[k]);
		countNode2->setAttribute("count", electrodeCounter[k]);
	}

    for (int i = 0; i < electrodes.size(); i++)
    {
        XmlElement* electrodeNode = mainNode->createNewChildElement("ELECTRODE");
        electrodeNode->setAttribute("name", electrodes[i]->name);
        electrodeNode->setAttribute("numChannels", electrodes[i]->numChannels);
        electrodeNode->setAttribute("prePeakSamples", electrodes[i]->prePeakSamples);
        electrodeNode->setAttribute("postPeakSamples", electrodes[i]->postPeakSamples);
		electrodeNode->setAttribute("advancerID", electrodes[i]->advancerID);
		electrodeNode->setAttribute("depthOffsetMM", electrodes[i]->depthOffsetMM);
		electrodeNode->setAttribute("electrodeID", electrodes[i]->electrodeID);

        for (int j = 0; j < electrodes[i]->numChannels; j++)
        {
            XmlElement* channelNode = electrodeNode->createNewChildElement("SUBCHANNEL");
            channelNode->setAttribute("ch",*(electrodes[i]->channels+j));
            channelNode->setAttribute("thresh",*(electrodes[i]->thresholds+j));
            channelNode->setAttribute("isActive",*(electrodes[i]->isActive+j));

        }

		// save spike sorting data.
		electrodes[i]->spikeSort->saveCustomParametersToXml(electrodeNode);

    }


}

void SpikeDetector::loadCustomParametersFromXml()
{

	if (parametersAsXml != nullptr)
	{

		int electrodeIndex = -1;

		forEachXmlChildElement(*parametersAsXml, mainNode)
		{

			// use parametersAsXml to restore state

			if (mainNode->hasTagName("SPIKEDETECTOR"))
			{
				int numElectrodes = mainNode->getIntAttribute("numElectrodes");
				currentElectrode = mainNode->getIntAttribute("activeElectrode");
				numPreSamples = mainNode->getIntAttribute("numPreSamples");
				numPostSamples = mainNode->getIntAttribute("numPostSamples");
				autoDACassignment = mainNode->getBoolAttribute("autoDACassignment");
				syncThresholds = mainNode->getBoolAttribute("syncThresholds");
				uniqueID = mainNode->getIntAttribute("uniqueID");

				forEachXmlChildElement(*mainNode, xmlNode)
				{

					if (xmlNode->hasTagName("ELECTRODE_COUNTER"))
					{
						int numElectrodeTypes = xmlNode->getIntAttribute("numElectrodeTypes");
						electrodeCounter.resize(numElectrodeTypes);
						electrodeTypes.resize(numElectrodeTypes);
						int counter = 0;
						forEachXmlChildElement(*xmlNode, xmltype)
							{
								if (xmltype->hasTagName("ELECTRODE_TYPE"))
								{
									electrodeTypes[counter] = xmltype->getStringAttribute("type");
									electrodeCounter[counter] = xmltype->getIntAttribute("count");
									counter++;
								}
						}
					} else
					if (xmlNode->hasTagName("ELECTRODE"))
					{

						electrodeIndex++;

						int channelsPerElectrode = xmlNode->getIntAttribute("numChannels");

						int advancerID = xmlNode->getIntAttribute("advancerID");
						float depthOffsetMM = xmlNode->getDoubleAttribute("depthOffsetMM");
						int electrodeID = xmlNode->getIntAttribute("electrodeID");
						String electrodeName=xmlNode->getStringAttribute("name");


						int channelIndex = -1;

						int *channels = new int[channelsPerElectrode];
						float *thres = new float[channelsPerElectrode];
						bool *isActive = new bool[channelsPerElectrode];

						forEachXmlChildElement(*xmlNode, channelNode)
						{
							if (channelNode->hasTagName("SUBCHANNEL"))
							{
								channelIndex++;
								channels[channelIndex] = channelNode->getIntAttribute("ch");
								thres[channelIndex] = channelNode->getDoubleAttribute("thresh");
								isActive[channelIndex] = channelNode->getBoolAttribute("isActive");
							}
						}
						Electrode* newElectrode = new Electrode(electrodeID, &computingThread, electrodeName, channelsPerElectrode, channels,getDefaultThreshold(),
							numPreSamples,numPostSamples, getSampleRate());
						for (int k=0;k<channelsPerElectrode;k++)
						{
							newElectrode->thresholds[k] = thres[k];
							newElectrode->isActive[k] = isActive[k];
						}

						newElectrode->advancerID = advancerID;
						newElectrode->depthOffsetMM = depthOffsetMM;
						// now read sorted units information
						newElectrode->spikeSort->loadCustomParametersFromXml(xmlNode);
						addElectrode(newElectrode);

					}
				}
			}
		}
	}
    SpikeDetectorEditor* ed = (SpikeDetectorEditor*) getEditor();
		ed->updateAdvancerList();

	if (currentElectrode >= 0) {
		ed->refreshElectrodeList(currentElectrode);
		ed->setSelectedElectrode(1+currentElectrode);
	} else
	{
		ed->refreshElectrodeList();
	}

	
}



void SpikeDetector::removeSpikePlots()
{
	mut.enter();
    for (int i = 0; i < getNumElectrodes(); i++)
    {
        Electrode *ee = electrodes[i];
		ee->spikePlot = nullptr;
    }
	mut.exit();
}

int SpikeDetector::getNumElectrodes()
{
	mut.enter();
    int i= electrodes.size();
	mut.exit();
	return i;

}

int SpikeDetector::getNumberOfChannelsForElectrode(int i)
{
	mut.enter();
    if (i > -1 && i < electrodes.size())
    {
		Electrode *ee = electrodes[i];
		int ii=ee->numChannels;
		mut.exit();
		return ii;
    } else {
		mut.exit();
        return 0;
    }
}



String SpikeDetector::getNameForElectrode(int i)
{
	mut.enter();
    if (i > -1 && i < electrodes.size())
    {
		Electrode *ee = electrodes[i];
        String s= ee->name;
		mut.exit();
		return s;
    } else {
		mut.exit();
        return " ";
    }
}


void SpikeDetector::addSpikePlotForElectrode(SpikeHistogramPlot* sp, int i)
{
	mut.enter();
    Electrode *ee = electrodes[i];
    ee->spikePlot = sp;
	mut.exit();
}

int SpikeDetector::getCurrentElectrodeIndex()
{
	return currentElectrode;
}

Electrode* SpikeDetector::getElectrode(int i)
{
	return electrodes[i];
}

Electrode* SpikeDetector::setCurrentElectrodeIndex(int i)
{
	jassert(i >= 0 & i  < electrodes.size());
	currentElectrode = i;
	return electrodes[i];
}
/*


Histogram::Histogram(float _minValue, float _maxValue, float _resolution, bool _throwOutsideSamples) :  
	minValue(_minValue), maxValue(_maxValue), resolution(_resolution), throwOutsideSamples(_throwOutsideSamples)
{
	numBins = 1+ abs(maxValue-minValue) / resolution;
	binCounts = new unsigned long[numBins];
	binCenters = new float[numBins];
	float deno = (numBins-1)/abs(maxValue-minValue);
	for (int k=0;k<numBins;k++) 
	{
		binCounts[k] = 0;
		binCenters[k] = minValue + k/deno;
	}
}
//
//Histogram::Histogram(float _minValue, float _maxValue, int _numBins, bool _throwOutsideSamples) :  
//	minValue(_minValue), maxValue(_maxValue), numBins(_numBins), throwOutsideSamples(_throwOutsideSamples)
//{
//	resolution = abs(maxValue-minValue) / numBins ;
//	binCounts = new int[numBins];
//	binCenters = new float[numBins];
//	for (int k=0;k<numBins;k++) 
//	{
//		binCounts[k] = 0;
//		binCenters[k] = minValue + k/(numBins-1)*resolution;
//	}
//
//}

void Histogram::clear()
{
for (int k=0;k<numBins;k++) 
	{
		binCounts[k] = 0;
	}	
}


void Histogram::addSamples(float *Samples, int numSamples) {
	for (int k=0;k<numSamples;k++) 
	{
		int indx = ceil( (Samples[k] - minValue) / (maxValue-minValue) * (numBins-1));
		if (indx >= 0 && indx < numBins)
			binCounts[indx]++;
	}
}

Histogram::~Histogram() 
{
		delete [] binCounts;
		delete [] binCenters;
}

	*/













/***********************************/
/*
circularBuffer::circularBuffer(int NumCh, int NumSamplesToHoldPerChannel, double SamplingRate)
{
            numCh = NumCh;
            samplingRate = SamplingRate;
            Buf.resize(numCh);
			for (int ch=0;ch<numCh;ch++) {
				Buf[ch].resize(NumSamplesToHoldPerChannel);
			}
            BufTS_H.resize(NumSamplesToHoldPerChannel);
            BufTS_S.resize(NumSamplesToHoldPerChannel);
            bufLen = NumSamplesToHoldPerChannel;
            numSamplesInBuf = 0;
            ptr = 0; // points to a valid position in the buffer.
}

circularBuffer::~circularBuffer()
{

}


std::vector<double> circularBuffer::getDataArray(int channel, int N)
{
	std::vector<double> LongArray;
	LongArray.resize(N);
	mut.enter();
           
            int p = ptr - 1;
            for (int k = 0; k < N; k++)
            {
                if (p < 0)
                    p = bufLen - 1;
                LongArray[k] = Buf[channel][p];
                p--;
            }
            mut.exit();
            return LongArray;
}

void circularBuffer::addDataToBuffer(std::vector<std::vector<double>> Data, double SoftwareTS, double HardwareTS)
{
	mut.enter();
	int iNumPoints = Data[0].size();
	for (int k = 0; k < iNumPoints; k++)
	{
		BufTS_H[ptr] = HardwareTS + k;
		BufTS_S[ptr] = SoftwareTS + k / samplingRate;
		for (int ch = 0; ch < numCh; ch++)
		{
			Buf[ch, ptr] = Data[ch, k];
		}
		ptr++;

		if (ptr == bufLen)
		{
			ptr = 0;
		}
		numSamplesInBuf++;
		if (numSamplesInBuf >= bufLen)
		{
			numSamplesInBuf = bufLen;
		}
	}
	mut.exit();
}


double circularBuffer::findThresholdForChannel(int channel)
{
	// Run median on analog input
	double numSamplesPerSecond = 30000;
	std::vector<double> LongArray = getDataArray(channel, numSamplesPerSecond*5);
	
	for (int k = 0; k < LongArray.size(); k++)
		LongArray[k] = fabs(LongArray[k]);

	std::sort (LongArray.begin(), LongArray.begin()+LongArray.size());           //(12 32 45 71)26 80 53 33


	int Middle = LongArray.size() / 2;
	double Median = LongArray[Middle];
	double NewThres = -4.0F * Median / 0.675F;

	return NewThres;
}


/**************************************/

void ContinuousCircularBuffer::reallocate(int NumCh)
{
	numCh =NumCh;
	Buf.resize(numCh);
	for (int k=0;k< numCh;k++)
	{
		Buf[k].resize(bufLen);
	}
	numSamplesInBuf = 0;
	ptr = 0; // points to a valid position in the buffer.

}


ContinuousCircularBuffer::ContinuousCircularBuffer(int NumCh, float SamplingRate, int SubSampling, float NumSecInBuffer)
{
		Time t;
	
	numTicksPerSecond = t.getHighResolutionTicksPerSecond();

	int numSamplesToHoldPerChannel = (SamplingRate * NumSecInBuffer / SubSampling);
	subSampling = SubSampling;
	samplingRate = SamplingRate;
	numCh =NumCh;
	leftover_k = 0;
	Buf.resize(numCh);


	for (int k=0;k< numCh;k++)
	{
		Buf[k].resize(numSamplesToHoldPerChannel);
	}

	hardwareTS.resize(numSamplesToHoldPerChannel);
	softwareTS.resize(numSamplesToHoldPerChannel);
	valid.resize(numSamplesToHoldPerChannel);
	bufLen = numSamplesToHoldPerChannel;
	numSamplesInBuf = 0;
	ptr = 0; // points to a valid position in the buffer.
}

/*
void ContinuousCircularBuffer::FindInterval(int saved_ptr, double Bef, double Aft, int &start, int &N)
{
	int CurrPtr = saved_ptr;
	N = 0;
	while (N < bufLen && N < numSamplesInBuf)
	{
		if ((TS[CurrPtr] < Bef) || (TS[CurrPtr] > Aft) || !valid[CurrPtr])
			break;
		// Add spike..
		CurrPtr--;
		N++;
		if (CurrPtr < 0)
			CurrPtr = bufLen - 1;
	}
	if (N > 0 && !valid[CurrPtr])
	{
		CurrPtr++;
		if (CurrPtr >= bufLen)
			CurrPtr = 0;
	}
	start = CurrPtr;
	CurrPtr = saved_ptr;
	while (N < bufLen && N < numSamplesInBuf)
	{
		if ((TS[CurrPtr] < Bef) || (TS[CurrPtr] > Aft) || !valid[CurrPtr])
			break;
		// Add spike..
		CurrPtr++;
		N++;
		if (CurrPtr >= bufLen)
			CurrPtr = 0;
	}
	if (N > 0 && !valid[CurrPtr])
	{
		CurrPtr--;
		if (CurrPtr < 0)
			CurrPtr = bufLen-1;
	}

}
*/

/*
LFP_Trial_Data ContinuousCircularBuffer::GetRelevantData(int saved_ptr, double Start_TS, double Align_TS, double End_TS, double BeforeSec, double AfterSec)
{
             // gurantee to return to return the first index "start" such that TS[start] < T0-SearchBeforeSec
             // and TS[end] > T0+SearchAfterSec
	mut.enter();
	int N,start;
	FindInterval(saved_ptr, Start_TS - BeforeSec, End_TS + AfterSec, start, N);
	LFP_Trial_Data triallfp(numCh, N);
	int p = start;
	for (int k = 0; k < N; k++)
	{
		triallfp.time[k] = TS[p]-Align_TS;
		for (int ch = 0; ch < numCh; ch++)
		{
			triallfp.data[ch][k] = Buf[ch][p];
		}
		p++;
		if (p >= bufLen)
			p = 0;
	}
	mut.exit();
	return triallfp;
}
*/
void ContinuousCircularBuffer::update(AudioSampleBuffer& buffer, int64 hardware_ts, int64 software_ts, int numpts)
{
	mut.enter();
	
	// we don't start from zero because of subsampling issues.
	// previous packet may not have ended exactly at the last given sample.
	int k = leftover_k;
	for (; k < numpts; k+=subSampling)
	{
		valid[ptr] = true;
		hardwareTS[ptr] = hardware_ts + k;
		softwareTS[ptr] = software_ts + int64(float(k) / samplingRate * numTicksPerSecond);

		for (int ch = 0; ch < numCh; ch++)
		{
			Buf[ch][ptr] = *(buffer.getSampleData(ch,k));
		}
		ptr++;
		if (ptr == bufLen)
		{
			ptr = 0;
		}
		numSamplesInBuf++;
		if (numSamplesInBuf >= bufLen)
		{
			numSamplesInBuf = bufLen;
		}
	}
	int lastUsedSample = floor(numpts/subSampling) * subSampling;
	int numMissedSamples = numpts-lastUsedSample;
	leftover_k =subSampling-numMissedSamples;
	mut.exit();

}
/*
void ContinuousCircularBuffer::AddDataToBuffer(std::vector<std::vector<double>> lfp, double soft_ts)
{
	mut.enter();
	int numpts = lfp[0].size();
	for (int k = 0; k < numpts / subSampling; k++)
	{
		valid[ptr] = true;
		for (int ch = 0; ch < numCh; ch++)
		{
			Buf[ch][ptr] = lfp[ch][k];
			TS[ptr] = soft_ts + (double)(k * subSampling) / samplingRate;
		}
		ptr++;
		if (ptr == bufLen)
		{
			ptr = 0;
		}
		numSamplesInBuf++;
		if (numSamplesInBuf >= bufLen)
		{
			numSamplesInBuf = bufLen;
		}
	}
	mut.exit();
}
*/
        
int ContinuousCircularBuffer::GetPtr()
{
	return ptr;
}

/************************************************************/



