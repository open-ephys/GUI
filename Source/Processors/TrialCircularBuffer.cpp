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
	avgResponse.resize(numBins);
	numDataPoints.resize(numBins);
	binTime.resize(numBins);
	
	for (int k = 0; k < numBins; k++)
	{
		numDataPoints[k] = 0;
		binTime[k] = (float)k / numBins * (timeSpanSecs) - preSecs;
		avgResponse[k] = 0;
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
	avgResponse=c.avgResponse;
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
		avgResponse[k] = ((numDataPoints[k] - 1) * avgResponse[k] + instantaneousSpikesRate[k]) / numDataPoints[k];
	}
}

void ConditionPSTH::updatePSTH(std::vector<float> alignedLFP,std::vector<float> valid)
{
	numTrials++;

	// Update average firing rate, up to when the trial ended. 
	for (int k = 0; k < valid.size(); k++)
	{
		if (!valid[k])
			break;

		numDataPoints[k]++;
		avgResponse[k] = ((numDataPoints[k] - 1) * avgResponse[k] + alignedLFP[k]) / numDataPoints[k];
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


ChannelPSTHs::ChannelPSTHs(int ID, float maxTrialTimeSeconds, int maxTrialsInMemory, float presecs, float postsecs, int binResolutionMS) : channelID(ID),
	preSecs(presecs), postSecs(postsecs)
{
	float timeSpanSecs=preSecs + postSecs + maxTrialTimeSeconds;
	int numBins = (timeSpanSecs) * 1000.0f / binResolutionMS; // 1 ms resolution
	binTime.resize(numBins);	
	for (int k = 0; k < numBins; k++)
	{
		binTime[k] = (float)k / numBins * (timeSpanSecs) - preSecs;
	}

}

void ChannelPSTHs::updateConditionsWithLFP(std::vector<int> conditionsNeedUpdating, std::vector<float> alignedLFP, std::vector<float> valid)
{
	if (conditionsNeedUpdating.size() == 0)
		return ;

	for (int k=0;k<conditionPSTHs.size();k++) {
		for (int j=0;j<conditionsNeedUpdating.size();j++) 
		{
			if (conditionPSTHs[k].conditionID == conditionsNeedUpdating[j]) 
			{
				// this condition needs to be updated.
				conditionPSTHs[k].updatePSTH(alignedLFP,valid);
			}
		}
	}

}

void ChannelPSTHs::clearStatistics()
{
	for (int k=0;k<conditionPSTHs.size();k++)
		conditionPSTHs[k].clear();
}

/***********************************************/
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

void ElectrodePSTH::updateChannelsConditionsWithLFP(std::vector<int> conditionsNeedUpdate, Trial *trial, SmartContinuousCircularBuffer *lfpBuffer)
{
	// compute trial aligned lfp for all channels 
	std::vector<float> valid;
	std::vector<std::vector<float>> alignedLFP;
	// resample all electrode channels 
	lfpBuffer->getAlignedData(channels,trial,&channelsPSTHs[0].binTime,
		channelsPSTHs[0].preSecs,channelsPSTHs[0].postSecs, alignedLFP,valid);
	// now we can average data

	for (int ch=0;ch<channelsPSTHs.size();ch++)
	{
		channelsPSTHs[ch].updateConditionsWithLFP(conditionsNeedUpdate, alignedLFP[ch], valid);
	}

}

/****************************************/






void SmartContinuousCircularBuffer::addTrialStartToSmartBuffer(int trialID)
{
	smartPointerIndex[trialptr] = ptr;
	smartPointerTrialID[trialptr] = trialID;
	trialptr++;
	if (trialptr>numTrials)
		trialptr = 0;
}

SmartContinuousCircularBuffer::SmartContinuousCircularBuffer(int NumCh, float SamplingRate, int SubSampling, float NumSecInBuffer) :
		ContinuousCircularBuffer(NumCh, SamplingRate, SubSampling, NumSecInBuffer)
{
	trialptr = 0;
	numTrials = 50;
	smartPointerIndex.resize(numTrials); // Number of trials to keep in memory
	smartPointerTrialID.resize(numTrials);
	for (int k=0;k<numTrials;k++)
	{
		smartPointerIndex[k] = 0;
		smartPointerTrialID[k] = 0;
	}
}


void SmartContinuousCircularBuffer::getAlignedData(std::vector<int> channels, Trial *trial, std::vector<float> *timeBins,
												   float preSec, float postSec,
									std::vector<std::vector<float>> &output,
									std::vector<float> &valid)
{
	// to update a condition's continuous data psth, we will first find 
	// data samples in the vicinity of the trial, and then interpolate at the
	// needed time bins.

	int numTimeBins = timeBins->size();

	output.resize(channels.size());
	valid.resize(numTimeBins);
	for (int ch=0;ch<channels.size();ch++)
	{
		output[ch].resize(numTimeBins);

		for (int j=0;j<numTimeBins;j++)
			output[ch][j] = 0;
	}
	for (int i=0;i<numTimeBins;i++)
	{
		valid[i] = false;
	}

	// 1. instead of searching the entire buffer, query when did the trial started....
	int k = 0;
	for (;k<numTrials;k++)
	{
		if (smartPointerTrialID[k] == trial->trialID)
		{
			break;
		}
	}
	if (k == 0) 
	{
		// couldn't find the trial !?!?!? buffer overrun?
		return;
	}

	// now we have a handle where to search the data...

	int p=smartPointerIndex[k];
	
	// go backward and find the find time stamp we will need.
	int nSamples = 1;
	int search_back_ptr = p;
	for (int q=0;q<numSamplesInBuf;q++) 
	{
		if (softwareTS[search_back_ptr] < trial->alignTS - int64(preSec*numTicksPerSecond))
		{
			// we found the first sample prior to required trial alignment
			break;
		}
		search_back_ptr--;
		nSamples++;
		if (search_back_ptr < 0)
			search_back_ptr=bufLen-1;
	}

	// go forward and find the last time stamp we will need.
	int search_forward_ptr = p;
	for (int q=0;q<numSamplesInBuf;q++) 
	{
		if (softwareTS[search_forward_ptr] > trial->endTS + int64(postSec*numTicksPerSecond))
		{
			// we found the first sample prior to required trial alignment
			break;
		}
		nSamples++;
		search_forward_ptr++;
		if (search_forward_ptr == bufLen)
			search_forward_ptr=0;
	}
	jassert(numSamplesInBuf > 1);

	// we would like to return the lfp, sampled at specific time bins
	// (typically, 1 ms resolution, which is an overkill).
	//

	// we know that softwareTS[search_back_ptr]-trialAlign < preSec.
	// and that softwareTS[search_back_ptr+1]-trialAlign > preSec.

	int index=search_back_ptr;
	int index_next=index+1;
	if (index_next > bufLen)
		index_next = 0;

	float tA = ((float)softwareTS[index]-(float)trial->alignTS)/numTicksPerSecond;
	float tB = ((float)softwareTS[index_next]-(float)trial->alignTS)/numTicksPerSecond;
	float trial_length_sec = ((float)trial->endTS-(float)trial->alignTS)/numTicksPerSecond;
	for (int i = 0;i < numTimeBins; i++)
	{
		float tSamlple = (*timeBins)[i];
		if (tSamlple > trial_length_sec + postSec)
		{
			// do not update  after trial ended
			break;
		}
		float dA = tSamlple-tA;
		float dB = tB-tSamlple;
		float fracA = dA/(dA+dB);
		valid[i] = true;
		for (int ch=0;ch<channels.size();ch++)
		{
			output[ch][i] =  Buf[channels[ch]][index] * (1-fracA) +  Buf[channels[ch]][index_next] * (fracA);
		}
		// now advance pointers if needed
		if (i < numTimeBins-1) 
		{
			float tSamlple_next = (*timeBins)[i+1];
			int cnt = 0;
			while (cnt < bufLen)
			{
				if (tA <= tSamlple_next & tB > tSamlple_next)
				{
					break;
				} else
				{
					index++;
					if (index >= bufLen)
						index = 0;
					index_next=index+1;
					if (index_next >= bufLen)
						index_next = 0;

					tA = ((float)softwareTS[index]-(float)trial->alignTS)/numTicksPerSecond;
					tB = ((float)softwareTS[index_next]-(float)trial->alignTS)/numTicksPerSecond;
				}
				cnt++;
			}

			if (cnt == bufLen) 
			{
				jassertfalse;
				break; // missing data!?!?!?
			}
		}
	}

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

TrialCircularBuffer::TrialCircularBuffer(int numChannels, float samplingRate, PeriStimulusTimeHistogramNode *p) : processor(p)
{
	Time t;
	maxTrialTimeSeconds = 10.0;
	MaxTrialTimeTicks = t.getHighResolutionTicksPerSecond() * maxTrialTimeSeconds;
	conditionCounter = 0;
	addDefaultTTLconditions = false;
	postSec =  preSec = 0.5;
	trialCounter = 0;
	maxTrialsInMemory = 200;
	binResolutionMS = 1;

	float desiredSamplingRateHz = 600; // LFPs are usually in the range of 0-300 Hz, so
	// sampling them should be at least 600 Hz (Nyquist!)
	// We typically sample everything at 30000, so a sub-sampling by a factor of 50 should be good
	int subSample = samplingRate / desiredSamplingRateHz;
	float numSeconds = 2*maxTrialTimeSeconds;
	lfpBuffer = new SmartContinuousCircularBuffer(numChannels, samplingRate, subSample, numSeconds);

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
			  int channelID = input[k+3].getIntValue();
			  e.channels.push_back(channelID);
			  ChannelPSTHs channelPSTH(channelID,maxTrialTimeSeconds, maxTrialsInMemory,preSec,postSec,binResolutionMS);
			  // add all known conditions
			  for (int c=0;c<conditions.size();c++)
			  {
				  channelPSTH.conditionPSTHs.push_back(ConditionPSTH(conditions[c].conditionID,maxTrialTimeSeconds,preSec,postSec));
			  }
			  e.channelsPSTHs.push_back(channelPSTH);
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
		  lfpBuffer->addTrialStartToSmartBuffer(currentTrial.trialID);
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
				for (int ch=0;ch<electrodesPSTH[i].channelsPSTHs.size();ch++)
				{
					electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs.clear();
				}

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
			  for (int ch=0;ch<electrodesPSTH[i].channelsPSTHs.size();ch++)
			  {
				  electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs.push_back(ConditionPSTH(newcondition.conditionID,maxTrialTimeSeconds,preSec,postSec));
			  }

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


void TrialCircularBuffer::updatePSTHwithTrial(Trial *trial)
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


	// update both spikes and LFP PSTHs 
	for (int i=0;i<electrodesPSTH.size();i++) 
	{
		electrodesPSTH[i].updateChannelsConditionsWithLFP(conditionsNeedUpdating,trial, lfpBuffer);
		for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
		{
			electrodesPSTH[i].unitsPSTHs[u].updateConditionsWithSpikes(conditionsNeedUpdating,trial);

		}
	}

	unlockPSTH();
	unlockConditions();
}

void TrialCircularBuffer::reallocate(int numChannels)
{
	lfpBuffer->reallocate(numChannels);

}

void TrialCircularBuffer::process(AudioSampleBuffer& buffer,int nSamples,uint64 hardware_timestamp,uint64 software_timestamp)
{
	Time t;
	// first, update LFP circular buffers
	lfpBuffer->update(buffer, hardware_timestamp,software_timestamp);
	// now, check if a trial finished, and enough time has elapsed so we also
	// have post trial information
	if (electrodesPSTH.size() > 0 && aliveTrials.size() > 0)
	{
		Trial topTrial = aliveTrials.front();
		uint64 ticksElapsed = t.getHighResolutionTicks() - topTrial.endTS;
		float timeElapsedSec = float(ticksElapsed)/ t.getHighResolutionTicksPerSecond();
		if (timeElapsedSec > postSec)
		{
			aliveTrials.pop();
			updatePSTHwithTrial(&topTrial);
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
