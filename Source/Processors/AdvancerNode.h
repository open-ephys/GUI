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

#ifndef __ADVANCERNODE_H
#define __ADVANCERNODE_H

#include "../../JuceLibraryCode/JuceHeader.h"
#include "GenericProcessor.h"

#include <list>

class AdvancerNode : public GenericProcessor
{
public:
    AdvancerNode();
    ~AdvancerNode();
	AudioProcessorEditor* createEditor();
    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages, int& nSamples);
	std::vector<String> splitString(String S, char sep);
	bool disable();
private:
	   void handleEvent(int eventType, MidiMessage& event, int samplePos);

	Time timer;
	std::list<float> advancerDepth;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdvancerNode);
};

#endif  // __NETWORKEVENT_H_91811541__
