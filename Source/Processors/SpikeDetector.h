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

#ifndef __SPIKEDETECTOR_H_3F920F95__
#define __SPIKEDETECTOR_H_3F920F95__

#include "../../JuceLibraryCode/JuceHeader.h"

#include "GenericProcessor.h"
#include "Editors/SpikeDetectorEditor.h"
#include "SpikeSortBoxes.h"
#include "NetworkEvents.h"
#include "Visualization/SpikeObject.h"
#include "AdvancerNode.h"
#include <algorithm>    // std::sort
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

class SpikeDetectorEditor;
class SpikeHistogramPlot;
class Trial;
/**

  Detects spikes in a continuous signal and outputs events containing the spike data.

  @see GenericProcessor, SpikeDetectorEditor

*/


// Implements various spike sorting algorithms.
// Currently, only box method is implemented.
// PCA will be added at some point later.


/*
class Histogram {
public:
	Histogram(float _minValue, float _maxValue, float _resolution, bool _throwOutsideSamples);
	//Histogram(float _minValue, float _maxValue, int _numBins, bool _throwOutsideSamples);
	void addSamples(float *Samples, int numSamples);
	~Histogram();
	void clear();

	float minValue, maxValue, resolution;
	int numBins;
	bool throwOutsideSamples;
	unsigned long *binCounts;
	float *binCenters;
};
*/

class PCAjob;
class PCAcomputingThread;

class Electrode
{
	public:
		Electrode(int electrodeID, PCAcomputingThread *pth,String _name, int _numChannels, int *_channels, float default_threshold, int pre, int post, float samplingRate );
        ~Electrode();
		String name;

        int numChannels;
        int prePeakSamples, postPeakSamples;
        int lastBufferIndex;

		int advancerID;
		float depthOffsetMM;

		int electrodeID;
        int* channels;
        double* thresholds;
        bool* isActive;
		SpikeHistogramPlot* spikePlot;
		SpikeSortBoxes* spikeSort;
		PCAcomputingThread *computingThread;
};


class ContinuousCircularBuffer
{
public:
	ContinuousCircularBuffer(int NumCh, float SamplingRate, int SubSampling, float NumSecInBuffer);
	void reallocate(int N);
	void update(AudioSampleBuffer& buffer, int64 hardware_ts, int64 software_ts, int numpts);
	int GetPtr();
	void addTrialStartToSmartBuffer(int trialID);
	int numCh;
	int subSampling;
	float samplingRate;
	CriticalSection mut;
	int numSamplesInBuf;
	float numTicksPerSecond;
	int ptr;
	int bufLen;
	int leftover_k;
	std::vector<std::vector<float>> Buf;
	std::vector<bool> valid;
	std::vector<int64> hardwareTS,softwareTS;
};


class StringTS;

class SpikeDetector : public GenericProcessor
{
public:

    // CONSTRUCTOR AND DESTRUCTOR //

    /** constructor */
    SpikeDetector();

    /** destructor */
    ~SpikeDetector();


    // PROCESSOR METHODS //

    /** Processes an incoming continuous buffer and places new
        spikes into the event buffer. */
    void process(AudioSampleBuffer& buffer, MidiBuffer& events, int& nSamples);

    /** Used to alter parameters of data acquisition. */
    void setParameter(int parameterIndex, float newValue);

    /** Called whenever the signal chain is altered. */
    void updateSettings();

    /** Called prior to start of acquisition. */
    bool enable();

    /** Called after acquisition is finished. */
    bool disable();

	
	bool isReady();
    /** Creates the SpikeDetectorEditor. */
    AudioProcessorEditor* createEditor();


	
	void addNetworkEventToQueue(StringTS S);

	void postEventsInQueue(MidiBuffer& events);

    // INTERNAL BUFFERS //

    /** Extra samples are placed in this buffer to allow seamless
        transitions between callbacks. */
    AudioSampleBuffer overflowBuffer;


    // CREATE AND DELETE ELECTRODES //

    /** Adds an electrode with n channels to be processed. */
    bool addElectrode(int nChans, String name, double depth);

	void addProbes(String probeType,int numProbes, int nElectrodesPerProbe, int nChansPerElectrode,  double firstContactOffset, double interelectrodeDistance);

    /** Removes an electrode with a given index. */
    bool removeElectrode(int index);


    // EDIT AND QUERY ELECTRODE SETTINGS //

    /** Returns the number of channels for a given electrode. */
    int getNumChannels(int index);

    /** Edits the mapping between input channels and electrode channels. */
    void setChannel(int electrodeIndex, int channelNum, int newChannel);

    /** Returns the continuous channel that maps to a given
    	electrode channel. */
    int getChannel(int index, int chan);

    /** Sets the name of a given electrode. */
    void setElectrodeName(int index, String newName);

    /** */
    void setChannelActive(int electrodeIndex, int channelNum, bool active);

    /** */
    bool isChannelActive(int electrodeIndex, int channelNum);

	/** returns the current active electrode, i.e., the one displayed in the editor */
	Electrode* getActiveElectrode();
    
    /** Returns a StringArray containing the names of all electrodes */
    StringArray getElectrodeNames();

	/** modify a channel spike detection threshold */
    void setChannelThreshold(int electrodeNum, int channelNum, float threshold);

	/** returns a channel's detection threshold */
    double getChannelThreshold(int electrodeNum, int channelNum);
	
	/** sync PSTH : inform of a new electrode added / removed */
	void updatePSTHsink(Electrode* newElectrode, bool addRemove); 
	/** sync PSTH : inform of a channel swap */
	void updatePSTHsink(int electrodeID, int channelindex, int newchannel);
	/** sync PSTH: inform of a new unit added / removed */
	void updatePSTHsink(int electrodeID, int unitID, uint8 r, uint8 g, uint8 b, bool addRemove);

	/** used to generate messages over the network and to inform PSTH sink */
	void addNewUnit(int electrodeID, int newUnitID, uint8 r, uint8 g, uint8 b);
	void removeUnit(int electrodeID, int newUnitID);

	/** saves all electrodes, thresholds, units, etc to xml */
    void saveCustomParametersToXml(XmlElement* parentElement);
    void loadCustomParametersFromXml();

	/** returns the depth of an electrode. The depth is calculated as the
	known depth of the advancer that is used to control that electrode, plus
	the defined depth offset. Depth offset is mainly useful for depth probes,
	in which the contact position is not always the at the tip */
	double getElectrodeDepth(int electrodeID);

	/** returns the number of electrodes */
	int getNumElectrodes();

	/** clears up the spike plots. Called during updates */
	void removeSpikePlots();

	int getNumberOfChannelsForElectrode(int i);
	String getNameForElectrode(int i);
	void addSpikePlotForElectrode(SpikeHistogramPlot* sp, int i);
	int getCurrentElectrodeIndex();
	Electrode* setCurrentElectrodeIndex(int i);
	Electrode* getElectrode(int i);
	StringTS createStringTS(String S);
	int64 getExtrapolatedHardwareTimestamp(int64 softwareTS);
	void postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events);
	void setElectrodeAdvancer(int i,int ID);
	void setElectrodeAdvancerOffset(int i, double v);
	double getAdvancerPosition(int advancerID);
	double getSelectedElectrodeDepth();

	Array<Electrode*> getElectrodes();

    std::vector<String> electrodeTypes;

private:
	void addElectrode(Electrode* newElectrode);
	void increaseUniqueProbeID(String type);
	int getUniqueProbeID(String type);

	float ticksPerSec;
	int uniqueID;
	std::queue<StringTS> eventQueue;
    /** Reference to a continuous buffer. */
    AudioSampleBuffer& dataBuffer;

    float getDefaultThreshold();

    int overflowBufferSize;

    int sampleIndex;

    std::vector<int> electrodeCounter;
    float getNextSample(int& chan);
    float getCurrentSample(int& chan);
    bool samplesAvailable(int& nSamples);

    bool useOverflowBuffer;

    int currentElectrode;
    int currentChannelIndex;
    int currentIndex;


	int numPreSamples,numPostSamples;
    uint8_t* spikeBuffer;///[256];
    //int64 timestamp;
		  int64 hardware_timestamp;
		  int64 software_timestamp;

	bool PCAbeforeBoxes;
 	ContinuousCircularBuffer* channelBuffers; // used to compute auto threshold

     void handleEvent(int eventType, MidiMessage& event, int sampleNum);

    void addSpikeEvent(SpikeObject* s, MidiBuffer& eventBuffer, int peakIndex);
 
    void resetElectrode(Electrode*);
	CriticalSection mut;

	private:
   void addWaveformToSpikeObject(SpikeObject* s,
                                  int& peakIndex,
                                  int& electrodeNumber,
                                  int& currentChannel);


		   Array<Electrode*> electrodes;
		   PCAcomputingThread computingThread;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeDetector);

};




/*

class circularBuffer {
public:
	circularBuffer(int NumCh, int NumSamplesToHoldPerChannel, double SamplingRate);
	~circularBuffer();
	
	std::vector<double> getDataArray(int channel, int N);
	double findThresholdForChannel(int channel);
	void update(AudioSampleBuffer& buffer);

private:
     CriticalSection mut;
 
	int numCh;
	int numSamplesInBuf;
	int ptr;
	double samplingRate;
	int bufLen;
	std::vector<std::vector<double>> Buf;
	std::vector<double> BufTS_H;
	std::vector<double> BufTS_S;
};


*/











#endif  // __SPIKEDETECTOR_H_3F920F95__
