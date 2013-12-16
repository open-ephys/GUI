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

#include "ISCANeditor.h"
#include "../ISCAN.h"
#include <stdio.h>

ISCANeditor::ISCANeditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
	desiredWidth = 220;

	communication = new ToggleButton("Serial Communication");//,Font("Default", 15, Font::plain));
	communication->addListener(this);
	communication->setColour(Label::textColourId, Colours::white);
	communication->setBounds(10,30,160,20);
	communication->setToggleState(true, false);
	addAndMakeVisible(communication);

    urlLabel = new Label("Device:", "Device:");
    urlLabel->setBounds(20,60,60,25);
    addAndMakeVisible(urlLabel);
	urlLabel->setVisible(true);

	deviceList = new ComboBox("SerialDevices");
	deviceList->setEditableText(false);
	deviceList->setJustificationType(Justification::centredLeft);
	deviceList->addListener(this);
	deviceList->setBounds(80,60,130,20);
	addAndMakeVisible(deviceList);
	refreshDevices();
	deviceList->setVisible(true);


	devXlbl = new Label("X Channel:", "X Channel:");
    devXlbl->setBounds(20,60,80,25);
    addAndMakeVisible(devXlbl);
	devXlbl->setVisible(false);

	devYlbl = new Label("Y Channel:", "Y Channel:");
    devYlbl->setBounds(20,90,80,25);
    addAndMakeVisible(devYlbl);
	devYlbl->setVisible(false);



	devX = new ComboBox("DeviceX");
	devX->setEditableText(false);
	devX->setJustificationType(Justification::centredLeft);
	devX->addListener(this);
	devX->setBounds(90,60,110,20);
	addAndMakeVisible(devX);
	devX->setVisible(false);

	devY = new ComboBox("DeviceX");
	devY->setEditableText(false);
	devY->setJustificationType(Justification::centredLeft);
	devY->addListener(this);
	devY->setBounds(90,90,110,20);
	addAndMakeVisible(devY);
	devY->setVisible(false);

    setEnabledState(false);

}

void ISCANeditor::refreshAnalogDevices()
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();
	StringArray analogDevices = processor->getAnalogDeviceNames();
	devX->clear();
	devY->clear();
	devX->addItemList(analogDevices,1);
	devY->addItemList(analogDevices,1);
}

void ISCANeditor::refreshDevices()
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();
	StringArray devices = processor->getDeviceNames();
	deviceList->clear();
	deviceList->addItemList(devices,1);
}

void ISCANeditor::buttonEvent(Button* button)
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();
	if (button == communication)
	{
		if (communication->getToggleState())
		{
			communication->setButtonText("Serial Communication");
			urlLabel->setVisible(true);
  		   refreshDevices();
			deviceList->setVisible(true);

			devXlbl->setVisible(false);
			devYlbl->setVisible(false);
			devX->setVisible(false);
			devY->setVisible(false);

		}
		else 
		{
			communication->setButtonText("Analog Communication");
			urlLabel->setVisible(false);
			deviceList->setVisible(false);

			devXlbl->setVisible(true);
			devYlbl->setVisible(true);
			devX->setVisible(true);
			devY->setVisible(true);
			refreshAnalogDevices();

		}
	}
}

void ISCANeditor::comboBoxChanged(ComboBox* comboBox)
{
	int selectedDevice = comboBox->getSelectedId();
	if (selectedDevice > 0)
	{
		ISCANnode *processor  = (ISCANnode*) getProcessor();
		processor->connect(selectedDevice);
	
	}
}

ISCANeditor::~ISCANeditor()
{

}


