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

ISCANcanvas::ISCANcanvas(ISCANeditor* ed, ISCANnode* n) :
    processor(n), editor(ed)
{
    setWantsKeyboardFocus(true);
   update();
	 
}



ISCANcanvas::~ISCANcanvas()
{
}

void ISCANcanvas::paint(Graphics &g)
{


}

void ISCANcanvas::update()
{
	resized();
	repaint();
}


void ISCANcanvas::resized()
{
}

void ISCANcanvas::refresh()
{
	repaint();
}

void ISCANcanvas::refreshState()
{
    // called when the component's tab becomes visible again
    resized();
}


/*************************************/
ISCANeditor::ISCANeditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
	: VisualizerEditor(parentNode, useDefaultParameterEditors)

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
    devXlbl->setBounds(20,50,80,25);
    addAndMakeVisible(devXlbl);
	devXlbl->setVisible(false);

	devYlbl = new Label("Y Channel:", "Y Channel:");
    devYlbl->setBounds(20,80,80,25);
    addAndMakeVisible(devYlbl);
	devYlbl->setVisible(false);

	devX = new ComboBox("DeviceX");
	devX->setEditableText(false);
	devX->setJustificationType(Justification::centredLeft);
	devX->addListener(this);
	devX->setBounds(110,55,90,20);
	addAndMakeVisible(devX);
	devX->setVisible(false);

	devY = new ComboBox("DeviceY");
	devY->setEditableText(false);
	devY->setJustificationType(Justification::centredLeft);
	devY->addListener(this);
	devY->setBounds(110,85,90,20);
	addAndMakeVisible(devY);
	devY->setVisible(false);


	sampleLbl = new Label("Sample Rate", "Sample Rate:");
    sampleLbl->setBounds(20,110,80,25);
    addAndMakeVisible(sampleLbl);
	sampleLbl->setVisible(false);


	sampleRate = new ComboBox("sampleRate");
	sampleRate->setEditableText(false);
	sampleRate->setJustificationType(Justification::centredLeft);
	sampleRate->addListener(this);
	sampleRate->setBounds(110,110,90,20);
	addAndMakeVisible(sampleRate);
	sampleRate->addItem("50 Hz",1);
	sampleRate->addItem("60 Hz",2);
	sampleRate->addItem("100 Hz",3);
	sampleRate->addItem("120 Hz",4);
	sampleRate->addItem("200 Hz",5);
	sampleRate->setSelectedId(4,false);
	sampleRate->setVisible(false);

    setEnabledState(false);

}


	Visualizer* ISCANeditor::createNewCanvas() 
{
   ISCANnode* processor = (ISCANnode*) getProcessor();
   ISCANcanvas* canvas = new ISCANcanvas(this,processor);
	return canvas;
}


void ISCANeditor::refreshAnalogDevices()
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();
	Array<int> channelNumbers;
	StringArray analogDevices = processor->getAnalogDeviceNames(channelNumbers);
	devX->clear();
	devY->clear();
	for (int k=0;k<channelNumbers.size();k++)
	{
		devX->addItem(analogDevices[k], channelNumbers[k]);
		devY->addItem(analogDevices[k], channelNumbers[k]);
	}
}

void ISCANeditor::refreshDevices()
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();
	StringArray devices = processor->getDeviceNames();
	deviceList->clear();
	deviceList->addItemList(devices,1);
}

void ISCANeditor::collapsedStateChanged()
{
    buttonEvent(communication);
}

void ISCANeditor::buttonEvent(Button* button)
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();
	if (button == communication)
	{
		if (communication->getToggleState())
		{
			processor->setSerialCommunication(true);
			communication->setButtonText("Serial Communication");
			urlLabel->setVisible(true);
  		    refreshDevices();
			deviceList->setVisible(true);

			devXlbl->setVisible(false);
			devYlbl->setVisible(false);
			devX->setVisible(false);
			devY->setVisible(false);
			sampleRate->setVisible(false);
			sampleLbl->setVisible(false);
		}
		else 
		{
			processor->setSerialCommunication(false);
			communication->setButtonText("Analog Communication");
			urlLabel->setVisible(false);
			deviceList->setVisible(false);

			devXlbl->setVisible(true);
			devYlbl->setVisible(true);
			devX->setVisible(true);
			devY->setVisible(true);
			sampleRate->setVisible(true);
			sampleLbl->setVisible(true);
			refreshAnalogDevices();

		}
	}
}

void ISCANeditor::comboBoxChanged(ComboBox* comboBox)
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();

	if (comboBox == deviceList)
	{
		int selectedDevice = comboBox->getSelectedId();
		if (selectedDevice > 0)
		{
			processor->connect(selectedDevice);
		}
	} else if (comboBox == devX)
	{
		int xChannel = devX->getSelectedId();
		processor->setXchannel(xChannel);
	} else if (comboBox == devY)
	{
		int yChannel = devY->getSelectedId();
		processor->setYchannel(yChannel);
	} else if (comboBox == sampleRate)
	{
		int selectedID = sampleRate->getSelectedId();
		int selectedSamplingRate=0;
		switch (selectedID)
		{
		case 1:
			selectedSamplingRate =50;
			break;
		case 2:
			selectedSamplingRate =60;
			break;
		case 3:
			selectedSamplingRate =100;
			break;
		case 4:
			selectedSamplingRate =120;
			break;
		case 5:
			selectedSamplingRate =200;
			break;
		}
		processor->setSamplingRate(selectedSamplingRate );
	}
}

ISCANeditor::~ISCANeditor()
{

}


