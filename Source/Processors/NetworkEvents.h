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

#include <list>
#include <queue>

/**

  Searches for threshold crossings and sends out TTL events.

  @see GenericProcessor

*/

class StringTS
{
public:
	StringTS();
	std::vector<String> splitString(char sep);
	StringTS(MidiMessage &event);
	String getString();
	StringTS(String S);
	StringTS(String S, int64 ts_software);
	StringTS(const StringTS &s);
	StringTS(unsigned char *buf, int _len, int64 ts_software);
	~StringTS();

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
	String handleSpecialMessages(StringTS msg);
	std::vector<String> splitString(String S, char sep);

	void simulateSingleTrial();
	bool isSource();

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

	void saveCustomParametersToXml(XmlElement* parentElement);
	void loadCustomParametersFromXml();

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
 	std::queue<StringTS> networkMessagesQueue;
	bool threadRunning ;

	std::queue<StringTS> simulation;
	int64 simulationStartTime;
	bool firstTime ;

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetworkEvents);

};

#endif  // __NETWORKEVENT_H_91811541__
