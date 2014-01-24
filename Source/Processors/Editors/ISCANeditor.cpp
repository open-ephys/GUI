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
	
		int h = getHeight();
		int w = getWidth();
		

	g.fillAll(Colours::grey);
	Array<EyePosition> buf = processor->getEyeBuffer();
	g.setColour(juce::Colours::green);
	g.drawEllipse(processor->getFixationSpotX()/(processor->screenCenterX*2)*w-15,
				  processor->getFixationSpotY()/(processor->screenCenterY*2)*h-15,
				  30,30,1);

	if (buf.size() > 0)
	{
		
		// paint the current eye position. Stretch things to GUI monitor size
		for (int k=0;k<buf.size();k++)
		{
			if (k == buf.size()-1)
			{
				g.setColour(juce::Colours::green);
				float xc =  buf[k].xc;
				float yc =  buf[k].yc;
				float fx = xc/(processor->screenCenterX*2) * w -3;
				float fy = yc/(processor->screenCenterY*2) * h -3;
				g.fillEllipse(fx,fy,6,6);

			} else
			{
				g.setColour(juce::Colour::fromRGB(float(k)/buf.size()*255.0,0,0));
				float x0 = buf[k].xc/(processor->screenCenterX*2) * w -5;
				float y0 = buf[k].yc/(processor->screenCenterY*2) * h -5; 
				float x1 = buf[k+1].xc/(processor->screenCenterX*2) * w -5;
				float y1 = buf[k+1].yc/(processor->screenCenterY*2) * h -5; 
				if (x0 >=0 && y0 >=0 && x1 <= w && y1 <= h)
					g.drawLine(x0,y0,x1,y1,1);
			}

								  

			
		}
		
	}
	repaint();
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
	gainXedt->addListener(this);
	gainXedt->setColour(Label::textColourId, Colours::white);
	gainXedt->setVisible(false);

	gainYedt= new Label("Y gain:", String(processor->getGainY()));
    gainYedt->setBounds(60,70,80,25);
	gainYedt->setEditable(true);
	gainYedt->addListener(this);
    addAndMakeVisible(gainYedt);

	gainYedt->setColour(Label::textColourId, Colours::white);
	gainYedt->setVisible(false);

    setEnabledState(false);

}


Visualizer* ISCANeditor::createNewCanvas() 
{
   ISCANnode* processor = (ISCANnode*) getProcessor();
   ISCANcanvas* canvas = new ISCANcanvas(this,processor);
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

void ISCANeditor::labelTextChanged(Label *label)
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();
	
	if (label == gainXedt)
	{
		processor->setGainX( label->getText().getDoubleValue());
	} else if (label == gainYedt)
	{
		processor->setGainY( label->getText().getDoubleValue());
	}

}

void ISCANeditor::comboBoxChanged(ComboBox* comboBox)
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();

}

ISCANeditor::~ISCANeditor()
{

}


