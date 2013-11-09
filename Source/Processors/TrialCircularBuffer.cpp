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
#include "TrialCircularBuffer.h"
#include "SpikeDetector.h"
#include "Channel.h"
#include <string>




/******************************/
ConditionPSTH::ConditionPSTH(int ID, float _maxTrialTimeSec, float pre, float post) : conditionID(ID), preSecs(pre), 
	postSecs(post), numTrials(0), maxTrialTimeSec(_maxTrialTimeSec), binResolutionMS(1)
{
	// allocate data for 1 ms resolution bins to cover trials 
	timeSpanSecs=preSecs + postSecs + maxTrialTimeSec;
	numBins = (timeSpanSecs) * 1000.0f / binResolutionMS; // 1 ms resolution
	avgFiringRateHz.resize(numBins);
	numDataPoints.resize(numBins);
	binTime.resize(numBins);
	
	for (int k = 0; k < numBins; k++)
	{
		numDataPoints[k] = 0;
		binTime[k] = (float)k / numBins * (timeSpanSecs) - preSecs;
		avgFiringRateHz[k] = 0;
	}

}

ConditionPSTH::ConditionPSTH(const ConditionPSTH& c)
{
	conditionID = c.conditionID;
	postSecs = c.postSecs;
	preSecs = c.preSecs;
	numTrials = c.numTrials;
	maxTrialTimeSec  = c.maxTrialTimeSec;
	binResolutionMS = c.binResolutionMS;
	numBins = c.numBins;
	avgFiringRateHz=c.avgFiringRateHz;
	numDataPoints=c.numDataPoints;
	timeSpanSecs = c.timeSpanSecs;
}


void ConditionPSTH::clear()
{
	numTrials= 0;
}

void ConditionPSTH::updatePSTH(SmartSpikeCircularBuffer *spikeBuffer, Trial *trial)
{
	Time t;
	float ticksPerSec =t.getHighResolutionTicksPerSecond();
	std::vector<int64> alignedSpikes = spikeBuffer->getAlignedSpikes(trial, preSecs, postSecs);
	std::vector<float> instantaneousSpikesRate;
	instantaneousSpikesRate.resize(numBins);
	for (int k = 0; k < numBins; k++)
	{
		instantaneousSpikesRate[k] = 0;
	}

	for (int k=0;k<alignedSpikes.size();k++)
	{
		// spike times are aligned relative to trial alignment (i.e.) , onset is at "0"
		// convert ticks back to seconds, then to bins.
		float spikeTimeSec = float(alignedSpikes[k]) / ticksPerSec;
		int binIndex = (spikeTimeSec + preSecs) / timeSpanSecs * numBins;
		if (binIndex >= 0 && binIndex < numBins)
		{
			instantaneousSpikesRate[binIndex] += 1.0;
		}
	}

	float lastUpdateTS = ((int64)trial->endTS-(int64)trial->alignTS) / ticksPerSec + postSecs;
	int lastBinIndex = (lastUpdateTS + preSecs) / timeSpanSecs * numBins;

	numTrials++;
	// Update average firing rate, up to when the trial ended. 
	for (int k = 0; k < lastBinIndex; k++)
	{
		numDataPoints[k]++;
		avgFiringRateHz[k] = ((numDataPoints[k] - 1) * avgFiringRateHz[k] + instantaneousSpikesRate[k]) / numDataPoints[k];
	}
}

/***********************/
Condition::Condition()
{
	conditionID = 0;
}

Condition::Condition(const Condition &c)
{
       name = c.name;
	   colorRGB[0] = c.colorRGB[0];
	   colorRGB[1] = c.colorRGB[1];
	   colorRGB[2] = c.colorRGB[2];
	   trialTypes = c.trialTypes;
	   trialOutcomes = c.trialOutcomes;
	   postSec = c.postSec;
	   preSec = c.preSec;
	   visible = c.visible;
	   conditionID = c.conditionID;
}

Condition::Condition(std::vector<String> items)
{
	 int k=1; // skip the addcontision in location 0
	 name = "Unknown";
	 colorRGB[0] = 255;
 	 colorRGB[1] = 255;
     colorRGB[2] = 255;
	 postSec = preSec = 0.5;
	 conditionID = 0;

	 visible = true;
	 bool bTrialTypes = false;
	 bool bOutcomes = false;

	 while (k < items.size())
	 {
		 String lower_item = items[k].toLowerCase();
		 if (lower_item == "") {
			 k++;
			 continue;
		 }

		 if (lower_item == "name")
		 {
			 bTrialTypes = false;
			 bOutcomes = false;
			 k++;
			 name = items[k];
			 k++;
			 continue;
		 }
		 if (lower_item == "visible")
		 {
			 bTrialTypes = false;
			 bOutcomes = false;
			 k++;
			 visible = items[k].getIntValue() > 0;
			 k++;
			 continue;
		 }
		 if (lower_item == "trialtypes")
		 {
			 k++;
			 bTrialTypes = true;
			 bOutcomes = false;
			 continue;
		 }
		  if (lower_item == "outcomes")
		 {
			 k++;
			 bTrialTypes = false;
			 bOutcomes = true;
			 continue;
		 }
		 if (bOutcomes)
		 {
			 trialOutcomes.push_back(items[k].getIntValue());
			 k++;
		 } else  if (bTrialTypes) {
			 trialTypes.push_back(items[k].getIntValue());
			 k++;
		 } else {
			 // unknown parameter!
			 // skip ?
			 k++;
		 }

	 }
}

Condition::Condition(String Name, std::vector<int> types, std::vector<int> outcomes, double _postSec, double _preSec)
{
       name = Name;
       colorRGB[0] = colorRGB[1] = colorRGB[2] = 255;
       trialTypes = types;
       trialOutcomes = outcomes;
	   postSec = _postSec;
	   preSec = _preSec;
       visible = true;
}

/**********************************************/
UnitPSTHs::UnitPSTHs(int ID, float maxTrialTimeSeconds, int maxTrialsInMemory):  unitID(ID), spikeBuffer(maxTrialTimeSeconds,maxTrialsInMemory)
{

}

void UnitPSTHs::addTrialStartToSmartBuffer(Trial *t)
{
	spikeBuffer.addTrialStartToBuffer(t);
}

void UnitPSTHs::clearStatistics()
{
	for (int k=0;k<conditionPSTHs.size();k++)
	{
		conditionPSTHs[k].clear();
	}
}

void UnitPSTHs::addSpikeToBuffer(uint64 spikeTimestampSoftware)
{
	spikeBuffer.addSpikeToBuffer(spikeTimestampSoftware);
}


void UnitPSTHs::updateConditionsWithSpikes(std::vector<int> conditionsNeedUpdating, Trial* trial)
{
	for (int k=0;k<conditionPSTHs.size();k++) {
		for (int j=0;j<conditionsNeedUpdating.size();j++) 
		{
			if (conditionPSTHs[k].conditionID == conditionsNeedUpdating[j]) 
			{
				// this condition needs to be updated.
				conditionPSTHs[k].updatePSTH(&spikeBuffer, trial);
			}
		}
	}

}

/********************/
Trial::Trial()
{
	outcome = type = -1;
	trialInProgress = false;
	trialID = startTS = alignTS = endTS = 0;
}

Trial::Trial(const Trial &t)
{
	outcome = t.outcome;
	type = t.type;
	trialID = t.trialID;
	startTS = t.startTS;
	alignTS = t.alignTS;
	endTS = t.endTS;
	trialInProgress = t.trialInProgress;
}
/***************************/
ElectrodePSTH::ElectrodePSTH(int ID) : electrodeID(ID)
{

}
/*************************/
SmartSpikeCircularBuffer::SmartSpikeCircularBuffer(float maxTrialTimeSeconds, int _maxTrialsInMemory)
{
	int MaxFiringRateHz = 300;
	maxTrialsInMemory = _maxTrialsInMemory;
	bufferSize = MaxFiringRateHz * maxTrialTimeSeconds * maxTrialsInMemory;

	bufferIndex = 0;
	trialIndex = 0;
	numSpikesStored = 0;
	numTrialsStored = 0;
	spikeTimesSoftware.resize(bufferSize);

	for (int k=0;k<bufferSize;k++)
		spikeTimesSoftware[k] = 0;

	trialID.resize(maxTrialsInMemory);
	pointers.resize(maxTrialsInMemory);
	for (int k=0;k<maxTrialsInMemory;k++)
	{
		trialID[k] = pointers[k] = 0;
	}

}

void SmartSpikeCircularBuffer::addSpikeToBuffer(uint64 spikeTimeSoftware)
{
	spikeTimesSoftware[bufferIndex] = spikeTimeSoftware;
	bufferIndex = (bufferIndex+1) % bufferSize;
	numSpikesStored++;
	if (numSpikesStored>bufferSize)
		numSpikesStored=numSpikesStored;
}


void SmartSpikeCircularBuffer::addTrialStartToBuffer(Trial *t)
{
	trialID[trialIndex] = t->trialID;
	pointers[trialIndex] = bufferIndex;
	trialIndex = (trialIndex+1) % maxTrialsInMemory;
	numTrialsStored++;
	if (numTrialsStored > maxTrialsInMemory)
		numTrialsStored = numTrialsStored;
}

int SmartSpikeCircularBuffer::queryTrialStart(int ID)
{
	for (int k=0;k<numTrialsStored;k++) 
	{
		int whereToLook = trialIndex-1-k;
		if (whereToLook < 0)
			whereToLook += maxTrialsInMemory;

		if (trialID[whereToLook] == ID)
			return pointers[whereToLook];
	}
	// trial not found?!!?!?
	return -1;
}



std::vector<int64> SmartSpikeCircularBuffer::getAlignedSpikes(Trial *trial, float preSecs, float postSecs)
{
	// we need to update the average firing rate with the spikes that were stored in the spike buffer.
	// first, query spike buffer where does the trial start....
	std::vector<int64> alignedSpikes;
	Time t;
	uint64 numTicksPreTrial =preSecs * t.getHighResolutionTicksPerSecond(); 
	uint64 numTicksPostTrial =postSecs * t.getHighResolutionTicksPerSecond();
	int saved_ptr = queryTrialStart(trial->trialID);
	if (saved_ptr < 0)
		return alignedSpikes; // trial is not in memory??!?


	// return all spikes within a given interval aligned to AlignTS
	// Use ptr as a search reference in the buffer
	// The interval is defined as Start_TS-BeforeSec .. End_TS+AfterSec

	// Search Backward
	int CurrPtr = saved_ptr;
	int  N = 0;
	while (N < numSpikesStored)
	{
		if (spikeTimesSoftware[CurrPtr] < trial->startTS-numTicksPreTrial || spikeTimesSoftware[CurrPtr] > trial->endTS+numTicksPostTrial) 
			break;
		// Add spike..
		alignedSpikes.push_back((int64)spikeTimesSoftware[CurrPtr]-(int64)trial->alignTS);
		CurrPtr--;
		N++;
		if (CurrPtr < 0)
			CurrPtr = bufferSize-1;
	}
	// Now Search Forward
	CurrPtr = saved_ptr + 1;

	while (N < numSpikesStored)
	{
		if (CurrPtr >= bufferSize)
			CurrPtr = 0;

		if (spikeTimesSoftware[CurrPtr] > trial->endTS + numTicksPostTrial || CurrPtr==bufferIndex)
			break;
		// Add spike..
		if (spikeTimesSoftware[CurrPtr] - trial->startTS >= -numTicksPreTrial)
		{
			alignedSpikes.push_back((int64)spikeTimesSoftware[CurrPtr] - (int64)trial->alignTS);
			N++;
		}
		CurrPtr++;

	}
	std::sort(alignedSpikes.begin(),alignedSpikes.begin()+alignedSpikes.size());
	return alignedSpikes;
}

/**********************/

TrialCircularBuffer::~TrialCircularBuffer()
{

}

TrialCircularBuffer::TrialCircularBuffer(PeriStimulusTimeHistogramNode *p) : processor(p)
{
	Time t;
	maxTrialTimeSeconds = 10.0;
	MaxTrialTimeTicks = t.getHighResolutionTicksPerSecond() * maxTrialTimeSeconds;
	conditionCounter = 0;
	addDefaultTTLconditions = false;
	postSec =  preSec = 0.5;
	trialCounter = 0;
	maxTrialsInMemory = 200;
	/*
	numTrials = 0;
	numCh = 16;
	numTTLch = 16;
	int SamplingRate = 30000;
	TTL_Trial_Length_Sec = 1;
	TTL_Trial_Inhibition_Sec = 1;
	AfterSec = 0.5;
	BeforeSec = 0.5;

	AfterSec = 0.5;
	BeforeSec = 0.5;

	int SubSampling = 10; // 2.5 kHz is more than enough for LFP...

    spikeBuffer.resize(numCh);
//	lfpBuffer = new ContinuousCircularBuffer(numCh, SamplingRate, SubSampling, 2*MAX_TRIAL_TIME_SEC);

    lastTTLtrialTS.resize(numTTLch);
    for (int k = 0; k < numTTLch; k++)
    {
		lastTTLtrialTS[k] = 0;
    }

    //AddDefaultTTLConditions();

//	startThread();
*/
}



std::vector<String> TrialCircularBuffer::splitString(String S, char sep)
{
	std::list<String> ls;
	String  curr;
	for (int k=0;k < S.length();k++) {
		if (S[k] != sep) {
			curr+=S[k];
		}
		else
		{
			ls.push_back(curr);
			while (S[k] == sep && k < S.length())
				k++;

			curr = "";
			if (S[k] != sep && k < S.length())
				curr+=S[k];
		}
	}
	if (S[S.length()-1] != sep)
		ls.push_back(curr);

	 std::vector<String> Svec(ls.begin(), ls.end()); 
	return Svec;

}

void TrialCircularBuffer::lockPSTH()
{
	psthMutex.enter();
}

void TrialCircularBuffer::unlockPSTH()
{
	psthMutex.exit();
}


void TrialCircularBuffer::lockConditions()
{
	conditionMutex.enter();
}

void TrialCircularBuffer::unlockConditions()
{
	conditionMutex.exit();
}

void TrialCircularBuffer::addDefaultTTLConditions()
{

}

  void TrialCircularBuffer::parseMessage(StringTS msg)
  {
	  std::vector<String> input = splitString(msg.getString(),' ');
	  String command = input[0].toLowerCase();
	  if (command == "newelectrode")
	  {
		  ElectrodePSTH e(input[1].getIntValue());
		  int numChannels = input[2].getIntValue();
		  for (int k=0;k<numChannels;k++) {
			  e.channels.push_back(input[k+3].getIntValue());
		  }
		  electrodesPSTH.push_back(e);
		  ((PeriStimulusTimeHistogramEditor *) processor->getEditor())->updateCanvas();
	  } else if (command == "newunit")
	  {
		  int electrodeID = input[1].getIntValue();
		  int unitID = input[2].getIntValue();

		  // build a new PSTH for all defined conditions
		  UnitPSTHs unitPSTHs(unitID, maxTrialTimeSeconds, maxTrialsInMemory);
		  for (int k=0;k<conditions.size();k++)
		  {
			  unitPSTHs.conditionPSTHs.push_back(ConditionPSTH(conditions[k].conditionID,maxTrialTimeSeconds,preSec,postSec));
		  }
		  for (int k=0;k<electrodesPSTH.size();k++) {
			  if (electrodesPSTH[k].electrodeID == electrodeID) {
				  electrodesPSTH[k].unitsPSTHs.push_back(unitPSTHs);
				  break;
			  }
		  }
		  ((PeriStimulusTimeHistogramEditor *) processor->getEditor())->updateCanvas();
		  // inform editor repaint is needed
	  } else if (command == "removeunit")
	  {
		  lockPSTH();
		    int electrodeID = input[1].getIntValue();
	   	    int unitID = input[2].getIntValue();
			for (int e =0;e<electrodesPSTH.size();e++)
			{
				if (electrodesPSTH[e].electrodeID == electrodeID)
				{
					for (int u=0;u<electrodesPSTH[e].unitsPSTHs.size();u++)
					{
						if (electrodesPSTH[e].unitsPSTHs[u].unitID == unitID)
						{
							electrodesPSTH[e].unitsPSTHs.erase(electrodesPSTH[e].unitsPSTHs.begin()+u);
						}
					}
				}
			}
		  unlockPSTH();
		  ((PeriStimulusTimeHistogramEditor *) processor->getEditor())->updateCanvas();
		  // inform editor repaint is needed
	  } else if (command == "removeelectrode")
	  {
			 lockPSTH();
		    int electrodeID = input[1].getIntValue();
			for (int e =0;e<electrodesPSTH.size();e++)
			{
				if (electrodesPSTH[e].electrodeID == electrodeID)
				{
					electrodesPSTH.erase(electrodesPSTH.begin() + e);
				}
			}
		  unlockPSTH();
		  // inform editor repaint is in order
			((PeriStimulusTimeHistogramEditor *) processor->getEditor())->updateCanvas();
	  } else if (command == "trialstart")
	  {
		  currentTrial.trialID = ++trialCounter;
		  currentTrial.startTS = msg.timestamp;
  		  currentTrial.trialInProgress = true;

		  for (int i=0;i<electrodesPSTH.size();i++) 
			{
				for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
				{
					electrodesPSTH[i].unitsPSTHs[u].addTrialStartToSmartBuffer(&currentTrial);
				}
		  }
		  


		  //currentTrial.SetBufferPtrAtTrialOnset(spikeBuffer,lfpBuffer);
	  } else if (command == "trialend") 
	  {
		  currentTrial.endTS = msg.timestamp;
		  currentTrial.trialInProgress = false;
		  if (currentTrial.type >= 0 && currentTrial.startTS > 0 &&  currentTrial.endTS - currentTrial.startTS < MaxTrialTimeTicks)
		  {
			  if (currentTrial.alignTS == 0) 
			  {
				  currentTrial.alignTS = currentTrial.startTS;
			  }

			  aliveTrials.push(Trial(currentTrial));
		  }
	  } else if (command == "trialtype")
	  {
		  if (input.size() > 1) {
			  currentTrial.type = input[1].getIntValue();
		  }
	  } else if (command == "trialoutcome")
	  {
		  if (input.size() > 1) {
			  currentTrial.outcome = input[1].getIntValue();
		  }
	  } else if (command == "trialalign")
	  {
		  currentTrial.alignTS = msg.timestamp;
	  }  else if (command == "newdesign")
	  {
		  designName = input[1];
	  }
	  else if (command == "cleardesign")
	  {
		  	lockConditions();
			conditions.clear();
			conditionCounter = 0;
			PeriStimulusTimeHistogramEditor* edt = (PeriStimulusTimeHistogramEditor*) processor->getEditor();
			edt->updateCondition(conditions);
			unlockConditions();
			// clear conditions from all units
			lockPSTH();
			for (int i=0;i<electrodesPSTH.size();i++) 
			{
				for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
				{
					electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs.clear();
				}
			}
			unlockPSTH();
			if (addDefaultTTLconditions)
				addDefaultTTLConditions();

			// inform editor repaint is needed
	  } else if (command == "addcondition")
	  {
		  Condition newcondition(input);
		  lockConditions();
		  newcondition.conditionID = ++conditionCounter;
		  conditions.push_back(newcondition);

		  PeriStimulusTimeHistogramEditor* edt = (PeriStimulusTimeHistogramEditor*) processor->getEditor();
		  edt->updateCondition(conditions);

		  unlockConditions();
		  // now add a new psth for this condition for all sorted units on all electrodes
		  lockPSTH();
		  for (int i=0;i<electrodesPSTH.size();i++) 
		  {
			  for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
			  {
				  electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs.push_back(ConditionPSTH(newcondition.conditionID,maxTrialTimeSeconds,preSec,postSec));
			  }
		  }
		  unlockPSTH();
		  // inform editor repaint is needed
	  }

  }

void TrialCircularBuffer::addSpikeToSpikeBuffer(SpikeObject newSpike)
{
	for (int e=0;e<electrodesPSTH.size();e++)
	{
		if (electrodesPSTH[e].electrodeID == newSpike.electrodeID)
		{
			for (int u=0;u<electrodesPSTH[e].unitsPSTHs.size();u++)
			{
				if (electrodesPSTH[e].unitsPSTHs[u].unitID == newSpike.sortedId)
				{
					electrodesPSTH[e].unitsPSTHs[u].addSpikeToBuffer(newSpike.timestamp_software);
					return;
				}
			}
			
		}
	}
	// get got a sorted spike event before we got the information about the new unit?!?!?!
	
}


bool TrialCircularBuffer::contains(std::vector<int> v, int x)
{
	for (int k=0;k<v.size();k++)
		if (v[k] == x)
			return true;
	return false;
}

void TrialCircularBuffer::updateUnitsWithTrial(Trial *trial)
{
	lockConditions();
	lockPSTH();

	// find out which conditions need to be updated
	std::vector<int> conditionsNeedUpdating;
	for (int c=0;c<conditions.size();c++)
	{
		if (contains(conditions[c].trialTypes, trial->type) &&
			( (conditions[c].trialOutcomes.size() == 0) || 
			(conditions[c].trialOutcomes.size() > 0 && contains(conditions[c].trialOutcomes, trial->outcome))))
			conditionsNeedUpdating.push_back(conditions[c].conditionID);
	}

	if (conditionsNeedUpdating.size() == 0)
	{
		// none of the conditions match. nothing to update.
		unlockPSTH();
		unlockConditions();

		return;
	}

	for (int i=0;i<electrodesPSTH.size();i++) 
	{
		for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
		{
			electrodesPSTH[i].unitsPSTHs[u].updateConditionsWithSpikes(conditionsNeedUpdating,trial);

		}
	}

	unlockPSTH();
	unlockConditions();
}

void TrialCircularBuffer::process()
{
	Time t;
	if (aliveTrials.size() > 0)
	{
		Trial topTrial = aliveTrials.front();
		uint64 ticksElapsed = t.getHighResolutionTicks() - topTrial.endTS;
		float timeElapsedSec = float(ticksElapsed)/ t.getHighResolutionTicksPerSecond();
		if (timeElapsedSec > postSec)
		{
			aliveTrials.pop();
			updateUnitsWithTrial(&topTrial);

			// update display
			//updateTrialWithLFPData(TopTrial);
			
		}
	}

}








/*******************************/


/*


template <class T> void thread_safe_queue<T>::lock() 
{
	c.enter();
}

 template <class T> void thread_safe_queue<T>::release() 
 {
	 c.exit();
 }

template <class T> T thread_safe_queue<T>::front()
{
	c.enter();
	T t = q.front();
	c.exit();
	return t;
}

template <class T> T thread_safe_queue<T>::pop()
{
	c.enter();
	T t = q.pop();
	c.exit();
	return t;
}

template <class T> void thread_safe_queue<T>::push(const T& t)
{
	c.enter();
	q.push(t);
	c.exit();
}
*/
/***************/

/*
std::vector<double> UnitPSTH::GetSmoothPSTH(double fGaussianWidthMS)
{
	const double PI = 3.14159265359;
	fGaussianWidthMS /= binResolutionMS;
	int gaussianSupportMS = (ceil(fGaussianWidthMS)*7);
	std::vector<double> gaussianKernel;
	gaussianKernel.resize(1 + gaussianSupportMS);

	double sum = 0;
	for (int k = 0; k < 1 + gaussianSupportMS; k++)
	{
		double x = k - gaussianSupportMS / 2;

		gaussianKernel[k] = exp(-0.5 * ((x / (fGaussianWidthMS * fGaussianWidthMS)))) / (sqrt(2 * PI) * fGaussianWidthMS);
		sum += gaussianKernel[k];
	}
	for (int k = 0; k < 1 + gaussianSupportMS; k++)
	{
		gaussianKernel[k] /= sum;
	}

	std::vector<double> smoothFiringRate;
	jassertfalse;
	//NEED TO IMPLEMENT THIS...
	//alglib.alglib.convr1dcircular(avg_firing_rate_Hz, avg_firing_rate_Hz.Length,
	//gaussianKernel, gaussianKernel.Length, out smoothFiringRate);
	
	return smoothFiringRate;
}
*/

/*******************************/


/*
ChannelPSTH::ChannelPSTH(int NumCh, double AfterSec, double BeforeSec)
        {
            numCh = NumCh;
            binResolutionMS = 10;
            afterSec = AfterSec;
            beforeSec = BeforeSec;
            numBins = ((afterSec + beforeSec) * 1000.0) / binResolutionMS; // 1 ms resolution
            int NumBinsBefore = ((beforeSec) * 1000.0) / binResolutionMS; // 1 ms resolution
            time.resize(numBins);
            numAvgPoints.resize(numCh);
			avg_lfp.resize(numCh);
			for (int k=0;k<numCh;k++) 
			{
				numAvgPoints[k].resize(numBins);
				avg_lfp[k].resize(numBins);
			}	
        
            for (int k = 0; k < numBins; k++)
            {
                time[k] = (double)k / numBins * (afterSec + beforeSec) - beforeSec;
                for (int ch = 0; ch < numCh; ch++)
                {
                    avg_lfp[ch][k] = 0;
                    numAvgPoints[ch][k] = 0;
                }
            }
        }


std::vector<int> ChannelPSTH::histc(std::vector<double> xi, std::vector<double> x)
{
	std::vector<int> aiInd(xi.size());

	int i = 0;
	int j = 0;
	int N = x.size();
	while (i < xi.size()) 
	{
		if (xi[i] < x[j] || xi[i] > x[N-1])
		{
			aiInd[i] = -1;
			i++;
			continue;
		}
		if (j + 1 < N && xi[i] >= x[j] && xi[i] < x[j + 1])
		{
			aiInd[i] = j;
			i++;
			continue;
		}
		j++;
		if (j > N - 1)
			j = N - 1;
	}
	return aiInd;
}

std::vector<double> ChannelPSTH::diff(std::vector<double> x)
{
	std::vector<double> d(x.size()-1);
	for (int k = 0; k < x.size() - 1; k++)
	{
		d[k] = x[k + 1] - x[k];
	}
	return d;
}

void ChannelPSTH::interp1(std::vector<double> x, std::vector<std::vector<double>>y, std::vector<double> xi, std::vector<std::vector<double>> &yi, std::vector<bool> &valid)
{
	// linear interpolate
	int N = x.size();
	int M = xi.size();

	valid.resize(xi.size());
	yi.resize(numCh);
	for (int k=0;k<numCh;k++)
		yi[k].resize(xi.size());

	if (x.size()== 0)
		return;

	std::vector<int> ind = histc(xi, x);
	std::vector<double> h = diff(x);

	for (int ch = 0; ch < numCh; ch++)
	{
		for (int i = 0; i < xi.size(); i++)
		{
			if (ind[i] < 0)
			{
				// invalid entry
				valid[i] = false;
				yi[ch][i] = 0;
				continue;
			}
			valid[i] = true;

			double s = (xi[i] - x[ind[i]]) / h[ind[i]];
			yi[ch][i] = y[ch][ind[i]] + s * (y[ch][ind[i] + 1] - y[ch][ind[i]]);
		}
	}
}


void ChannelPSTH::Update(const Trial &t)
{
	std::vector<std::vector<double>> interpolated_lfp;
	std::vector<bool> valid_values;
	interp1(t.lfp.time,t.lfp.data,time, interpolated_lfp, valid_values);
	for (int k = 0; k < valid_values.size(); k++)
	{
		for (int ch = 0; ch < numCh; ch++)
		{
			numAvgPoints[ch][k]++;
			if (valid_values[k])
			{
				avg_lfp[ch][k] = ((numAvgPoints[ch][k] - 1) * avg_lfp[ch][k] + interpolated_lfp[ch][k]) / numAvgPoints[ch][k];
			}
		}
	}
}

  */
/********************************/


/*
 void TrialCircularBuffer::AddDefaultTTLConditions()
{
	std::vector<int> AllOutcomes;

    for (int ch = 0; ch < numTTLch; ch++)
    {
		std::vector<int> TrialTypes(1);
        TrialTypes[0] = 30000 + ch;
        Condition c(numCh, String("TTL ")+ String(ch+1), TrialTypes, AllOutcomes, AfterSec + MAX_TRIAL_TIME_SEC, BeforeSec);
        AddCondition(c);
    }
}
*/
/*

	} else if (cmd ==  "ttl")
	{
		// inject a fake trial...
		int ch = split[1].getIntValue();
		int value = split[2].getIntValue();

		juce::Time timer;
		double curr_time = double(timer.getHighResolutionTicks()) / double(timer.getHighResolutionTicksPerSecond()); 
	

		if (value > 0 && ((curr_time - lastTTLtrialTS[ch]) > TTL_Trial_Inhibition_Sec)) 
		{
			ttlTrial.Type = 30000+ch;
			ttlTrial.Start_TS = E.soft_ts;
			ttlTrial.Align_TS = E.soft_ts;
			ttlTrial.End_TS = E.soft_ts + TTL_Trial_Length_Sec;
			ttlTrial.Outcome = -1; // will be ignored anyway...
			ttlTrial.SetBufferPtrAtTrialOnset(spikeBuffer, lfpBuffer);

			AliveTrials.push(Trial(ttlTrial));
			lastTTLtrialTS[ch] = double(timer.getHighResolutionTicks()) / double(timer.getHighResolutionTicksPerSecond());
		}
		*/
