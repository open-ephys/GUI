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

#include "../../Resources/ZeroMQ/include/zmq.h"
#include "../../Resources/ZeroMQ/include/zmq_utils.h"

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
	StringTS()
	{
		
		str = nullptr;
		len= 0;
		timestamp = 0;
	}

	String getString()
	{
		
		return String((const char*)str,len);
	}
	StringTS(String S)
	{
		Time t;
		str = new uint8[S.length()];
		memcpy(str,S.toRawUTF8(),S.length());
		timestamp = t.getHighResolutionTicks();

		len = S.length();
	}

	StringTS(String S, int64 ts_software)
	{
		str = new uint8[S.length()];
		memcpy(str,S.toRawUTF8(),S.length());
		timestamp = ts_software;

		len = S.length();
	}

	StringTS(const StringTS &s)
	{
		str = new uint8[s.len];
		memcpy(str,s.str,s.len);
		timestamp = s.timestamp;
		len = s.len;
	}


	StringTS(unsigned char *buf, int _len, int64 ts_software) : len(_len),timestamp(ts_software) {
		str = new juce::uint8[len];
		for (int k=0;k<len;k++)
			str[k] = buf[k];
	}

	~StringTS() {
			delete str;
	}

	juce::uint8 *str;
	int len;
	juce::int64 timestamp;
};

class NetworkEvents : public GenericProcessor,  public Thread
{
public:
    NetworkEvents(void *zmq_context);
    ~NetworkEvents();
	AudioProcessorEditor* createEditor();
	int64 getExtrapolatedHardwareTimestamp(int64 softwareTS);
	void initSimulation();
	void simulateDesignAndTrials(juce::MidiBuffer& events);
    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages, int& nSamples);
    void setParameter(int parameterIndex, float newValue);
	void handleSpecialMessages(StringTS msg);
	std::vector<String> splitString(String S, char sep);

	void simulateSingleTrial();
	
	void simulateStartRecord();
	void simulateStopRecord();
	bool disable();
	void run();
	void opensocket();


	bool isReady();
	float getDefaultSampleRate();
	int getDefaultNumOutputs();
	float getDefaultBitVolts();
	void enabledState(bool t);



	void postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events);
	void setNewListeningPort(int port);
	int urlport;
	 String socketStatus;
private:
	   void handleEvent(int eventType, MidiMessage& event, int samplePos);

	   StringTS createStringTS(String S, int64 t);
	  
	void *zmqcontext;
	void *responder;
    float threshold;
    float bufferZone;
    bool state;
	Time timer;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetworkEvents);
	std::queue<StringTS> networkMessagesQueue;
	bool threadRunning ;

	std::queue<StringTS> simulation;
	int64 simulationStartTime;
	bool firstTime ;
};

#endif  // __NETWORKEVENT_H_91811541__
