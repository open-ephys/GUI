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

/***************************/
class ContinuousCircularBuffer;

template <class T> class thread_safe_queue {
  public:
	 int size();
	 T front();
	 T pop();
	 void push(const T& t);
	 void lock();
	 void release();

	  CriticalSection c;
	  std::queue<T> q;
};

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
/***************/

class Event
{
public:
	Event(double Software_TS, double Hardware_TS, String EventDescription, bool bParse = true) ;
	Event(String EventDescription);

	bool parseEvent;
	double soft_ts;
	double hard_ts;
	String event_description;
};

Event::Event(double Software_TS, double Hardware_TS, String EventDescription, bool bParse) {
	soft_ts = Software_TS; hard_ts = Hardware_TS; event_description = EventDescription; parseEvent = bParse;
}

Event::Event(String EventDescription)
{
	parseEvent = true;

	juce::Time timer;
	soft_ts = double(timer.getHighResolutionTicks()) / double(timer.getHighResolutionTicksPerSecond()); 
	hard_ts = -1; 
	event_description = EventDescription;
}


/*******************************/ 
class SpikeCircularBuffer
{
public:
	SpikeCircularBuffer(int UnitID, int NumSamplesToHoldPerChannel);
	void addSpikeToBuffer(double ts);
	std::vector<double> GetDataArray(int N);
	void GetAlingedSpikesInTimeFrame(int saved_ptr, double BeforeSec, double AfterSec, double Start_TS, double Align_TS, double End_TS, std::vector<double> AlignedSpikes) ;
	int GetPtr();
	int unitID;
	int numSamplesInBuf;
	int ptr;
	int bufLen;
	std::vector<double> Buf;

};

SpikeCircularBuffer::SpikeCircularBuffer(int UnitID, int NumSamplesToHoldPerChannel)
{
            unitID = UnitID;
			Buf.resize(NumSamplesToHoldPerChannel);
            bufLen = NumSamplesToHoldPerChannel;
            numSamplesInBuf = 0;
            ptr = 0; // points to a valid position in the buffer.
}

std::vector<double> SpikeCircularBuffer::GetDataArray(int N)
{
	std::vector<double> LongArray;
	LongArray.resize(N);
//	mut.enter();

	int p = ptr - 1;
	for (int k = 0; k < N; k++)
	{
		if (p < 0)
			p = bufLen - 1;
		LongArray[k] = Buf[p];
		p--;
	}
	//mut.exit();
	return LongArray;
}

void SpikeCircularBuffer::addSpikeToBuffer(double ts)
{
	//mut.enter();
	Buf[ptr] = ts;
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
	//mut.exit();
}

int SpikeCircularBuffer::GetPtr()
{
	return ptr;
}

void SpikeCircularBuffer::GetAlingedSpikesInTimeFrame(int saved_ptr, double BeforeSec, double AfterSec, double Start_TS, double Align_TS, double End_TS, std::vector<double> AlignedSpikes) 
{
	// return all spikes within a given interval aligned to Align_TS
	// Use ptr as a search reference in the buffer
	// The interval is defined as Start_TS-BeforeSec .. End_TS+AfterSec
//	mut.enter();
	// count how many spikes happened after Start.

	// Search Backward
	int CurrPtr = saved_ptr;
	int  N = 0;
	while (N < numSamplesInBuf)
	{
		if (Buf[CurrPtr] < Start_TS-BeforeSec || Buf[CurrPtr] > End_TS+AfterSec) 
			break;
		// Add spike..
		AlignedSpikes.push_back(Buf[CurrPtr]-Align_TS);
		CurrPtr--;
		N++;
		if (CurrPtr < 0)
			CurrPtr = bufLen-1;
	}
	// Now Search Forward
	CurrPtr = saved_ptr + 1;

	while (N < numSamplesInBuf)
	{
		if (CurrPtr >= bufLen)
			CurrPtr = 0;

		if (Buf[CurrPtr] > End_TS + AfterSec || CurrPtr==ptr)
			break;
		// Add spike..
		if (Buf[CurrPtr] - Start_TS >= -BeforeSec)
		{
			AlignedSpikes.push_back(Buf[CurrPtr] - Align_TS);
			N++;
		}
		CurrPtr++;

	}
	std::sort(AlignedSpikes.begin(),AlignedSpikes.begin()+AlignedSpikes.size());
	//AlignedSpikes.Sort();
//	mut.exit();
}   

/*************************************************/
class UnitPtr
{
public:
	UnitPtr();
	UnitPtr(int Ptr, int UnitID);
	int ptr;
	int unitID;
};
UnitPtr::UnitPtr()
{
	ptr = -1;
	unitID = -1;
}

UnitPtr::UnitPtr(int Ptr, int UnitID)
{
	ptr = Ptr;
	unitID = UnitID;
}


/**************************************/



/************************************/


class UnitSpikes
{
public:
	UnitSpikes(int ID);
	int UnitID;
	std::vector<double> spikeTimesAligned;
};

UnitSpikes::UnitSpikes(int ID)
{
	UnitID = ID;
}


class ChannelSpikes
{
public:
	ChannelSpikes();
	std::vector<UnitSpikes> unitSpikes;
};

ChannelSpikes::ChannelSpikes() 
{
}

/**************************************/

class LFP_Trial_Data
{
public:
	LFP_Trial_Data();
	LFP_Trial_Data(int numCh, int numSamples);

	std::vector<std::vector<double>> data;
	std::vector<double> time;
};

LFP_Trial_Data::LFP_Trial_Data()
{

}

LFP_Trial_Data::LFP_Trial_Data(int numCh, int numSamples)
{
	data.resize(numCh);
	for (int k=0;k<numCh;k++)
		data[k].resize(numSamples);
	time.resize(numSamples);
}

class Trial {
public:
	Trial();
	Trial(const Trial &t);
	int GetBufferPtrAtTrialOnset(int ch, int UnitID);
	void SetBufferPtrAtTrialOnset(std::vector<std::list<SpikeCircularBuffer>> spikeBuffer, ContinuousCircularBuffer* lfp_buffer);

	int Outcome;
	int Type;
	double Start_TS, Align_TS, End_TS;
	bool trialInProgress; 
	LFP_Trial_Data lfp;
	std::vector<ChannelSpikes> channelSpikes;
	std::vector<std::vector<UnitPtr>> spikeBufferPtrs;
	int LFP_Buf_Ptr;
};

 Trial::Trial() 
 {
	 Outcome = -1;
	 Type = -1;
	Start_TS=-1;
	Align_TS=-1;
	End_TS=-1;
	trialInProgress = false;

 }


 Trial::Trial(const Trial &t)
 {
	 Outcome = t.Outcome;
	 Type = t.Type;
	 Start_TS = t.Start_TS;
	 Align_TS = t.Align_TS;
	 End_TS = t.End_TS;
	 channelSpikes = t.channelSpikes;
	 spikeBufferPtrs = t.spikeBufferPtrs;
	 trialInProgress = t.trialInProgress;
	 LFP_Buf_Ptr = t.LFP_Buf_Ptr;
	 lfp = t.lfp;
 }

 
        
 int Trial::GetBufferPtrAtTrialOnset(int ch, int UnitID) {
	 for (int k=0;k<spikeBufferPtrs[ch].size();k++)
	 {
		 if (spikeBufferPtrs[ch][k].unitID == UnitID)
			 return spikeBufferPtrs[ch][k].ptr;
	 }
	 jassertfalse;
	 return 0; // shouldn't occur...
 }


 void Trial::SetBufferPtrAtTrialOnset(std::vector<std::list<SpikeCircularBuffer>> spikeBuffer, ContinuousCircularBuffer* lfp_buffer)
{
	LFP_Buf_Ptr = lfp_buffer->GetPtr();
	int numCh = spikeBuffer.size();
	spikeBufferPtrs.resize(numCh);
	for (int ch = 0; ch < numCh; ch++)
	{
		int numUnitsForThisChannel = spikeBuffer[ch].size();
		spikeBufferPtrs[ch].resize(numUnitsForThisChannel);
		for (std::list<SpikeCircularBuffer>::iterator it = spikeBuffer[ch].begin(); it != spikeBuffer[ch].end(); it++)
		{
			spikeBufferPtrs[ch].push_back(UnitPtr((*it).GetPtr(), (*it).unitID));
		}
	}
}

/***************************************/


class UnitPSTH
{
public:
		UnitPSTH();
        UnitPSTH(int ID, double AfterSec, double BeforeSec);
		void Update(std::vector<double> spikeTimes, double LastTS);
		std::vector<double> GetSmoothPSTH(double fGaussianWidthMS);
		void Clear();

		bool NewData;
		int numBins;
		int binResolutionMS;
		double afterSec, beforeSec;
		int NumTrials;
		std::vector<int> numAvgPoints;
		std::vector<double> time;
		std::vector<double> avg_firing_rate_Hz;
		int UnitID;

};

UnitPSTH::UnitPSTH() {
	NumTrials=-1;
	UnitID = -1;
	binResolutionMS = -1;
	afterSec = 0;
	beforeSec = 0;
}

UnitPSTH::UnitPSTH(int ID, double AfterSec, double BeforeSec)
{
	NumTrials=0;
	UnitID = ID;
	binResolutionMS = 10;
	afterSec = AfterSec;
	beforeSec = BeforeSec;
	numBins = (afterSec + beforeSec) * 1000.0f / binResolutionMS; // 1 ms resolution
	int NumBinsBefore = beforeSec * 1000.0f / binResolutionMS; // 1 ms resolution
	avg_firing_rate_Hz.resize(numBins);
	numAvgPoints.resize(numBins);
	time.resize(numBins);
	NewData = true;
	for (int k = 0; k < numBins; k++)
	{
		numAvgPoints[k] = 0;
		time[k] = (double)k / numBins * (afterSec + beforeSec) - beforeSec;
		avg_firing_rate_Hz[k] = 0;
	}
}

void UnitPSTH::Update(std::vector<double> spikeTimes, double LastTS)
{
	NewData = true;
	std::vector<double> instantaneousSpikes;
	instantaneousSpikes.resize(numBins);
	for (int k = 0; k < numBins; k++)
	{
		instantaneousSpikes[k] = 0;
	}
	for (int k=0;k<spikeTimes.size();k++)
	{
		// spike times are aligned relative to trial alignment (i.e.) , onset is at "0"

		int Bin = (spikeTimes[k] + beforeSec) / (beforeSec + afterSec) * numBins;
		if (Bin >= 0 && Bin < numBins)
		{
			instantaneousSpikes[Bin] += 1.0;
		}
	}

	int LastBin = ((LastTS + beforeSec) / (beforeSec + afterSec) * numBins);

	NumTrials++;
	// Update average firing rate
	for (int k = 0; k < LastBin; k++)
	{
		numAvgPoints[k]++;

		avg_firing_rate_Hz[k] = ((numAvgPoints[k] - 1) * avg_firing_rate_Hz[k] + instantaneousSpikes[k]) / numAvgPoints[k];
	}

}

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
	/* NEED TO IMPLEMENT THIS...
	alglib.alglib.convr1dcircular(avg_firing_rate_Hz, avg_firing_rate_Hz.Length,
	gaussianKernel, gaussianKernel.Length, out smoothFiringRate);
	*/
	return smoothFiringRate;
}

void UnitPSTH::Clear()
{
	for (int k = 0; k < numBins; k++)
	{
		numAvgPoints[k] = 0;
		avg_firing_rate_Hz[k] = 0;
		NumTrials = 0;
	}
}

/*******************************/


class ChannelPSTH
{
public:
	ChannelPSTH(int NumCh, double AfterSec, double BeforeSec);
	std::vector<int> histc(std::vector<double> xi, std::vector<double> x);
	std::vector<double> diff(std::vector<double> x);
	void interp1(std::vector<double> x, std::vector<std::vector<double>>y, std::vector<double> xi, std::vector<std::vector<double>> &yi, std::vector<bool> &valid);
	void Update(const Trial &t);

	int numCh;
	int binResolutionMS, numBins;
	double afterSec, beforeSec;
	std::vector<double> time;
	std::vector<std::vector<int>> numAvgPoints;
	std::vector<std::vector<double>> avg_lfp;

};

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

  
/********************************/


class Condition
{
	public:
  	   Condition(int NumCh, String Name, std::vector<int> types, std::vector<int> outcomes, double AfterSec, double BeforeSec);
	   void UpdatePSTHUsingTrialData(const Trial &trial);

       int numCh;
       String name;
       float colorRGB[3]; 
       std::vector<int> trialTypes;
       std::vector<int> trialOutcomes;
       double afterSec, beforeSec;
       bool visible;
       std::vector<std::vector<UnitPSTH>> psth; // array per channel, PSTH per unit
       ChannelPSTH *lfp_psth;

};


Condition::Condition(int NumCh, String Name, std::vector<int> types, std::vector<int> outcomes, double AfterSec, double BeforeSec)
{
            name = Name;
            trialTypes = types;

            afterSec = AfterSec;
            beforeSec = BeforeSec;

            trialOutcomes = outcomes;
            
            numCh = NumCh;
            psth.resize(numCh);
            lfp_psth = new ChannelPSTH(numCh,afterSec, beforeSec);

            for (int ch = 0; ch < numCh; ch++)
            {
                psth[ch].resize(0);
            }
}


void Condition::UpdatePSTHUsingTrialData(const Trial &trial)
{
	// trial has all the relevant statistics aligned properly.
	// all we need to do is to take the spikes and bin them into a psth
	for (int ch=0;ch<numCh;ch++) {
		int numUnits = trial.channelSpikes[ch].unitSpikes.size();
		// Trial has spike information for some units... 
		for (int unit_iter = 0; unit_iter < numUnits; unit_iter++)
		{
			int indx_in_psth = -1;
			for (int indx = 0; indx < psth[ch].size(); indx++)
			{
				if (psth[ch][indx].UnitID == trial.channelSpikes[ch].unitSpikes[unit_iter].UnitID)
				{
					indx_in_psth = indx;
					break;
				}
			}
			if (indx_in_psth == -1)
			{
				// unit wasn't found in psth unit array. Add it!

				psth[ch].push_back(UnitPSTH(trial.channelSpikes[ch].unitSpikes[unit_iter].UnitID, afterSec, beforeSec));
				indx_in_psth = 0;
			}

			// Add trial spikes to psth
			psth[ch][indx_in_psth].Update(trial.channelSpikes[ch].unitSpikes[unit_iter].spikeTimesAligned, trial.End_TS-trial.Start_TS);
		}

	}


	lfp_psth->Update(trial);
}




/**************************/

class TrialCircularBuffer : public Thread
    {
	public:
	 TrialCircularBuffer(int NumCh, int NumTTLch, double SamplingRate);
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
	Trial currentTrial;
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

};


TrialCircularBuffer::TrialCircularBuffer(int NumCh, int NumTTLch, double SamplingRate) : Thread("TrialThread")
{
	numTrials = 0;
	numCh = NumCh;
	numTTLch = NumTTLch;
	TTL_Trial_Length_Sec = 1;
	TTL_Trial_Inhibition_Sec = 1;
	AfterSec = 0.5;
	BeforeSec = 0.5;

	AfterSec = 0.5;
	BeforeSec = 0.5;

	int SubSampling = 10; // 2.5 kHz is more than enough for LFP...

    spikeBuffer.resize(numCh);
	lfpBuffer = new ContinuousCircularBuffer(numCh, SamplingRate, SubSampling, 2*MAX_TRIAL_TIME_SEC);

    lastTTLtrialTS.resize(numTTLch);
    for (int k = 0; k < numTTLch; k++)
    {
		lastTTLtrialTS[k] = 0;
    }

    AddDefaultTTLConditions();

	startThread();
}

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

void TrialCircularBuffer::AddCondition(const Condition &c) 
{
	LockConditions();
	conditions.push_back(c);
    UnlockConditions();
}

void TrialCircularBuffer::LockConditions()
{
	conditionsMutex.enter();
}

void TrialCircularBuffer::UnlockConditions()
{
	conditionsMutex.exit();
}




bool TrialCircularBuffer::UnitAlive(int channel, int UnitID)
{
	for (std::list<SpikeCircularBuffer>::iterator it = spikeBuffer[channel].begin(); it != spikeBuffer[channel].end();it++)
            {
				
                if ((*it).unitID == UnitID)
                {
                    return true;
                }
            }
            return false;
        }



void TrialCircularBuffer::RemoveUnitStats(int channel, int UnitID)
{
	for (std::list<SpikeCircularBuffer>::iterator it = spikeBuffer[channel].begin(); it != spikeBuffer[channel].end();it++)
	{
		if ((*it).unitID == UnitID)
		{
			spikeBuffer[channel].erase(it);
			break;
		}
	}
}


void TrialCircularBuffer::AddSpikeToSpikeBuffer(int channel, int UnitID, double ts) 
{
	for (std::list<SpikeCircularBuffer>::iterator it = spikeBuffer[channel].begin(); it != spikeBuffer[channel].end();it++)
	{
		if ((*it).unitID == UnitID)
		{
			(*it).addSpikeToBuffer(ts);
		}
	}
}


// Given a circular buffer (queue) of wave forms, fill the trial with 
// spike data (per channel, per unit), aligned to trial alignment ts.
void TrialCircularBuffer::UpdateTrialWithSpikeData(Trial trial)
{
	trial.channelSpikes.resize(numCh);// = new ChannelSpikes[numCh];
	for (int ch = 0; ch < numCh; ch++)
	{
		// we now need to go over the entire circular buffer and search for spikes that occurred within the
		// time frame of the trial.
		// To speed things up, instead of searching the entire spike buffer, we hold reference pointers
		// to where the buffer point was at the trial onset, and search from that point backward & forward.
		//trial.channelSpikes[ch] = new ChannelSpikes();
		//trial.channelSpikes[ch].unitSpikes = new List<UnitSpikes>();
		//.unitSpikes = new List<UnitSpikes>();

		for (std::list<SpikeCircularBuffer>::iterator it = spikeBuffer[ch].begin(); it != spikeBuffer[ch].end();it++)
		{
			int ptr = trial.GetBufferPtrAtTrialOnset(ch, (*it).unitID);
			// ptr points to where the buffer was when the trial started...
			// Search for spikes up to "BeforeSec" before and up to
			// AfterSec + trial.End_TS-trial.Start_TS
			UnitSpikes unitSpikesAligned((*it).unitID);
			(*it).GetAlingedSpikesInTimeFrame(ptr, BeforeSec, AfterSec, trial.Start_TS, trial.Align_TS, trial.End_TS, unitSpikesAligned.spikeTimesAligned);
			trial.channelSpikes[ch].unitSpikes.push_back(unitSpikesAligned);
		}
	}
}



void TrialCircularBuffer::UpdateTrialWithLFPData(Trial t)
{
//	t.lfp = lfpBuffer->GetRelevantData(t.LFP_Buf_Ptr, t.Start_TS, t.Align_TS,t.End_TS, BeforeSec, AfterSec);
}


bool TrialCircularBuffer::Contains(std::vector<int> vec, int value) 
{
	for (int k=0;k<vec.size();k++)
	{
		if (vec[k] == value)
		{
			return true;
		}
	}
	return false;
}

void TrialCircularBuffer::UpdateConditionsWithTrial(Trial trial) 
{
	numTrials++;
	avgTrialLengthSec = ((numTrials - 1) * avgTrialLengthSec + (trial.End_TS - trial.Start_TS)) / numTrials;

	for (int k=0;k<conditions.size();k++)
	{
		if (Contains(conditions[k].trialTypes,trial.Type) && (conditions[k].trialOutcomes.size() == 0 || (conditions[k].trialOutcomes.size()> 0 && 
			Contains(conditions[k].trialOutcomes,trial.Outcome)))) {
				// Add the trial to the statistics of this conditions.
				// Update all channels and all units....
				conditions[k].UpdatePSTHUsingTrialData(trial);
		}
	}
}




void TrialCircularBuffer::run()
{
	Time timer;
	ProcessThreadRunning = true;
	while (ProcessThreadRunning)
	{
		// Check for incoming messages
		eventQueue.lock();
		{
			while (eventQueue.q.size() > 0)
			{
				Event E = eventQueue.q.front();
				eventQueue.q.pop();
				ParseMessage(E);
			}
		}
		eventQueue.release();

		if (AliveTrials.size() > 0)
		{
			Trial TopTrial = AliveTrials.front();
			double timeElapsed = double(timer.getHighResolutionTicks())/double(timer.getHighResolutionTicksPerSecond());
			if (timeElapsed > TopTrial.End_TS+AfterSec)
			{
				AliveTrials.pop();
				UpdateTrialWithSpikeData(TopTrial);
				UpdateTrialWithLFPData(TopTrial);
				UpdateConditionsWithTrial(TopTrial);
			}
		}
		// Be nice
		sleep(5); //		Thread.Sleep(5);
	}
}



void TrialCircularBuffer::AddEvent(Event E)
{
	eventQueue.push(E);
}


 Condition TrialCircularBuffer::fnParseCondition(std::vector<String> items) 
 {
	 int k=1;
	 String ConditionName = "Unknown";
	 bool Visible = true;
	 std::list<int> trialtypes;
	 std::list<int> outcomes;
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
			 ConditionName = items[k];
			 k++;
			 continue;
		 }
		 if (lower_item == "visible")
		 {
			 bTrialTypes = false;
			 bOutcomes = false;
			 k++;
			 Visible = items[k].getIntValue() > 0;
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
		 if (bOutcomes)
		 {
			 outcomes.push_back(items[k].getIntValue());
			 k++;
		 }

		 if (bTrialTypes) {
			 trialtypes.push_back(items[k].getIntValue());
			 k++;
		 }

	 }

	 std::vector<int> trialtypes_vector(trialtypes.begin(), trialtypes.end()); 
	 std::vector<int> trialoutcomes_vector(outcomes.begin(), outcomes.end()); 
	 return Condition(numCh, ConditionName, trialtypes_vector, trialoutcomes_vector, AfterSec + MAX_TRIAL_TIME_SEC, BeforeSec);   
 }

 
std::vector<String> TrialCircularBuffer::SplitString(String S, char sep)
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
		}
	}

	 std::vector<String> Svec(ls.begin(), ls.end()); 
	return Svec;

}


void TrialCircularBuffer::ParseMessage(Event E)
{
	String Message = E.event_description;

	std::vector<String> split = SplitString(Message,' ');
	String Command = split[0];
	String cmd = Command.toLowerCase();
	if (cmd == "cleardesign")
	{
		LockConditions();
		conditions.clear();
		UnlockConditions();
		AddDefaultTTLConditions();
	} else if (cmd == "newdesign")
	{
		DesignName = split[1];
	} else if (cmd == "addcondition")
	{
		Condition c = fnParseCondition(split);
		AddCondition(c);
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
	} else if (cmd ==  "trialtype")
	{
		currentTrial.Type = split[1].getIntValue();
	} else if (cmd == "trialalign")
	{
		currentTrial.Align_TS = E.soft_ts;
	}  else if (cmd == "trialoutcome")
	{
		currentTrial.Outcome = split[1].getIntValue();
	}  else if (cmd == "trialstart")
	{
		currentTrial.Start_TS = E.soft_ts;
		currentTrial.trialInProgress = true;
		currentTrial.SetBufferPtrAtTrialOnset(spikeBuffer,lfpBuffer);
	} else if (cmd == "trialend")
	{
		currentTrial.End_TS = E.soft_ts;
		currentTrial.trialInProgress = false;
		if (currentTrial.Type >= 0 && currentTrial.Outcome >= 0 &&
			currentTrial.Start_TS >= 0 && 
			currentTrial.End_TS - currentTrial.Start_TS < MAX_TRIAL_TIME_SEC)
		{
			// no alignment? use trial start.
			if (currentTrial.Align_TS < 0)
				currentTrial.Align_TS = currentTrial.Start_TS;

			AliveTrials.push(Trial(currentTrial));
		}
	}


}

void TrialCircularBuffer::ClearStats(int channel, int unitID) {
	LockConditions();
	for (int k=0;k<conditions.size();k++)
	{
		// zero out spikes
		for (int j=0;j<conditions[k].psth[channel].size();j++)
		{
			if (conditions[k].psth[channel][j].UnitID == unitID)
			{
				conditions[k].psth[channel][j].Clear();
			}
		}
		// zero out lfp
		for (int m=0;m<conditions[k].lfp_psth->time.size();m++) {
			conditions[k].lfp_psth->avg_lfp[channel][ m] = 0;
			conditions[k].lfp_psth->numAvgPoints[channel][m] = 0;
		}
	}
	UnlockConditions();
}
