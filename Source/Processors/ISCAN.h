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
	double x,y,pupil; // raw values
	double xc, yc; // calibrated to pixel coordinates
	int64 software_timestamp,hardware_timestamp;
};
class ISCANnode : public GenericProcessor
{
public:
    ISCANnode();
    ~ISCANnode();
	AudioProcessorEditor* createEditor();
    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages, int& nSamples);
    void setParameter(int parameterIndex, float newValue);

	bool isSource();
	bool disable();
	bool connect(int deviceID);
	StringArray getDeviceNames();
	StringArray getAnalogDeviceNames(Array<int> &channelNumbers);

	bool isReady();
	float getDefaultSampleRate();
	int getDefaultNumOutputs();
	float getDefaultBitVolts();
	void enabledState(bool t);

	void postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events);
	void saveCustomParametersToXml(XmlElement* parentElement);
	void loadCustomParametersFromXml();
	void setSerialCommunication(bool state);
	void process_serialCommunication(MidiBuffer& events);
	void setSamplingRate(int sampleRate);
	void setXchannel(int ch);
	void setYchannel(int ch);
	double applyCalibration(double input, int channel);

	void updateSettings();

	 String device;

private:
	int sampleCounter;
	void postEyePositionToMidiBuffer(EyePosition p, MidiBuffer& events);
	bool serialCommunication;
	int analogXchannel, analogYchannel,analogPupilchannel;
 	ofSerial serialPort;
	std::vector<ofSerialDeviceInfo> devices;
	bool connected;
	std::string serialBuffer;
	Time timer;
	EyePosition prevEyePosition;
	void handleEvent(int eventType, MidiMessage& event, int samplePos);
	int eyeSamplingRateHz;
	int64 software_ts;
	int64 numTicksPerSec;
	bool firstTime;
	int packetCounter;
	float offsetX, offsetY, gainX, gainY;
	int64 hardware_timestamp,software_timestamp;
	

   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ISCANnode);

};

#endif  // __NETWORKEVENT_H_91811541__
