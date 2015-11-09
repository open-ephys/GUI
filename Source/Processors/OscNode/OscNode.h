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
#include "OscServer.h"

#include <stdio.h>

class OscNode : public GenericProcessor
{
public:

    OscNode();
    ~OscNode();

    AudioProcessorEditor* createEditor();

    bool isSource();
    bool isReady();

    void receiveMessage(std::vector<float> message);

    void process(AudioSampleBuffer&, MidiBuffer&);
    int getNumEventChannels();
    void updateSettings();
    void setAddress(String address);
    String address();
    void setPort(int port);
    int port();

private:

    int64 timestamp;
    int64 previousEventTime;
    juce::uint8 eventId;

    CriticalSection lock;

    std::vector<float> m_message;

    bool m_positionIsUpdated;
    String m_address;
    int m_port;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscNode);

};


#endif  // __OSCNODE_H_A75239F7__
