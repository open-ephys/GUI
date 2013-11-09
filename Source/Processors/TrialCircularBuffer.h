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

#include "GenericProcessor.h"
#include "PeriStimulusTimeHistogramNode.h"
#include "Editors/PeriStimulusTimeHistogramEditor.h"
#include "Visualization/SpikeObject.h"
#include "SpikeDetector.h"
#include <algorithm>    
#include <queue>
#include <vector>
#include <list>

class PeriStimulusTimeHistogramNode;


class Condition
{
	public:
		Condition();
		Condition(std::vector<String> items);
		Condition(const Condition &c);
  	   Condition(String Name, std::vector<int> types, std::vector<int> outcomes, double _postSec, double _preSec);
       String name;
       float colorRGB[3]; 
       std::vector<int> trialTypes;
       std::vector<int> trialOutcomes;
       double postSec, preSec;
       bool visible;
	   int conditionID;
};


class Trial {
public:
	Trial();
	Trial(const Trial &t);

	int trialID;
	int outcome;
	int type;
	uint64 startTS, alignTS, endTS;
	bool trialInProgress; 
};

class SmartSpikeCircularBuffer 
{
public:
	SmartSpikeCircularBuffer(float maxTrialTimeSeconds, int maxTrialsInMemory);
	// contains spike times, but also pointers for trial onsets so we don't need to search
	// the entire array 
	void addSpikeToBuffer(uint64 spikeTimeSoftware);
	std::vector<int64> getAlignedSpikes(Trial *trial, float preSecs, float postSecs);
	void addTrialStartToBuffer(Trial *t);

	int queryTrialStart(int trialID);
private:
	std::vector<uint64> spikeTimesSoftware;
	std::vector<int> trialID;
	std::vector<int> pointers;
	int maxTrialsInMemory;
	int bufferSize;
	int bufferIndex;
	int trialIndex;

	int numTrialsStored;
	int numSpikesStored;
};

class ConditionPSTH
{
public:
	ConditionPSTH(int ID, float _maxTrialTimeSec, float _preSecs, float _postSecs);
	ConditionPSTH(const ConditionPSTH& c);
	void clear();
	void updatePSTH(SmartSpikeCircularBuffer *spikeBuffer, Trial *trial);
	

	float preSecs, postSecs, maxTrialTimeSec;
	int conditionID;

	int numBins;
	int binResolutionMS;
	int numTrials;
	float timeSpanSecs;
	std::vector<int> numDataPoints;
	std::vector<float> binTime;
	std::vector<float> avgFiringRateHz;
	
private:
	std::vector<uint64> getAlignSpikes(SmartSpikeCircularBuffer *spikeBuffer, Trial *t);
};

class UnitPSTHs 
{
public:
	UnitPSTHs(int ID, float maxTrialTimeSeconds, int maxTrialsInMemory);
	void updateConditionsWithSpikes(std::vector<int> conditionsNeedUpdating, Trial *trial);
	void addSpikeToBuffer(uint64 spikeTimestampSoftware);
	void addTrialStartToSmartBuffer(Trial *t);
	void clearStatistics();

	int unitID;
	std::vector<ConditionPSTH> conditionPSTHs;
	
	SmartSpikeCircularBuffer spikeBuffer;
};

class ElectrodePSTH
{
public:
	ElectrodePSTH(int ID);

	int electrodeID;
	std::vector<int> channels;
	std::vector<UnitPSTHs> unitsPSTHs;
};

class TrialCircularBuffer 
{
public:
	TrialCircularBuffer();
	TrialCircularBuffer(PeriStimulusTimeHistogramNode *p);
	~TrialCircularBuffer();
	void updateUnitsWithTrial(Trial *trial);
  
	bool contains(std::vector<int> v, int x);
	
	void parseMessage(StringTS s);
	void addSpikeToSpikeBuffer(SpikeObject newSpike);
	void process();
	
	void addDefaultTTLConditions();
	void addCondition(std::vector<String> input);
	void lockConditions();
	void unlockConditions();
	void lockPSTH();
	void unlockPSTH();

	 double postSec, preSec;
     
	 float maxTrialTimeSeconds;
	 int  maxTrialsInMemory;
	 int trialCounter;
	int conditionCounter;
	CriticalSection conditionMutex, psthMutex;
	Trial currentTrial;
	uint64 MaxTrialTimeTicks;
	String designName;
	bool addDefaultTTLconditions;
	std::queue<Trial> aliveTrials;
	std::vector<Condition> conditions;
	std::vector<ElectrodePSTH> electrodesPSTH;
  
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
	std::vector<String> splitString(String S, char sep);
	

	PeriStimulusTimeHistogramNode *processor;
};


#endif  // __TRIALCIRCULARBUFFER_H__
