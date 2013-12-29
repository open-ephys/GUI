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
      createNewFilesOnTrigger(false), triggerChannel(0), recordNode(0),eventsSavedBySink(true)
{
	firstTime=true;
}

RecordControl::~RecordControl()
{

}

StringArray RecordControl::getChannelNames()
{
	return names;
}

void RecordControl::updateSettings()
{
    if (recordNode == 0)
        recordNode = getProcessorGraph()->getRecordNode();

	int numExistingNames = names.size();
	int numNewNames = getNumInputs();
	StringArray oldnames = names;

    names.clear();
    for (int k = 0; k < getNumInputs(); k++)
    {
        names.add(channels[k]->getName());
    }

	if (numNewNames >= numExistingNames)
	{
		// keep existing changes.
		names.clear();
		for (int k = 0; k < getNumInputs(); k++)
		{
			if (k < numExistingNames)
			{
				channels[k]->setName(oldnames[k]);
				recordNode->updateChannelName(k, oldnames[k]);
				names.add(oldnames[k]);
			}
			else
				names.add(channels[k]->getName());
		}
	} else
	{
		// just chop off , but keep names.
		// keep existing changes.
		StringArray oldnames = names;
		names.clear();
		for (int k = 0; k < getNumInputs();k++)
		{
				channels[k]->setName(oldnames[k]);
				recordNode->updateChannelName(k, oldnames[k]);
				names.add(oldnames[k]);
		}
	}

	const MessageManagerLock mmLock;
	RecordControlEditor *ed = (RecordControlEditor*) getEditor();
	if (ed != nullptr)
		ed->updateNames();
}

AudioProcessorEditor* RecordControl::createEditor()
{
    editor = new RecordControlEditor(this, true);
    return editor;
}

void RecordControl::setParameter(int parameterIndex, float newValue)
{
	int x = getNumInputs();

    if (parameterIndex == 0)
    {
        updateTriggerChannel((int) newValue);
    } else if (parameterIndex == 1 ){
        
        if (newValue == 0.0)
        {
            createNewFilesOnTrigger = false;
            
            
        } else {
            createNewFilesOnTrigger = true;
        }
        //recordNode->appendTrialNumber(createNewFilesOnTrigger);
    } else if (parameterIndex == 2 ){
        
		eventsSavedBySink = newValue > 0;
        }
}

void RecordControl::modifyChannelName(int ch, String newname)
{
    if (recordNode == 0)
        recordNode = getProcessorGraph()->getRecordNode();

    if (ch < getNumInputs())
    {
    	channels[ch]->setName(newname);
    	recordNode->updateChannelName(ch, newname);

    	names.clear();
    	for (int k = 0; k < getNumInputs(); k++)
    	{
    		names.add(channels[k]->getName());
    	}
    }


}

void RecordControl::updateTriggerChannel(int newChannel)
{
    triggerChannel = newChannel;
}

bool RecordControl::enable()
{
    if (recordNode == 0)
        recordNode = getProcessorGraph()->getRecordNode();
    
    recordNode->appendTrialNumber(createNewFilesOnTrigger);
    recordNode->setEventSavingState(eventsSavedBySink);
		

    return true;
}

void RecordControl::startRecording() 
{ 
	RecordControlEditor *ed = (RecordControlEditor *)getEditor();
	ed->disableButtons();
}


void RecordControl::stopRecording() 
{ 
	RecordControlEditor *ed = (RecordControlEditor *)getEditor();
	ed->enableButtons();
}

void RecordControl::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events,
                            int& nSamples)
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

        //std::cout << "Trigger!" << std::endl;

        const MessageManagerLock mmLock;

        if (eventId == 1)
        {
            if (createNewFilesOnTrigger)
            {
                recordNode->updateTrialNumber();
            }
            getControlPanel()->setRecordState(true);
        }
        else
        {
            getControlPanel()->setRecordState(false);
        }


    }

}