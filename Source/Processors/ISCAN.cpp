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

#include <stdio.h>
#include "ISCAN.h"
#include "Editors/ISCANeditor.h"
#include "../UI/UIComponent.h"

#if JUCE_WINDOWS
#define SLEEPY_TIME Sleep(100);
#else
#define SLEEPY_TIME usleep(100000);
#endif

/*********************************************/
ISCANnode::ISCANnode()
	: GenericProcessor("Eye Tracking")

{
	sendSampleCount = false; // disable updating the continuous buffer sample counts,
								 // since this processor only sends events
	connected = false;
	eyeSamplingRateHz = 120;
	firstTime = true;
}

ISCANnode::~ISCANnode()
{
	if (connected) {
		int iscan_track_off_code = 129;
		serialPort.writeByte(iscan_track_off_code);
		SLEEPY_TIME
		serialPort.close();
	}
	
}

bool ISCANnode::disable()
{
	return true;
}

AudioProcessorEditor* ISCANnode::createEditor()
{
    editor = new ISCANeditor(this, true);

    return editor;


}

StringArray ISCANnode::getDeviceNames()
{
	StringArray names;
	devices = serialPort.getDeviceList();
	for (int k=0;k<devices.size();k++)
	{
		names.add(devices[k].getDeviceName());
	}
	return names;
}

bool ISCANnode::connect(int selectedDevice)
{
	connected = serialPort.setup(devices[selectedDevice-1].getDeviceID(), 115200);
	if (connected)
	{
		int maxComponents = 3;	//horizontal H1, vertical V1, pupil diameter D1
		int maxReadQuantum = maxComponents * 7 + 2; // last entry is followed by a tab!
		int lineTerminator = 10;

		int	iscan_track_on_code = 128;
		int iscan_track_off_code = 129;

		serialPort.writeByte(iscan_track_off_code);
		SLEEPY_TIME
		serialPort.flush();
		SLEEPY_TIME
		serialPort.writeByte(iscan_track_on_code);
		SLEEPY_TIME
		int avail = serialPort.available();
		if (avail > 0)
		{
			// connection is successful.
			unsigned char *buf = new unsigned char[avail];
			int res = serialPort.readBytes(buf, avail);
			delete buf;
		}
	}
	return connected;
}

void ISCANnode::setParameter(int parameterIndex, float newValue)
{
/*
	editor->updateParameterButtons(parameterIndex);

	Parameter& p =  parameters.getReference(parameterIndex);
	p.setValue(newValue, 0);

	threshold = newValue;
	*/
	//std::cout << float(p[0]) << std::endl;

}

void ISCANnode::handleEvent(int eventType, juce::MidiMessage& event, int samplePosition)
{

}

void ISCANnode::postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events)
{
	uint8* msg_with_ts = new uint8[s.len+8]; // for the two timestamps
	memcpy(msg_with_ts, s.str, s.len);	
	memcpy(msg_with_ts+s.len, &s.timestamp, 8);
	addEvent(events, 
			 (uint8) NETWORK,
			 0,
			 0,
			 (uint8) GENERIC_EVENT,
			 (uint8) s.len+8,
			 msg_with_ts);

	delete msg_with_ts;
}



void ISCANnode::postEyePositionToMidiBuffer(EyePosition p, MidiBuffer& events)
{
	uint8* eyePositionSerialized = new uint8[8+8+8+8]; 
	memcpy(eyePositionSerialized, &p.x, 8);	
	memcpy(eyePositionSerialized+8, &p.y, 8);	
	memcpy(eyePositionSerialized+8+8, &p.pupil, 8);	
	memcpy(eyePositionSerialized+8+8+8, &p.timestamp, 8);	
	addEvent(events, 
			 (uint8) EYE_POSITION,
			 0,
			 0,
			 (uint8) GENERIC_EVENT,
			 (uint8) 8+8+8+8+8, //x,y,p+ts
			 eyePositionSerialized);

	delete eyePositionSerialized;
}

void ISCANnode::process(AudioSampleBuffer& buffer,
							MidiBuffer& events,
							int& nSamples)
{

	checkForEvents(events);

	// read samples and pass them as midi events.

	int bytesAvail = serialPort.available();
	if (serialBuffer.length() == 0 || firstTime)
	{
		 software_ts = timer.getHighResolutionTicks();
		 firstTime = false;
		 packetCounter = 0;
	}

	if (bytesAvail > 0)
	{
		unsigned char *pktdata = new unsigned char[bytesAvail];
		serialPort.readBytes(pktdata,bytesAvail);
	 
		for (int k=0;k<bytesAvail;k++)
			serialBuffer += pktdata[k];

	delete pktdata;
		
		while (true)
		{
			// now search for a line termination.
			// if one is present, we can extract the packet.
			bool lineTerminationFound = false;
			int line_termination_pos=-1;
			for (int k=0;k<serialBuffer.length();k++)
			{
				if (serialBuffer[k] == '\n')
				{
					lineTerminationFound = true;
					line_termination_pos = k;
					break;
				}
			}
			if (lineTerminationFound)
			{
				EyePosition e;
				if (sscanf(serialBuffer.c_str(),"%lf %lf %lf", &e.x, &e.y, &e.pupil) == 3)
				{
					// post new eye position message.
					packetCounter++;
					if (prevEyePosition.x != e.x || prevEyePosition.y != e.y  || prevEyePosition.pupil != e.pupil)
					{
						// send midi message
						int64 timestamp = software_ts+ packetCounter * 1.0/eyeSamplingRateHz * timer.getHighResolutionTicksPerSecond();
						prevEyePosition.timestamp = timestamp;
						prevEyePosition.x = e.x;
						prevEyePosition.y = e.y;
						prevEyePosition.pupil = e.pupil;
						postEyePositionToMidiBuffer(prevEyePosition, events);

					}
				} 
				serialBuffer = serialBuffer.substr(	1+line_termination_pos);
				
			} else
			{
				break;
			}
		}
	}

	 nSamples = -10; // make sure this is not processed;
	
}


bool ISCANnode::isReady()
{
   
        return true;
    
}


float ISCANnode::getDefaultSampleRate()
{
    return 30000.0f;
}

int ISCANnode::getDefaultNumOutputs()
{
    return 0;
}

float ISCANnode::getDefaultBitVolts()
{
    return 0.05f;
}

void ISCANnode::enabledState(bool t)
{

    isEnabled = t;

}

bool ISCANnode::isSource()
{
	return true;
}

void ISCANnode::saveCustomParametersToXml(XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("ISCAN");
    mainNode->setAttribute("device", device);
}


void ISCANnode::loadCustomParametersFromXml()
{

	if (parametersAsXml != nullptr)
	{
		forEachXmlChildElement(*parametersAsXml, mainNode)
		{
			if (mainNode->hasTagName("ISCAN"))
			{
				device = mainNode->getStringAttribute("device");
			}
		}
	}
}

