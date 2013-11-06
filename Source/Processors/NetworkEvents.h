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

#ifndef __NETWORKEVENT_H_91811541__
#define __NETWORKEVENT_H_91811541__

#include "zmq.h"
#include "zmq_utils.h"

#include "../../JuceLibraryCode/JuceHeader.h"
#include "GenericProcessor.h"


#include <queue>

/**

  Searches for threshold crossings and sends out TTL events.

  @see GenericProcessor

*/

class StringTS
{
public:
	StringTS(unsigned char *buf, int _len, juce::uint64 ts) : len(_len),timestamp(ts)  {
		str = new juce::uint8[len];
		for (int k=0;k<len;k++)
			str[k] = buf[k];
	}
	uint8* packWithTS() {
		uint8* tmp = new uint8[len+8];
		memcpy(tmp,str,len);
		memcpy(tmp+len,&timestamp,8);
		return tmp;
	}

	~StringTS() {
			delete str;
	}

	juce::uint8 *str;
	int len;
	juce::uint64 timestamp;
};

class NetworkEvents : public GenericProcessor,  public Thread
{
public:
    NetworkEvents(void *zmq_context);
    ~NetworkEvents();
	AudioProcessorEditor* createEditor();

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages, int& nSamples);
    void setParameter(int parameterIndex, float newValue);
	void run();
	void opensocket();
 
private:
	   void handleEvent(int eventType, MidiMessage& event, int samplePos);

	void *zmqcontext;
    float threshold;
    float bufferZone;
    bool state;
	Time timer;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetworkEvents);
	std::queue<StringTS> networkMessagesQueue;
	bool threadRunning ;
};

#endif  // __NETWORKEVENT_H_91811541__
