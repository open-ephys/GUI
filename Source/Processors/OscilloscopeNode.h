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

#ifndef OSCILLOSCOPE_NODE_H_
#define OSCILLOSCOPE_NODE_H_


#include "../../JuceLibraryCode/JuceHeader.h"
#include "PeriStimulusTimeHistogramNode.h"
#include "GenericProcessor.h"

#include "Editors/PeriStimulusTimeHistogramEditor.h"
#include "Visualization/SpikeObject.h"
#include "SpikeDetector.h"

#include "Editors/VisualizerEditor.h"

#include <queue>
#include <vector>
#include "TrialCircularBuffer.h"

class DataViewport;
class SpikePlot;
class TrialCircularBuffer;

enum TriggerTypes {AUTO = 0, RISING = 1, FALLING = 2};

class OscilloscopeNode :  public GenericProcessor
{
public:

    OscilloscopeNode();
    ~OscilloscopeNode();

    AudioProcessorEditor* createEditor();
	
	void toggleConditionVisibility(int cond);

    bool isSink()
    {
        return true;
    }

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages, int& nSamples);
    void handleEvent(int, MidiMessage&, int);
	void setTriggerChannel(int channel, bool TTL);
	void setTriggerThreshold(double thres);
	double getTriggerThreshold();
    void updateSettings();
	void setFrequencyAnalyzerMode(bool state);
	bool getFrequencyAnalyzerMode();

    bool enable();
    bool disable();


	int getTriggerChannel();
	bool isTTLtrigger();
	TriggerTypes getTriggerType();
	void setTriggerType(TriggerTypes t);

	void saveCustomParametersToXml(XmlElement* parentElement);
	void loadCustomParametersFromXml();

	TrialCircularBuffer *trialCircularBuffer;
	
private:
	
	void four1(double data[], int nn, int isign);

	int64 totalSamplesProcessed;
	void allocateTrialCircularBuffer();
	TriggerTypes triggerType;
	TrialCircularBufferParams params;
	float prevValue;
	bool triggerByTTL;
	int numTTLchannels;
	int triggerChannel;
	double triggerThresholduV;
	bool fft;
	int64 hardware_timestamp,software_timestamp, numTicksPerSec;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeNode);

};


#endif // OSCILLOSCOPE_NODE_H_
