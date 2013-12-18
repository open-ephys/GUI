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

#ifndef __TRIALCIRCULARBUFFER_H__
#define __TRIALCIRCULARBUFFER_H__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "PeriStimulusTimeHistogramNode.h"
#include "GenericProcessor.h"

#include "Editors/PeriStimulusTimeHistogramEditor.h"
#include "Visualization/SpikeObject.h"
#include "SpikeDetector.h"

#include <algorithm>    
#include <queue>
#include <vector>
#include <list>
class Electrode;
class PeriStimulusTimeHistogramNode;

#define TTL_TRIAL_OFFSET 30000

#ifndef MAX
#define MAX(a,b)((a)<(b)?(b):(a))
#endif

#ifndef MIN
#define MIN(a,b)((a)<(b)?(a):(b))
#endif
class Condition
{
	public:
		Condition();
		Condition(std::vector<String> items, int ID);
		Condition(const Condition &c);
  	   Condition(String Name, std::vector<int> types, std::vector<int> outcomes, double _postSec, double _preSec);
       String name;
       uint8 colorRGB[3]; 
       std::vector<int> trialTypes;
       std::vector<int> trialOutcomes;
	   //void setDefaultColors(uint8 &R, uint8 &G, uint8 &B, int ID);

       double postSec, preSec;
       bool visible;
	   int conditionID;
	   float posX, posY; // some conditions might be displayed according to their spatial location
	   int conditionGroup; // conditions may be groupped to allow better visualization.
};


class Trial {
public:
	Trial();
	Trial(const Trial &t);

	int trialID;
	int outcome;
	int type;
	int64 startTS, alignTS, endTS, alignTS_hardware;
	bool trialInProgress; 
	bool hardwareAlignment;
};

class SmartSpikeCircularBuffer 
{
public:
	SmartSpikeCircularBuffer(float maxTrialTimeSeconds, int maxTrialsInMemory, int _sampleRateHz);
	// contains spike times, but also pointers for trial onsets so we don't need to search
	// the entire array 
	void addSpikeToBuffer(int64 spikeTimeSoftware,int64 spikeTimeHardware);
	std::vector<int64> getAlignedSpikes(Trial *trial, float preSecs, float postSecs);
	void addTrialStartToBuffer(Trial *t);

	int queryTrialStart(int trialID);
private:
	std::vector<int64> spikeTimesSoftware;
	std::vector<int64> spikeTimesHardware;
	std::vector<int> trialID;
	std::vector<int> pointers;
	int maxTrialsInMemory;
	int bufferSize;
	int bufferIndex;
	int trialIndex;
	int sampleRateHz;
	int numTrialsStored;
	int numSpikesStored;
};



class SmartContinuousCircularBuffer : public ContinuousCircularBuffer 
{
public:
	SmartContinuousCircularBuffer(int NumCh, float SamplingRate, int SubSampling, float NumSecInBuffer);
	bool getAlignedData(std::vector<int> channels, Trial *trial, std::vector<float> *timeBins,
									float preSec, float postSec,
									std::vector<std::vector<float> > &output,
									std::vector<float> &valid);

	void addTrialStartToSmartBuffer(int trialID);
	int trialptr;
	int numTrials;
	std::vector<int> smartPointerIndex;
	std::vector<int> smartPointerTrialID;

};


class ConditionPSTH
{
public:
	ConditionPSTH(int ID, float _maxTrialTimeSec, float _preSecs, float _postSecs);
	ConditionPSTH(const ConditionPSTH& c);
	void clear();
	void updatePSTH(SmartSpikeCircularBuffer *spikeBuffer, Trial *trial);
	void updatePSTH(std::vector<float> alignedLFP,std::vector<float> valid);

	void getRange(float &xMin, float &xMax, float &yMin, float &yMax);
	float preSecs, postSecs, maxTrialTimeSec;
	int conditionID;

	float xmin, xmax, ymax,ymin;
	bool visible;
	int numBins;
	int binResolutionMS;
	int numTrials;
	float timeSpanSecs;
	std::vector<int> numDataPoints;
	std::vector<float> binTime;
	std::vector<float> avgResponse; // either firing rate or lfp
	uint8 colorRGB[3];
	float preSec,postSec;
private:
	std::vector<int64> getAlignSpikes(SmartSpikeCircularBuffer *spikeBuffer, Trial *t);
};

class UnitPSTHs 
{
public:
	UnitPSTHs(int ID, float maxTrialTimeSeconds, int maxTrialsInMemory, int sampleRateHz,uint8 R, uint8 G, uint8 B);
	void updateConditionsWithSpikes(std::vector<int> conditionsNeedUpdating, Trial *trial);
	void addSpikeToBuffer(int64 spikeTimestampSoftware,int64 spikeTimestampHardware);
	void addTrialStartToSmartBuffer(Trial *t);
	void clearStatistics();
	void getRange(float &xmin, float &xmax, float &ymin, float &ymax);
	bool isNewDataAvailable();
	void informPainted();

	std::vector<ConditionPSTH> conditionPSTHs;
	SmartSpikeCircularBuffer spikeBuffer;
	uint8 colorRGB[3];
	int unitID;
	bool redrawNeeded;
};

class ChannelPSTHs 
{
public:
	ChannelPSTHs(int channelID, float maxTrialTimeSeconds, int maxTrialsInMemory, float preSecs, float postSecs, int binResolutionMS);
	void updateConditionsWithLFP(std::vector<int> conditionsNeedUpdating, std::vector<float> lfpData, std::vector<float> valid);
	void clearStatistics();
	void getRange(float &xmin, float &xmax, float &ymin, float &ymax);
	bool isNewDataAvailable();
	void informPainted();
	int channelID;
	std::vector<ConditionPSTH> conditionPSTHs;
	std::vector<float> binTime;
	float preSecs, postSecs;
	bool redrawNeeded;
};


class ElectrodePSTH
{
public:
	ElectrodePSTH(int ID);

	void updateChannelsConditionsWithLFP(std::vector<int> conditionsNeedUpdate, Trial *trial, SmartContinuousCircularBuffer *lfpBuffer);



	int electrodeID;
	std::vector<int> channels;
	std::vector<UnitPSTHs> unitsPSTHs;
	std::vector<ChannelPSTHs> channelsPSTHs;
};

class TrialCircularBuffer 
{
public:
	TrialCircularBuffer();
	TrialCircularBuffer(int numCh, float samplingRate, PeriStimulusTimeHistogramNode *p);
	~TrialCircularBuffer();
    void updatePSTHwithTrial(Trial *trial);
	bool contains(std::vector<int> v, int x);
	void toggleConditionVisibility(int cond);
	void parseMessage(StringTS s);
	void addSpikeToSpikeBuffer(SpikeObject newSpike);
	void process(AudioSampleBuffer& buffer,int nSamples,int64 hardware_timestamp,int64 software_timestamp);
	
	void addTTLevent(int channel,int64 ttl_timestamp_software,int64 ttl_timestamp_hardware);
	void addDefaultTTLConditions();
	void addCondition(std::vector<String> input);
	void lockConditions();
	void unlockConditions();
	void lockPSTH();
	void unlockPSTH();
	void reallocate(int numChannels);
	void simulateTTLtrial(int channel, int64 ttl_timestamp_software);
	void clearDesign();
	void clearAll();
	
	void channelChange(int electrodeID, int channelindex, int newchannel);
	void syncInternalDataStructuresWithSpikeSorter(Array<Electrode *> electrodes);
	void addNewElectrode(Electrode *electrode);
	void removeElectrode(Electrode *electrode);
	void addNewUnit(int electrodeID, int unitID, uint8 r,uint8 g,uint8 b);
	void removeUnit(int electrodeID, int unitID);
	void setHardwareTriggerAlignmentChannel(int k);

	int samplingRateHz;
	bool firstTime;
	 double postSec, preSec;
     int numTTLchannels ;
	 float numTicksPerSecond;
	 float binResolutionMS;
	 float maxTrialTimeSeconds;
	 int  maxTrialsInMemory;
	 int trialCounter;
	int conditionCounter;
	CriticalSection conditionMutex, psthMutex;
	Trial currentTrial;
	int64 MaxTrialTimeTicks;
	String designName;
	bool addDefaultTTLconditions;

	int hardwareTriggerAlignmentChannel;
	float ttlSupressionTimeSec;
	float ttlTrialLengthSec;

	std::vector<int64> lastTTLts;
	std::queue<Trial> aliveTrials;
	std::vector<Condition> conditions;
	std::vector<ElectrodePSTH> electrodesPSTH;
	SmartContinuousCircularBuffer *lfpBuffer;
  
	/*
	void AddDefaultTTLConditions();
	void AddCondition(const Condition &c);

	std::vector<String> SplitString(String S, char sep);

	void LockConditions();
	void UnlockConditions();

	void ClearStats(int channel, int unitID);
	void ParseMessage(Event E);
	Condition fnParseCondition(std::vector<String> items);
	void AddEvent(Event E);
	void run();
	void UpdateConditionsWithTrial(Trial trial);
	void UpdateTrialWithLFPData(Trial t);
	void UpdateTrialWithSpikeData(Trial trial);
	void AddSpikeToSpikeBuffer(int channel, int UnitID, double ts) ;

	bool UnitAlive(int channel, int UnitID);
	void RemoveUnitStats(int channel, int UnitID);

	bool Contains(std::vector<int> vec, int value);

	//int debug;
	double MAX_TRIAL_TIME_SEC;
	double AfterSec, BeforeSec;
	String DesignName;
	
	Trial ttlTrial;
	std::vector<double> lastTTLtrialTS;
	thread_safe_queue<Event> eventQueue;
	std::queue<Trial> AliveTrials;
	juce::Time timer;
	double TTL_Trial_Length_Sec;
	double TTL_Trial_Inhibition_Sec;
	std::vector<Condition> conditions;
	int numCh, numTTLch;
	//DWORD StatThread;
	bool ProcessThreadRunning;
	int numTrials;
	double avgTrialLengthSec;
	//int num_recv_trials = 0;
	std::vector<std::list<SpikeCircularBuffer>> spikeBuffer;
	ContinuousCircularBuffer *lfpBuffer;
	CriticalSection conditionsMutex;
	*/
private:
	PeriStimulusTimeHistogramNode *processor;
};


#endif  // __TRIALCIRCULARBUFFER_H__
