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
#include "RecordControl.h"
#include "../../UI/ControlPanel.h"

RecordControl::RecordControl()
    : GenericProcessor("Record Control"),
      triggerChannel(0)
{

}

RecordControl::~RecordControl()
{

}

AudioProcessorEditor* RecordControl::createEditor()
{
    editor = new RecordControlEditor(this, true);
    return editor;
}

void RecordControl::setParameter(int parameterIndex, float newValue)
{
    if (parameterIndex == 0)
    {
        updateTriggerChannel((int) newValue);
    }
    else if (parameterIndex == 1)
    {
        triggerType = (Types)((int)newValue - 1);
    }
    else if (parameterIndex == 2)
    {
        triggerEdge = (Edges)((int)newValue - 1);
    }
}

void RecordControl::updateTriggerChannel(int newChannel)
{
    triggerChannel = newChannel;
}

bool RecordControl::enable()
{

    return true;
}

void RecordControl::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events)
{
    checkForEvents(events);
}


void RecordControl::handleEvent(int eventType, MidiMessage& event, int)
{
    const uint8* dataptr = event.getRawData();

    int eventId = *(dataptr+2);
    int eventChannel = *(dataptr+3);

    //std::cout << "Received event with id=" << eventId << " and ch=" << eventChannel << std::endl;

    if (eventType == TTL && eventChannel == triggerChannel)
    {
        int edge = triggerEdge == RISING ? 1 : 0;

        //std::cout << "Trigger!" << std::endl;

        const MessageManagerLock mmLock;

        if (triggerType == SET)
        {
            if (eventId == edge)
            {
                CoreServices::setRecordingStatus(true);
            }
            else
            {
                CoreServices::setRecordingStatus(false);
            }
        }
        else if (triggerType == TOGGLE && eventId == edge)
        {
            CoreServices::setRecordingStatus(!CoreServices::getRecordingStatus());
        }


    }

}


String RecordControl::interProcessorCommunication(String msg)
{
	String status;
	StringArray inputs;

	inputs.addTokens(msg, " ");

	const MessageManagerLock mmLock;
	if (String("StartAcquisition").compareIgnoreCase(inputs[0]) == 0)
	{
		if (AccessClass::getControlPanel()->getAcquisitionState())
		{
			/* Start data acquisition */
			AccessClass::getControlPanel()->setAcquisitionState(true);
		}
		return String("OK");

	} else 	if (String("StopAcquisition").compareIgnoreCase(inputs[0]) == 0)
	{
		if (AccessClass::getControlPanel()->getAcquisitionState())
		{
			/* Stop data acquisition */
			AccessClass::getControlPanel()->setAcquisitionState(false);
		}
		return String("OK");

	} else if (String("StartRecord").compareIgnoreCase(inputs[0]) == 0)
	{
		for (int i=1; i<inputs.size(); i++)
		{
			StringArray nameValuePair;
			nameValuePair.addTokens(inputs[i], "=", "");

			if (nameValuePair[0].compareIgnoreCase("CreateNewDateDirectory") == 0)
			{
				if (nameValuePair[1].compareIgnoreCase("1") == 0)
				{
					/* This will handle all UI stuff */
					AccessClass::getControlPanel()->labelTextChanged(NULL);
				}
			}
			else if (nameValuePair[0].compareIgnoreCase("RecordingDirectory") == 0)
			{
				/* This will send all notification events */
				AccessClass::getControlPanel()->setRecordingDirectory(nameValuePair[1]);
			}
			else if (nameValuePair[0].compareIgnoreCase("PrependText") == 0)
			{
				AccessClass::getControlPanel()->setPrependText(nameValuePair[1]);
			}
			else if (nameValuePair[0].compareIgnoreCase("AppendText") == 0)
			{
				AccessClass::getControlPanel()->setAppendText(nameValuePair[1]);
			}
		}

		AccessClass::getControlPanel()->setRecordState(true);

		return String("OK");

	} else if (String("StopRecord").compareIgnoreCase(inputs[0]) == 0)
	{
		bool stopAcquisition = false;
		for (int i=1; i<inputs.size(); i++)
		{
			StringArray nameValuePair;
			nameValuePair.addTokens(inputs[i], "=", "");

			if (nameValuePair[0].compareIgnoreCase("StopAcquisition") == 0)
			{
				if (nameValuePair[1].compareIgnoreCase("1") == 0)
				{
					stopAcquisition = true;
				}
			}
		}

		/* This might be redundant if stopAcquisition is true */
		AccessClass::getControlPanel()->setRecordState(false);

		if (stopAcquisition)
		{
			AccessClass::getControlPanel()->setAcquisitionState(false);
		}

		return String("OK");

	} else
	{
		return String("NotHandled");
	}
}

