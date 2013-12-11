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

#ifndef __ISCAN_H
#define __ISCAN_H

#include "../../JuceLibraryCode/JuceHeader.h"
#include "GenericProcessor.h"
#include "NetworkEvents.h"
#include "Serial/ofSerial.h"
#include <list>
#include <queue>

class EyePosition
{
public:
	float x,y,pupil;
	int64 timestamp;
};
class ISCANnode : public GenericProcessor,  public Thread
{
public:
    ISCANnode();
    ~ISCANnode();
	AudioProcessorEditor* createEditor();
    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages, int& nSamples);
    void setParameter(int parameterIndex, float newValue);

	bool isSource();
	bool disable();
	void run();
	bool connect(int deviceID);
	StringArray getDeviceNames();

	bool isReady();
	float getDefaultSampleRate();
	int getDefaultNumOutputs();
	float getDefaultBitVolts();
	void enabledState(bool t);

	void postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events);
	void saveCustomParametersToXml(XmlElement* parentElement);
	void loadCustomParametersFromXml();


	 String device;

private:
 	ofSerial serialPort;
	std::vector<ofSerialDeviceInfo> devices;
	bool connected;
	std::string serialBuffer;
	Time timer;
	EyePosition prevEyePosition;
	void handleEvent(int eventType, MidiMessage& event, int samplePos);
	int eyeSamplingRateHz;
	int64 software_ts;
	bool firstTime;
	int packetCounter;
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ISCANnode);

};

#endif  // __NETWORKEVENT_H_91811541__
