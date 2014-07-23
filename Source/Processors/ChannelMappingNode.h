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

#ifndef __CHANNELMAPPINGNODE_H_330E50E0__
#define __CHANNELMAPPINGNODE_H_330E50E0__


#include "../../JuceLibraryCode/JuceHeader.h"


#include "GenericProcessor.h"


/**

  Channel mapping node.

  Allows the user to select a subset of channels, remap their order, and reference them against
  any other channel.

  @see GenericProcessor

*/

class ChannelMappingNode : public GenericProcessor

{
public:

    ChannelMappingNode();
    ~ChannelMappingNode();

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages, int& nSamples);
    void setParameter(int parameterIndex, float newValue);

    AudioProcessorEditor* createEditor();

    bool hasEditor() const
    {
        return true;
    }

    void updateSettings();

private:

    Array<int> referenceArray;
	Array<int> referenceChannels;
    Array<int> channelArray;
	Array<bool> enabledChannelArray;

	int previousChannelCount;
	bool editorIsConfigured;

    AudioSampleBuffer channelBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelMappingNode);

};


#endif  // __CHANNELMAPPINGNODE_H_330E50E0__
