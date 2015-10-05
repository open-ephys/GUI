/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef __OSCNODE_H_A75239F7__
#define __OSCNODE_H_A75239F7__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "ReceiveOSC.h"

#include <stdio.h>


/**

  Allows the user to split the signal chain.

  This processor doesn't modify the data passing through it. In fact,
  it has no incoming or outgoing connections. It just allows the outputs from
  its source node to be connected to TWO destination nodes.

  @see GenericProcessor, ProcessorGraph

*/


class OSCNode : public GenericProcessor
{
public:

    OSCNode();
    ~OSCNode();

    AudioProcessorEditor* createEditor();

    bool isSource()
    {
        DBG("Is source!");
        return true;
    }

    bool isReady();

    void switchIO(int);
    void switchIO();
    void setOSCNodeDestNode(GenericProcessor* dn);

    void setPathToProcessor(GenericProcessor* processor);

	void receivePosition(float x, float y);
//    void process(AudioSampleBuffer& buffer, MidiBuffer& events);

    int getPath();

	void process(AudioSampleBuffer&, MidiBuffer&);
    int getNumEventChannels();
	void updateSettings();

private:

    int64 timestamp;
	int64 previousEventTime;
	juce::uint8 eventId;

    GenericProcessor* destNodeA;
    GenericProcessor* destNodeB;
    int activePath;
    ReceiveOSC osc;
    CriticalSection lock;

	float m_x;
	float m_y;
	bool m_positionIsUpdated;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCNode);

};


#endif  // __OSCNODE_H_A75239F7__
