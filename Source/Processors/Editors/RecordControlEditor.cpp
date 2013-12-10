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

#include "RecordControlEditor.h"
#include "../Utilities/RecordControl.h"
#include "ChannelSelector.h"
#include "../ProcessorGraph.h"
#include <stdio.h>

RecordControlEditor::RecordControlEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)
{
    desiredWidth = 230;

    //channelSelector->eventsOnly = true;

    chanSel = new Label("Channel Text","Trigger:");
    chanSel->setEditable(false);
	chanSel->setFont(Font("Default", 15, Font::plain));
    chanSel->setJustificationType(Justification::centredLeft);
    chanSel->setBounds(10,60,120,20);

    addAndMakeVisible(chanSel);


    chanRename = new Label("Channel Text","Ch Names:");
    chanRename->setEditable(false);
	chanRename->setFont(Font("Default", 15, Font::plain));
    chanRename->setJustificationType(Justification::centredLeft);
    chanRename->setBounds(10,30,120,20);

    addAndMakeVisible(chanRename);

	chanRenameCombo = new ComboBox("Channels");

    chanRenameCombo->setEditableText(true);
    chanRenameCombo->setJustificationType(Justification::centredLeft);
    chanRenameCombo->addListener(this);
    chanRenameCombo->setBounds(100,30,120,20);
    chanRenameCombo->setSelectedId(0);

    addAndMakeVisible(chanRenameCombo);
    

    availableChans = new ComboBox("Event Channels");

    availableChans->setEditableText(false);
    availableChans->setJustificationType(Justification::centredLeft);
    availableChans->addListener(this);
    availableChans->setBounds(100,60,120,20);
    availableChans->setSelectedId(0);

    addAndMakeVisible(availableChans);
    
    availableChans->addItem("None",1);
    for (int i = 0; i < 10 ; i++)
    {
        String channelName = "Channel ";
        channelName += i + 1;
        availableChans->addItem(channelName,i+2);
    }
	availableChans->setSelectedId(1);

    
    newFileToggleButton = new UtilityButton("SPLIT FILES", Font("Small Text", 13, Font::plain));
    newFileToggleButton->setRadius(3.0f);
    newFileToggleButton->setBounds(35, 85, 90, 18);
    newFileToggleButton->addListener(this);
    newFileToggleButton->setClickingTogglesState(true);
    addAndMakeVisible(newFileToggleButton);

  
    eventsBySink = new UtilityButton("SINK WRITE EVENTS", Font("Default", 10, Font::plain));
    eventsBySink->setRadius(3.0f);
    eventsBySink->setBounds(10, 110, 140, 18);
    eventsBySink->addListener(this);
    eventsBySink->setClickingTogglesState(true);
    addAndMakeVisible(eventsBySink);
	lastId = 0;
}

RecordControlEditor::~RecordControlEditor()
{
    
}

void RecordControlEditor::comboBoxChanged(ComboBox* comboBox)
{
	if (comboBox == availableChans)
	{
		if (comboBox->getSelectedId() > 1)
	        getProcessor()->setParameter(0, (float) comboBox->getSelectedId()-2);
	    else
			getProcessor()->setParameter(0, -1);
	} else if (comboBox == chanRenameCombo)
	{

	     int ID = comboBox->getSelectedId();
		 if (ID == 0)
			{
	            // name change
				getProcessor()->getProcessorGraph()->getRecordNode()->updateChannelName(lastId-1,comboBox->getText());
				updateSettings();
				comboBox->setSelectedId(lastId,dontSendNotification);
        }
		else
        {
            lastId = ID;
		}
	}
}

void RecordControlEditor::buttonEvent(Button* button)
{
	if (button == newFileToggleButton)
	{
		if (button->getToggleState())
		{
			getProcessor()->setParameter(1, 1.0f);
		} else {
			getProcessor()->setParameter(1, 0.0f);
		}
	} else if (button == eventsBySink)
	{
		getProcessor()->setParameter(2, eventsBySink->getToggleState());
	}
}

void RecordControlEditor::updateSettings()
{
    //availableChans->clear();
    //GenericProcessor* processor = getProcessor();
    
	// update number of recorded channels...
		const MessageManagerLock mmLock;
	Array<bool> isRecording;
	((RecordControl *)getProcessor())->getProcessorGraph()->getRecordNode()->getChannelNamesAndRecordingStatus(channelNames, isRecording);
	chanRenameCombo->clear();
	for (int k=0;k<channelNames.size();k++)
	{
		chanRenameCombo->addItem(channelNames[k],k+1);
	}
	
}

void RecordControlEditor::saveEditorParameters(XmlElement* xml)
{
    
    XmlElement* info = xml->createNewChildElement("PARAMETERS");
    
    info->setAttribute("Type", "RecordControlEditor");
    info->setAttribute("Channel",availableChans->getSelectedId());
    info->setAttribute("FileSaveOption",newFileToggleButton->getToggleState());
	info->setAttribute("EventSaveOption",eventsBySink->getToggleState());

	info->setAttribute("numChannels",channelNames.size());
    for (int k=0;k<channelNames.size();k++)
	{
		XmlElement* ch = xml->createNewChildElement("CHANNEL_NAME");
		ch->setAttribute("Name",channelNames[k]);
	}
}

void RecordControlEditor::loadEditorParameters(XmlElement* xml)
{
     
    forEachXmlChildElement(*xml, xmlNode)
    {
        
        if (xmlNode->hasTagName("PARAMETERS"))
        {
            newFileToggleButton->setToggleState(xmlNode->getBoolAttribute("FileSaveOption"), true);
            availableChans->setSelectedId(xmlNode->getIntAttribute("Channel"), sendNotification);
			eventsBySink->setToggleState(xmlNode->getBoolAttribute("EventSaveOption"), true);

			int numCh = xmlNode->getIntAttribute("numChannels");
			channelNames.clear();
			int ch = 0;
		    forEachXmlChildElement(*xml, xmlSubNode)
			{
				  if (xmlSubNode->hasTagName("CHANNEL_NAME"))
				  {
					  String chName = xmlSubNode->getStringAttribute("Name");
					  channelNames.add(chName);
				  }
			}
        }

    }
}