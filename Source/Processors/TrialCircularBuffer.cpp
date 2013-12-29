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



void setDefaultColors(uint8 &R, uint8 &G, uint8 &B, int ID)
{
	int IDmodule = (ID-1) % 6; // ID can't be zero
	const int colors[6][3] = {
   {      0,         0,    255},
   {      0,    128,         0},
   { 255,         0,         0},
   {      0,    192,    192},
   { 192,         0,    192},
   { 192,    192,         0}};

	R = colors[IDmodule][0];
	G = colors[IDmodule][1];
	B = colors[IDmodule][2];

}


/******************************/
ConditionPSTH::ConditionPSTH(int ID, float _maxTrialTimeSec, float pre, float post, bool vis) : conditionID(ID), preSecs(pre), 
	postSecs(post), numTrials(0), maxTrialTimeSec(_maxTrialTimeSec), binResolutionMS(1),preSec(pre),postSec(post),visible(vis)
{
	// allocate data for 1 ms resolution bins to cover trials 
	xmin = -pre;
	xmax = post;
	ymax = -1e10;
	ymin = 1e10;
	setDefaultColors(colorRGB[0],colorRGB[1],colorRGB[2], ID);

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
	binTime = c.binTime;
	xmin = c.xmin;
	xmax = c.xmax;
	ymax = c.ymax;
	ymin = c.ymin;
	colorRGB[0] = c.colorRGB[0];
	colorRGB[1] = c.colorRGB[1];
	colorRGB[2] = c.colorRGB[2];
	visible = c.visible;
	preSec = c.preSec;
	postSec = c.postSec;
}


void ConditionPSTH::clear()
{
	numTrials= 0;
	xmin = -preSec;
	xmax = postSec;
	ymax = -1e10;
	ymin = 1e10;
	for (int k = 0; k < numBins; k++)
	{
		numDataPoints[k] = 0;
		avgResponse[k] = 0;
	}
	
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

	float lastUpdateTS = float(trial->endTS-trial->alignTS) / ticksPerSec + postSecs;
	int lastBinIndex = (int)( (lastUpdateTS + preSecs) / timeSpanSecs * numBins);


	xmax = MAX(xmax,lastUpdateTS);


	numTrials++;
	// Update average firing rate, up to when the trial ended. 
	ymax = -1e10;
	ymin = 1e10;

	 float scale = 1000.0;
	for (int k = 0; k < lastBinIndex; k++)
	{
		numDataPoints[k]++;
		avgResponse[k] = ((numDataPoints[k] - 1) * avgResponse[k] + scale*instantaneousSpikesRate[k]) / numDataPoints[k];
		ymax = MAX(ymax,avgResponse[k]);
		ymin = MIN(ymin,avgResponse[k]);
	}
}

void ConditionPSTH::getRange(float &xMin, float &xMax, float &yMin, float &yMax)
{
	xMin = xmin;
	yMax = ymax;
	xMax = xmax;
	yMin = ymin;
}

void ConditionPSTH::updatePSTH(std::vector<float> alignedLFP,std::vector<float> valid)
{
	numTrials++;

	ymax = -1e10;
	ymin = 1e10;
	xmin = -preSec;
	xmax = 0;

	// Update average firing rate, up to when the trial ended. 
	for (int k = 0; k < valid.size(); k++)
	{
		if (!valid[k]) {
			xmax = MAX(xmax, binTime[k]);
			break;
		}
		numDataPoints[k]++;
		avgResponse[k] = ((numDataPoints[k] - 1) * avgResponse[k] + alignedLFP[k]) / numDataPoints[k];
		ymax = MAX(ymax, avgResponse[k]);
		ymin = MIN(ymin, avgResponse[k]);
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
	   posX = c.posX;
	   posY = c.posY;
	   conditionGroup = c.conditionGroup;
	   conditionID = c.conditionID;
}

Condition::Condition(std::vector<String> items, int ID)
{
	 int k=1; // skip the addcontision in location 0
	 name = "Unknown";
	 setDefaultColors(colorRGB[0],colorRGB[1],colorRGB[2], ID);
	 postSec = preSec = 0.5;
	 conditionID = ID;

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
		  if (lower_item == "group")
		 {
			 bTrialTypes = false;
			 bOutcomes = false;
			 k++;
			 conditionGroup = items[k].getIntValue();
			 k++;
			 continue;
		 }
		if (lower_item == "spatialposition")
		 {
			 bTrialTypes = false;
			 bOutcomes = false;
			 k++;
			 posX = items[k].getFloatValue();
			 k++;
			 posY = items[k].getFloatValue();
			 k++;
			 continue;
		 }
		 
		 if (lower_item == "color")
		 {
			 bTrialTypes = false;
			 bOutcomes = false;
			 k++;
			 colorRGB[0] = items[k].getIntValue();
			 k++;
			 colorRGB[1] = items[k].getIntValue();
			 k++;
			 colorRGB[2] = items[k].getIntValue();
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
	redrawNeeded = true;
}

void ChannelPSTHs::updateConditionsWithLFP(std::vector<int> conditionsNeedUpdating, std::vector<float> alignedLFP, std::vector<float> valid)
{
	if (conditionsNeedUpdating.size() == 0)
		return ;

	redrawNeeded = true;
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

bool ChannelPSTHs::isNewDataAvailable()
{
	return redrawNeeded;
}

void ChannelPSTHs::informPainted()
{
	redrawNeeded = false;
}

void ChannelPSTHs::getRange(float &xmin, float &xmax, float &ymin, float &ymax)
{
	float xMin = 1e10;
	float xMax = -1e10;
	float yMin = 1e10;
	float yMax = -1e10;
	for (int k=0;k<conditionPSTHs.size();k++) 
	{
		if (conditionPSTHs[k].numTrials > 0 && conditionPSTHs[k].visible)
		{
			conditionPSTHs[k].getRange(xMin,xMax,yMin,yMax);
			xmin = MIN(xmin, xMin);
			ymin = MIN(ymin, yMin);
			xmax = MAX(xmax, xMax);
			ymax = MAX(ymax, yMax);
		}

	}
}


void ChannelPSTHs::clearStatistics()
{
	for (int k=0;k<conditionPSTHs.size();k++)
		conditionPSTHs[k].clear();
}

/***********************************************/
UnitPSTHs::UnitPSTHs(int ID, float maxTrialTimeSeconds, int maxTrialsInMemory, int sampleRateHz, uint8 R, uint8 G, uint8 B): unitID(ID), spikeBuffer(maxTrialTimeSeconds,maxTrialsInMemory,sampleRateHz)
{
	colorRGB[0] = R;
	colorRGB[1] = G;
	colorRGB[2] = B;
	redrawNeeded = false;
}

void UnitPSTHs::informPainted()
{
	redrawNeeded = false;
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

void UnitPSTHs::addSpikeToBuffer(int64 spikeTimestampSoftware, int64 spikeTimestampHardware)
{
	spikeBuffer.addSpikeToBuffer(spikeTimestampSoftware,spikeTimestampHardware);
}

bool UnitPSTHs::isNewDataAvailable()
{
	return redrawNeeded;
}

void UnitPSTHs::getRange(float &xmin, float &xmax, float &ymin, float &ymax)
{
	xmin = 1e10;
	xmax = -1e10;
	ymax = -1e10;
	ymin = 1e10;
	float minX, maxX, maxY, minY;
	for (int k=0;k<conditionPSTHs.size();k++) {
		if (conditionPSTHs[k].visible)
		{
			conditionPSTHs[k].getRange(minX, maxX, minY, maxY);
			xmin = MIN(xmin, minX);
			xmax = MAX(xmax, maxX);
			ymax = MAX(ymax, maxY);
			ymin = MIN(ymin, minY);
		}
	}
}


void UnitPSTHs::updateConditionsWithSpikes(std::vector<int> conditionsNeedUpdating, Trial* trial)
{
	redrawNeeded = true;
	if (conditionsNeedUpdating.size() == 0)
		return ;

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
	hardwareAlignment = false;
	alignTS_hardware = trialID = startTS = alignTS = endTS = 0;
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
	hardwareAlignment = t.hardwareAlignment;
	alignTS_hardware = t.alignTS_hardware;
}
/***************************/
ElectrodePSTH::ElectrodePSTH(int ID) : electrodeID(ID)
{

}

void ElectrodePSTH::updateChannelsConditionsWithLFP(std::vector<int> conditionsNeedUpdate, Trial *trial, SmartContinuousCircularBuffer *lfpBuffer)
{
	// compute trial aligned lfp for all channels 
	std::vector<float> valid;
	std::vector<std::vector<float> > alignedLFP;
	// resample all electrode channels 
	bool success = lfpBuffer->getAlignedData(channels,trial,&channelsPSTHs[0].binTime,
		channelsPSTHs[0].preSecs,channelsPSTHs[0].postSecs, alignedLFP,valid);
	// now we can average data
	if (success)
	{
		for (int ch=0;ch<channelsPSTHs.size();ch++)
		{
			channelsPSTHs[ch].updateConditionsWithLFP(conditionsNeedUpdate, alignedLFP[ch], valid);
		}
	}

}

/****************************************/






void SmartContinuousCircularBuffer::addTrialStartToSmartBuffer(int trialID)
{
	smartPointerIndex[trialptr] = ptr;
	smartPointerTrialID[trialptr] = trialID;
	trialptr++;
	if (trialptr>=numTrials)
		trialptr = 0;
}

SmartContinuousCircularBuffer::SmartContinuousCircularBuffer(int NumCh, float SamplingRate, int SubSampling, float NumSecInBuffer) :
		ContinuousCircularBuffer(NumCh, SamplingRate, SubSampling, NumSecInBuffer)
{
	trialptr = 0;
	numTrials = 100;
	smartPointerIndex.resize(numTrials); // Number of trials to keep in memory
	smartPointerTrialID.resize(numTrials);
	for (int k=0;k<numTrials;k++)
	{
		smartPointerIndex[k] = 0;
		smartPointerTrialID[k] = 0;
	}
}


bool SmartContinuousCircularBuffer::getAlignedData(std::vector<int> channels, Trial *trial, std::vector<float> *timeBins,
												   float preSec, float postSec,
									std::vector<std::vector<float> > &output,
									std::vector<float> &valid)
{
	// to update a condition's continuous data psth, we will first find 
	// data samples in the vicinity of the trial, and then interpolate at the
	// needed time bins.
	if (numSamplesInBuf <= 1 )
		return false;

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
	bool found = false;
	for (;k<numTrials;k++)
	{
		if (smartPointerTrialID[k] == trial->trialID)
		{
			found = true;
			break;
		}
	}
	if (!found) 
	{
		//jassertfalse;
		// couldn't find the trial !?!?!? buffer overrun?
		return false;
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
	
	// we would like to return the lfp, sampled at specific time bins
	// (typically, 1 ms resolution, which is an overkill).
	//

	// we know that softwareTS[search_back_ptr]-trialAlign < preSec.
	// and that softwareTS[search_back_ptr+1]-trialAlign > preSec.

	int index=search_back_ptr;

	int index_next=index+1;
	if (index_next >= bufLen)
		index_next = 0;

	float tA,tB;

	if (trial->hardwareAlignment)
	{
		tA = float(hardwareTS[index]-trial->alignTS_hardware)/(float)samplingRate;
		tB = float(hardwareTS[index_next]-trial->alignTS_hardware)/(float)samplingRate;
	} else
	{
		tA = float(softwareTS[index]-trial->alignTS)/numTicksPerSecond;
		tB = float(softwareTS[index_next]-trial->alignTS)/numTicksPerSecond;
	}
	float trial_length_sec = float(trial->endTS-trial->alignTS)/numTicksPerSecond;

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

					if (trial->hardwareAlignment)
					{
						tA = float(hardwareTS[index]-trial->alignTS)/samplingRate;
						tB = float(hardwareTS[index_next]-trial->alignTS)/samplingRate;
					} else
					{
						tA = float(softwareTS[index]-trial->alignTS)/numTicksPerSecond;
						tB = float(softwareTS[index_next]-trial->alignTS)/numTicksPerSecond;
					}

				}
				cnt++;
			}

			if (cnt == bufLen) 
			{
				// missing data. This can happen when we just add a channel and a trial is in progress?!?!?
				return false; 
			}
		}
	}
	return true;
}

/*************************/
SmartSpikeCircularBuffer::SmartSpikeCircularBuffer(float maxTrialTimeSeconds, int _maxTrialsInMemory, int _sampleRateHz)
{
	sampleRateHz = _sampleRateHz;
	int MaxFiringRateHz = 300;
	maxTrialsInMemory = _maxTrialsInMemory;
	bufferSize = MaxFiringRateHz * maxTrialTimeSeconds * maxTrialsInMemory;
	jassert(bufferSize > 0);

	bufferIndex = 0;
	trialIndex = 0;
	numSpikesStored = 0;
	numTrialsStored = 0;
	spikeTimesSoftware.resize(bufferSize);
	spikeTimesHardware.resize(bufferSize);

	for (int k=0;k<bufferSize;k++) 
	{
		spikeTimesSoftware[k] = 0;
		spikeTimesHardware[k] = 0;
	}

	trialID.resize(maxTrialsInMemory);
	pointers.resize(maxTrialsInMemory);
	for (int k=0;k<maxTrialsInMemory;k++)
	{
		trialID[k] = pointers[k] = 0;
	}

}

void SmartSpikeCircularBuffer::addSpikeToBuffer(int64 spikeTimeSoftware,int64 spikeTimeHardware)
{
	spikeTimesHardware[bufferIndex] = spikeTimeHardware;
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
		numTrialsStored = maxTrialsInMemory;
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
	jassert(spikeTimesSoftware.size() > 0);
	std::vector<int64> alignedSpikes;
	Time t;
	int64 ticksPerSec = t.getHighResolutionTicksPerSecond();
	int64 numTicksPreTrial =preSecs * ticksPerSec;
	int64 numTicksPostTrial =postSecs * ticksPerSec;

	int64 samplesToTicks = 1.0/float(sampleRateHz) * ticksPerSec;


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
			if (trial->hardwareAlignment)
				// convert from samples to ticks...
				alignedSpikes.push_back( (spikeTimesHardware[CurrPtr]-trial->alignTS_hardware)*samplesToTicks);
			else
				alignedSpikes.push_back(spikeTimesSoftware[CurrPtr]-trial->alignTS);

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

				if (trial->hardwareAlignment)
					alignedSpikes.push_back( (spikeTimesHardware[CurrPtr] - trial->alignTS_hardware) * samplesToTicks);
				else
					alignedSpikes.push_back(spikeTimesSoftware[CurrPtr] - trial->alignTS);

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
	delete lfpBuffer;
	lfpBuffer=nullptr;
	electrodesPSTH.clear();

}
TrialCircularBuffer::TrialCircularBuffer()
{
	lfpBuffer = nullptr;
	hardwareTriggerAlignmentChannel = -1;
}

void TrialCircularBuffer::setHardwareTriggerAlignmentChannel(int k)
{
	hardwareTriggerAlignmentChannel = k;
}

TrialCircularBuffer::TrialCircularBuffer(int numChannels, float samplingRate, PeriStimulusTimeHistogramNode *p) : processor(p)
{
	Time t;
	 numTicksPerSecond = t.getHighResolutionTicksPerSecond();

	maxTrialTimeSeconds = 10.0;
	MaxTrialTimeTicks = numTicksPerSecond * maxTrialTimeSeconds;
	conditionCounter = 0;
	addDefaultTTLconditions = true;
	firstTime = true;
	postSec =  preSec = 0.5;
	trialCounter = 0;
	maxTrialsInMemory = 200;
	binResolutionMS = 1;

	float desiredSamplingRateHz = 600; // LFPs are usually in the range of 0-300 Hz, so
	// sampling them should be at least 600 Hz (Nyquist!)
	// We typically sample everything at 30000, so a sub-sampling by a factor of 50 should be good
	samplingRateHz = samplingRate;
	int subSample = samplingRateHz / desiredSamplingRateHz;
	float numSeconds = 2*maxTrialTimeSeconds;
	lfpBuffer = new SmartContinuousCircularBuffer(numChannels, samplingRateHz, subSample, numSeconds);
	numTTLchannels = 8;
	lastTTLts.resize(numTTLchannels);

	 ttlSupressionTimeSec = 1.0;
	 ttlTrialLengthSec = 2;

	for (int k=0;k<numTTLchannels;k++)
		lastTTLts[k] = 0;
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

void TrialCircularBuffer::addDefaultTTLConditions(Array<bool> visibility)
{
	  int numExistingConditions = conditions.size();
	  for (int channel = 0; channel < numTTLchannels; channel++)
	  {
		  StringTS simulatedConditionString;
		  if (visibility[channel])
			simulatedConditionString = StringTS("addcondition name ttl"+String(channel+1)+" trialtypes "+String(TTL_TRIAL_OFFSET+channel)+" visible 1");
		  else
		    simulatedConditionString = StringTS("addcondition name ttl"+String(channel+1)+" trialtypes "+String(TTL_TRIAL_OFFSET+channel)+" visible 0");

		   std::vector<String> input = simulatedConditionString.splitString(' ');
 		  Condition newcondition(input,numExistingConditions+1+channel);
		  lockConditions();
		  newcondition.conditionID = ++conditionCounter;
		  conditions.push_back(newcondition);

		  PeriStimulusTimeHistogramEditor* edt = (PeriStimulusTimeHistogramEditor*) processor->getEditor();
//		  edt->updateCondition(conditions);

		  unlockConditions();
		  // now add a new psth for this condition for all sorted units on all electrodes
		  lockPSTH();
		  for (int i=0;i<electrodesPSTH.size();i++) 
		  {
			  for (int ch=0;ch<electrodesPSTH[i].channelsPSTHs.size();ch++)
			  {
				  electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs.push_back(ConditionPSTH(newcondition.conditionID,maxTrialTimeSeconds,preSec,postSec,visibility[channel]));
			  }

			  for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
			  {
				  electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs.push_back(ConditionPSTH(newcondition.conditionID,maxTrialTimeSeconds,preSec,postSec,visibility[channel]));
			  }
		  }
		  unlockPSTH();
		  
	  }

}

void TrialCircularBuffer::clearAll()
{

	lockPSTH();
	for (int i=0;i<electrodesPSTH.size();i++) 
	{
		for (int ch=0;ch<electrodesPSTH[i].channelsPSTHs.size();ch++)
		{
			electrodesPSTH[i].channelsPSTHs[ch].redrawNeeded = true;
			for (int psth=0;psth<electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs.size();psth++)
			{
				electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs[psth].clear();
			}
		}

		for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
		{
			electrodesPSTH[i].unitsPSTHs[u].redrawNeeded = true;
			for (int psth=0;psth<electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs.size();psth++)
			{
				electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs[psth].clear();
			}
		}
	}
	unlockPSTH();

}

void TrialCircularBuffer::clearDesign()
{
	lockConditions();
	// keep ttl visibility status
	Array<bool> ttlVisible;
	if (conditions.size() > 0)
	{
		for (int k=0;k<numTTLchannels;k++)
			ttlVisible.add(conditions[k].visible);
	} else
	{
		for (int k=0;k<numTTLchannels;k++)
			ttlVisible.add(false);
	}

	conditions.clear();
	conditionCounter = 0;
	PeriStimulusTimeHistogramEditor* edt = (PeriStimulusTimeHistogramEditor*) processor->getEditor();
//	edt->updateCondition(conditions);
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
		addDefaultTTLConditions(ttlVisible);
}



void TrialCircularBuffer::toggleConditionVisibility(int cond)
{
	// now add a new psth for this condition for all sorted units on all electrodes
	lockPSTH();
	conditions[cond].visible = !conditions[cond].visible;
	for (int i=0;i<electrodesPSTH.size();i++) 
	{
		for (int ch=0;ch<electrodesPSTH[i].channelsPSTHs.size();ch++)
		{
			electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs[cond].visible = !electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs[cond].visible;
			electrodesPSTH[i].channelsPSTHs[ch].redrawNeeded = true;
		}

		for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
		{
			electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs[cond].visible = !electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs[cond].visible;
			electrodesPSTH[i].unitsPSTHs[u].redrawNeeded = true;
		}
	}
	unlockPSTH();
	// inform editor repaint is needed

}

void TrialCircularBuffer::channelChange(int electrodeID, int channelindex, int newchannel)
{
	lockPSTH();
	lockConditions();
	for (int i=0;i<electrodesPSTH.size();i++) 
	{
		if (electrodesPSTH[i].electrodeID == electrodeID)
		{
			electrodesPSTH[i].channels[channelindex] = newchannel;
			electrodesPSTH[i].channelsPSTHs[channelindex].clearStatistics();
			for (int k=0;k<electrodesPSTH[i].unitsPSTHs.size();k++)
			{
				electrodesPSTH[i].unitsPSTHs[k].clearStatistics();
			}
		}
	}
	
	unlockConditions();
	unlockPSTH();
}

void TrialCircularBuffer::syncInternalDataStructuresWithSpikeSorter(Array<Electrode *> electrodes)
{
	lockPSTH();
	lockConditions();
	// note. This will erase all existing internal structures. Only call this in the constructor.
	electrodesPSTH.clear();
	for (int electrodeIter=0;electrodeIter<electrodes.size();electrodeIter++)
	{

		ElectrodePSTH electrodePSTH(electrodes[electrodeIter]->electrodeID);
		int numChannels = electrodes[electrodeIter]->numChannels;
		  
		for (int k=0;k<numChannels;k++) {
			int channelID = electrodes[electrodeIter]->channels[k];
			electrodePSTH.channels.push_back(channelID);
			ChannelPSTHs channelPSTH(channelID,maxTrialTimeSeconds, maxTrialsInMemory,preSec,postSec,binResolutionMS);
			// add all known conditions
			for (int c=0;c<conditions.size();c++)
			{
				channelPSTH.conditionPSTHs.push_back(ConditionPSTH(conditions[c].conditionID,maxTrialTimeSeconds,preSec,postSec,conditions[c].visible));
			}
			electrodePSTH.channelsPSTHs.push_back(channelPSTH);
		  }

		  // add all known units
		  std::vector<BoxUnit> boxUnits = electrodes[electrodeIter]->spikeSort->getBoxUnits();
		  std::vector<PCAUnit> pcaUnits = electrodes[electrodeIter]->spikeSort->getPCAUnits();
		  for (int boxIter=0;boxIter<boxUnits.size();boxIter++)
		  {

			  int unitID = boxUnits[boxIter].UnitID;
			  UnitPSTHs unitPSTHs(unitID, maxTrialTimeSeconds, maxTrialsInMemory,samplingRateHz,boxUnits[boxIter].ColorRGB[0],
				  boxUnits[boxIter].ColorRGB[1],boxUnits[boxIter].ColorRGB[2]);
			  for (int k=0;k<conditions.size();k++)
			  {
				  unitPSTHs.conditionPSTHs.push_back(ConditionPSTH(conditions[k].conditionID,maxTrialTimeSeconds,preSec,postSec,conditions[k].visible));
			  }
			  electrodePSTH.unitsPSTHs.push_back(unitPSTHs);
		  }

		  for (int pcaIter=0;pcaIter<pcaUnits.size();pcaIter++)
		  {

			  int unitID = pcaUnits[pcaIter].UnitID;
			  UnitPSTHs unitPSTHs(unitID, maxTrialTimeSeconds, maxTrialsInMemory,samplingRateHz,pcaUnits[pcaIter].ColorRGB[0],
				  pcaUnits[pcaIter].ColorRGB[1],pcaUnits[pcaIter].ColorRGB[2]);
			  for (int k=0;k<conditions.size();k++)
			  {
				  unitPSTHs.conditionPSTHs.push_back(ConditionPSTH(conditions[k].conditionID,maxTrialTimeSeconds,preSec,postSec,conditions[k].visible));
			  }
			  electrodePSTH.unitsPSTHs.push_back(unitPSTHs);
		  }
		  electrodesPSTH.push_back(electrodePSTH);
	}
	unlockConditions();
	unlockPSTH();
}

void TrialCircularBuffer::addNewElectrode(Electrode *electrode)
{
	lockPSTH();
	ElectrodePSTH e(electrode->electrodeID);
	int numChannels = electrode->numChannels;

	for (int k=0;k<numChannels;k++) 
	{
		int channelID = electrode->channels[k];
		e.channels.push_back(channelID);
		ChannelPSTHs channelPSTH(channelID,maxTrialTimeSeconds, maxTrialsInMemory,preSec,postSec,binResolutionMS);
		// add all known conditions
		for (int c=0;c<conditions.size();c++)
		{
			channelPSTH.conditionPSTHs.push_back(ConditionPSTH(conditions[c].conditionID,maxTrialTimeSeconds,preSec,postSec,conditions[c].visible));
		}
		e.channelsPSTHs.push_back(channelPSTH);	
	}
	electrodesPSTH.push_back(e);


	// Usually when we add a new electrode it doesn't have any units, unless it was added when loading an xml...
		if (electrode->spikeSort != nullptr)
		{
			std::vector<BoxUnit> boxUnits = electrode->spikeSort->getBoxUnits();
			for (int boxIter=0;boxIter < boxUnits.size();boxIter++)
			{
				addNewUnit(electrode->electrodeID, boxUnits[boxIter].UnitID, boxUnits[boxIter].ColorRGB[0],boxUnits[boxIter].ColorRGB[1],boxUnits[boxIter].ColorRGB[2]);
			}
			std::vector<PCAUnit> PcaUnits = electrode->spikeSort->getPCAUnits();
			for (int pcaIter=0;pcaIter < PcaUnits.size();pcaIter++)
			{
				addNewUnit(electrode->electrodeID, PcaUnits[pcaIter].UnitID, PcaUnits[pcaIter].ColorRGB[0],PcaUnits[pcaIter].ColorRGB[1],PcaUnits[pcaIter].ColorRGB[2]);
			}
		}
	unlockPSTH();
	((PeriStimulusTimeHistogramEditor *) processor->getEditor())->updateCanvas();
}

void  TrialCircularBuffer::addNewUnit(int electrodeID, int unitID, uint8 r,uint8 g,uint8 b)
{
	// build a new PSTH for all defined conditions
	UnitPSTHs unitPSTHs(unitID, maxTrialTimeSeconds, maxTrialsInMemory,samplingRateHz,r,g,b);
	for (int k=0;k<conditions.size();k++)
	{
		unitPSTHs.conditionPSTHs.push_back(ConditionPSTH(conditions[k].conditionID,maxTrialTimeSeconds,preSec,postSec,conditions[k].visible));
	}
	for (int k=0;k<electrodesPSTH.size();k++) {
		if (electrodesPSTH[k].electrodeID == electrodeID) {
			electrodesPSTH[k].unitsPSTHs.push_back(unitPSTHs);
			break;
		}
	}
	((PeriStimulusTimeHistogramEditor *) processor->getEditor())->updateCanvas();
	// inform editor repaint is needed
}

void  TrialCircularBuffer::removeUnit(int electrodeID, int unitID)
{
  lockPSTH();
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
}


void TrialCircularBuffer::removeElectrode(int electrodeID)
{
	lockPSTH();
	for (int e =0;e<electrodesPSTH.size();e++)
	{
		if (electrodesPSTH[e].electrodeID == electrodeID)
		{
			electrodesPSTH.erase(electrodesPSTH.begin() + e);
			break;
		}
	}
	unlockPSTH();
	// inform editor repaint is in order
	((PeriStimulusTimeHistogramEditor *) processor->getEditor())->updateCanvas();
}



void TrialCircularBuffer::parseMessage(StringTS msg)
  {
	  std::vector<String> input = msg.splitString(' ');
	  String command = input[0].toLowerCase();
	  
 if (command == "trialstart")
	  {
		  currentTrial.trialID = ++trialCounter;
		  currentTrial.startTS = msg.timestamp;
		  currentTrial.alignTS = 0;
		  currentTrial.alignTS_hardware = 0;
		  currentTrial.hardwareAlignment = false;
  		  currentTrial.trialInProgress = true;
		  lfpBuffer->addTrialStartToSmartBuffer(currentTrial.trialID);
		  for (int i=0;i<electrodesPSTH.size();i++) 
			{
				for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
				{
					electrodesPSTH[i].unitsPSTHs[u].addTrialStartToSmartBuffer(&currentTrial);
				}
		  }
		  if (input.size() > 1) {
			  currentTrial.type = input[1].getIntValue();
		  }
	  } else if (command == "trialend") 
	  {
		  currentTrial.endTS = msg.timestamp;
		  currentTrial.trialInProgress = false;

		
		  if (input.size() > 1) {
			  currentTrial.outcome = input[1].getIntValue();
		  }

		  if (currentTrial.type >= 0 && currentTrial.startTS > 0 &&  currentTrial.endTS - currentTrial.startTS < MaxTrialTimeTicks)
		  {
			  if (currentTrial.alignTS == 0) 
			  {
				  currentTrial.alignTS = currentTrial.startTS;
			  }

			  double trialLength = (currentTrial.endTS-currentTrial.alignTS)/numTicksPerSecond;
			  //processor->getUIComponent()->getLogWindow()->addLineToLog("Trial Length: "+String(trialLength*1000));
	
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
		  //clearDesign();
		  designName = input[1];
	  }
	  else if (command == "cleardesign")
	  {
			clearDesign();
			// inform editor repaint is needed
	  } else if (command == "addcondition")
	  {
		  int numExistingConditions = conditions.size();
		  Condition newcondition(input,numExistingConditions+1);
		  lockConditions();
		  newcondition.conditionID = ++conditionCounter;
		  conditions.push_back(newcondition);

		  PeriStimulusTimeHistogramEditor* edt = (PeriStimulusTimeHistogramEditor*) processor->getEditor();

		  unlockConditions();
		  // now add a new psth for this condition for all sorted units on all electrodes
		  lockPSTH();
		  for (int i=0;i<electrodesPSTH.size();i++) 
		  {
			  for (int ch=0;ch<electrodesPSTH[i].channelsPSTHs.size();ch++)
			  {
				  electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs.push_back(ConditionPSTH(newcondition.conditionID,maxTrialTimeSeconds,preSec,postSec,newcondition.visible));
			  }

			  for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
			  {
				  electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs.push_back(ConditionPSTH(newcondition.conditionID,maxTrialTimeSeconds,preSec,postSec,newcondition.visible));
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
					electrodesPSTH[e].unitsPSTHs[u].addSpikeToBuffer(newSpike.timestamp_software,newSpike.timestamp);
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

void TrialCircularBuffer::simulateTTLtrial(int channel, int64 ttl_timestamp_software)
{
	Trial ttlTrial;
	ttlTrial.trialID = ++trialCounter;
	ttlTrial.startTS = ttl_timestamp_software;
	ttlTrial.alignTS = ttl_timestamp_software;
	ttlTrial.endTS = ttl_timestamp_software + ttlTrialLengthSec*numTicksPerSecond;
	ttlTrial.outcome = 0;
	ttlTrial.trialInProgress = false;
	ttlTrial.type = TTL_TRIAL_OFFSET + channel;

	lfpBuffer->addTrialStartToSmartBuffer(ttlTrial.trialID);
	for (int i=0;i<electrodesPSTH.size();i++) 
	{
		for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
		{
			electrodesPSTH[i].unitsPSTHs[u].addTrialStartToSmartBuffer(&ttlTrial);
		}
	}
	aliveTrials.push(ttlTrial);
}

void TrialCircularBuffer::addTTLevent(int channel,int64 ttl_timestamp_software, int64 ttl_timestamp_hardware)
{
	// measure how much time passed since last ttl on this channel.
	// and only if its above some threshold, simulate a ttl trial.
	// this is useful when sending train of pulses and you are interested in aligning things just
	// to the first pulse.
	if (channel >= 0 && channel < lastTTLts.size())
	{
		int64 tickdiff = ttl_timestamp_software-lastTTLts[channel];
		float secElapsed = float(tickdiff) / numTicksPerSecond;
		if (secElapsed > ttlSupressionTimeSec)
		{
			// simulate a ttl trial 
			simulateTTLtrial(channel, ttl_timestamp_software);
		}
		lastTTLts[channel] = tickdiff;

		if (channel == hardwareTriggerAlignmentChannel)
		{
			currentTrial.alignTS = ttl_timestamp_software;
			currentTrial.alignTS_hardware = ttl_timestamp_hardware;
			currentTrial.hardwareAlignment = true;
		}
	}
	  
}
	

void TrialCircularBuffer::process(AudioSampleBuffer& buffer,int nSamples,int64 hardware_timestamp,int64 software_timestamp)
{
	if (firstTime) {

		Array<bool> ttlVisible;
		for (int k=0;k<numTTLchannels;k++)
			ttlVisible.add(false);

		addDefaultTTLConditions(ttlVisible);
		firstTime = false;
	}
	// first, update LFP circular buffers
	lfpBuffer->update(buffer, hardware_timestamp,software_timestamp, nSamples);
	// now, check if a trial finished, and enough time has elapsed so we also
	// have post trial information
	if (electrodesPSTH.size() > 0 && aliveTrials.size() > 0)
	{
		Trial topTrial = aliveTrials.front();
		int64 ticksElapsed = software_timestamp - topTrial.endTS;
		float timeElapsedSec = float(ticksElapsed)/ numTicksPerSecond;
		if (timeElapsedSec > postSec)
		{
			aliveTrials.pop();
			updatePSTHwithTrial(&topTrial);
		}
	}

}
