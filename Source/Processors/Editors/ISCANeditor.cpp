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
	static int x = 0;
	x++;

	g.fillAll(Colours::grey);
	if (x % 2 == 0)
		g.setColour(juce::Colours::blueviolet);
	else
		g.setColour(juce::Colours::darkgreen);

	g.fillRect(50,100,300,400);

	// paint the current eye position. Stretch things to GUI monitor size
	int h = getHeight();
	int w = getWidth();

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
    ISCANnode* processor = (ISCANnode*) getProcessor();

	tabText = "Eye Tracking";
	desiredWidth = 220;

	configButton = new UtilityButton("Config",Font("Default", 15, Font::plain));
	configButton->addListener(this);
	configButton->setColour(Label::textColourId, Colours::white);
	configButton->setBounds(10,30,70,20);
	addAndMakeVisible(configButton);

	clearButton = new UtilityButton("Clear",Font("Default", 15, Font::plain));
	clearButton->addListener(this);
	clearButton->setColour(Label::textColourId, Colours::white);
	clearButton->setBounds(100,30,70,20);
	addAndMakeVisible(clearButton);
	
	gainX = new Label("X gain:", "X gain:");
    gainX->setBounds(10,50,80,25);
    addAndMakeVisible(gainX);
	gainX->setVisible(false);

	gainY= new Label("Y gain:", "Y gain:");
    gainY->setBounds(10,70,80,25);
    addAndMakeVisible(gainY);
	gainY->setVisible(false);


	gainXedt = new Label("X gain:", String(processor->getGainX()));
    gainXedt->setBounds(60,50,80,25);
    addAndMakeVisible(gainXedt);
	gainXedt->setEditable(true);
	gainXedt->setColour(Label::textColourId, Colours::white);
	gainXedt->setVisible(false);

	gainYedt= new Label("Y gain:", String(processor->getGainY()));
    gainYedt->setBounds(60,70,80,25);
	gainYedt->setEditable(true);
    addAndMakeVisible(gainYedt);
	gainYedt->setColour(Label::textColourId, Colours::white);
	gainYedt->setVisible(false);

    setEnabledState(false);

}


Visualizer* ISCANeditor::createNewCanvas() 
{
   ISCANnode* processor = (ISCANnode*) getProcessor();
   ISCANcanvas* canvas = new ISCANcanvas(this,processor);
   ActionListener* listener = (ActionListener*) canvas;
   getUIComponent()->registerAnimatedComponent(listener);
	return canvas;
}


void ISCANeditor::collapsedStateChanged()
{
    buttonEvent(communication);
}

void ISCANeditor::buttonCallback(Button* button)
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();
	if (button == clearButton)
	{
	} else if (button == configButton)
	{
		ISCANnode *processor  = (ISCANnode*) getProcessor();
		StringArray devices = processor->getDeviceNames();

		PopupMenu m;
		PopupMenu sourceSubMenu,analogSubMenu,serialSubMenu,XchannelMenu,YchannelMenu,analogSampleRateMenu,calibSubMenu;
		// update serial device list
		for (int k=0;k<devices.size();k++)
		{
			serialSubMenu.addItem(1+k, devices[k], true, processor->getSerialDevice() == devices[k]);
		}

		// update analog device list
		Array<int> channelNumbers;
		StringArray analogDevices = processor->getAnalogDeviceNames(channelNumbers);
		for (int j=0;j<analogDevices.size();j++)
		{
			XchannelMenu.addItem(1000+channelNumbers[j], analogDevices[j],true, processor->getXchannel() ==channelNumbers[j]);
			YchannelMenu.addItem(2000+channelNumbers[j], analogDevices[j],true, processor->getYchannel() ==channelNumbers[j]);
		}
		int sampleRates[6] = {50,60,80,100,120,200};
		for (int i=0;i<6;i++)
		{
			analogSampleRateMenu.addItem(100+i,String(sampleRates[i])+" Hz",true, processor->getAnalogSamplingRate() == sampleRates[i]);
		}

		analogSubMenu.addSubMenu("X", XchannelMenu);
		analogSubMenu.addSubMenu("Y", YchannelMenu);
		analogSubMenu.addSubMenu("Sample Rate", analogSampleRateMenu);

		sourceSubMenu.addSubMenu("Serial", serialSubMenu,true,juce::Image::null,processor->getSerialCommunication());
		sourceSubMenu.addSubMenu("Analog", analogSubMenu,true,juce::Image::null,!processor->getSerialCommunication());

		calibSubMenu.addItem(3000,"None",true,processor->getCalibrationMode() == 0);
		calibSubMenu.addItem(3001,"2 DOF (Fixed Gain)",true,processor->getCalibrationMode() == 1);
		calibSubMenu.addItem(3002,"4 DOF (linear model)",false,processor->getCalibrationMode() == 2);

		m.addSubMenu("Source", sourceSubMenu);
		m.addSubMenu("Calibration", calibSubMenu);
	
		const int result = m.show();
		if (result <= 0)
			return;

		if (result < 100)
		{
			// selected serial communication.
			processor->setSerialCommunication(true);
			processor->setSerialDevice(devices[result-1]);
		
		} else	if (result < 106)
		{
			// analog sample rate
			processor->setSerialCommunication(false);
			processor->setSamplingRate(sampleRates[result - 100]);
		} else if (result < 2000)
		{
			// analog communication
			processor->setSerialCommunication(false);
			processor->setXchannel(result-1000);
		} else if (result < 3000)
		{
			// analog communication
			processor->setSerialCommunication(false);
			processor->setYchannel(result-2000);
		} else if (result < 4000)
		{
			processor->setCalibrationMode(result-3000);
			if (result-3000 == 1)
			{
				gainX->setVisible(true);
				gainY->setVisible(true);
				gainXedt->setVisible(true);
				gainYedt->setVisible(true);

			} else 
			{
				gainX->setVisible(false);
				gainY->setVisible(false);
				gainXedt->setVisible(false);
				gainYedt->setVisible(false);

			}

		}

	}
	
}

void ISCANeditor::comboBoxChanged(ComboBox* comboBox)
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();

}

ISCANeditor::~ISCANeditor()
{

}


