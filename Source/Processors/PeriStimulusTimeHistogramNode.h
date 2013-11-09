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

#ifndef PSTHNODE_H_
#define PSTHNODE_H_

#include "../../JuceLibraryCode/JuceHeader.h"
#include "Editors/SpikeDisplayEditor.h"
#include "Editors/VisualizerEditor.h"
#include "NetworkEvents.h"
#include "GenericProcessor.h"
#include "Visualization/SpikeObject.h"
#include "TrialCircularBuffer.h"
#include <queue>
#include <vector>

class DataViewport;
class SpikePlot;
class TrialCircularBuffer;

class PeriStimulusTimeHistogramNode :  public GenericProcessor
{
public:

    PeriStimulusTimeHistogramNode();
    ~PeriStimulusTimeHistogramNode();

    AudioProcessorEditor* createEditor();
	
	

	std::vector<String> splitString(String S, char sep);

    bool isSink()
    {
        return true;
    }

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages, int& nSamples);

    

    void handleEvent(int, MidiMessage&, int);

    void updateSettings();

    bool enable();
    bool disable();
    
    void startRecording();
    void stopRecording();

    String getNameForElectrode(int i);
    int getNumberOfChannelsForElectrode(int i);
    int getNumElectrodes();

    void addSpikePlotForElectrode(SpikePlot* sp, int i);
    void removeSpikePlots();

	TrialCircularBuffer *trialCircularBuffer;
  
private:
	StringTS unpackStringTS(MidiMessage &event);

    int displayBufferSize;
    bool redrawRequested;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeriStimulusTimeHistogramNode);

};


#endif  PSTHNODE_H_
