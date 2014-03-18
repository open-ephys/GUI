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

#include "OscilloscopeEditor.h"
#include "../OscilloscopeNode.h"
#include "../../UI/EditorViewport.h"
#include "../TrialCircularBuffer.h"
#include <stdio.h>





/***************************/

Colour OscilloscopeChannelButton::getDefaultColor(int ID)
{
	int IDmodule = (ID) % 32; // ID can't be zero
	const int colors[32][3] = {

		{26, 188, 156},
	{46, 204, 113},
	{52, 152, 219},
	{155, 89, 182},
	{241, 196, 15},
	{52, 73, 94},
	{230, 126, 34},
	{231, 76, 60},
	{22, 160, 133},
	{39, 174, 96},
	{41, 128, 185},
	{142, 68, 173},
	{44, 62, 80},
	{243, 156, 18},
	{211, 84, 0},
	{192, 57, 43},

		{224,185,36},
  {214,210,182},
  {243,119,33},
  {186,157,168},
  {237,37,36},
  {179,122,79},
  {217,46,171},
  {217, 139,196},
  {101,31,255},
  {141,111,181},
  {48,117,255},
  {184,198,224},
  {116,227,156},
  {150,158,155},
  {82,173,0},
		{125,99,32}};

	return Colour(colors[IDmodule][0], colors[IDmodule][1], colors[IDmodule][2]);

}
OscilloscopeChannelButton::OscilloscopeChannelButton(ChannelList *cl, String label_, Font font_, bool ttlChannel_, int channel_, bool initialState) :
    label(label_), font(font_),ttlChannel(ttlChannel_), channel(channel_), channelList(cl)
{
	gainUpButton = new ColorButton(">", font);
	gainUpButton->setShowEnabled(false);
	gainUpButton->setColors(Colours::white, getDefaultColor(channel));
	gainUpButton->addListener(this);
	addAndMakeVisible(gainUpButton);

	gainDownButton = new ColorButton("<", font);
	gainDownButton->setShowEnabled(false);
	gainDownButton->setColors(Colours::white, getDefaultColor(channel));
	gainDownButton->addListener(this);
	addAndMakeVisible(gainDownButton);

	gainResetButton = new ColorButton("R", font);
	gainResetButton->setShowEnabled(false);
	gainResetButton->setColors(Colours::white, getDefaultColor(channel));
	gainResetButton->addListener(this);
	addAndMakeVisible(gainResetButton);

	channelNameButton = new ColorButton(label, font);
	channelNameButton->setShowEnabled(true);
	channelNameButton->setColors(Colours::white, getDefaultColor(channel));
	channelNameButton->addListener(this);
	channelNameButton->setEnabledState(initialState);
	addAndMakeVisible(channelNameButton);


	userDefinedData = -1;
    setEnabledState(true);
}

void OscilloscopeChannelButton::buttonClicked(Button *btn)
{
	ColorButton* cbtn = (ColorButton*) btn;
	

	if (cbtn == gainResetButton) {
		// reset gain
		channelList->setChannelDefaultGain(channel,ttlChannel);
	} else if (cbtn == gainUpButton) {
		// increase gain
		channelList->increaseChannelGain(channel,ttlChannel);
	} else if (cbtn == gainDownButton) {
		// decrease gain
		channelList->decreaseChannelGain(channel,ttlChannel);
	} else	if (cbtn == channelNameButton) { // toggle visibility
			bool prevState = cbtn->getEnabledState();
			cbtn->setEnabledState(!prevState);
			channelList->setChannelsVisibility(channel,ttlChannel,!prevState);
		}
}

void OscilloscopeChannelButton::setEnabledState(bool state)
{

    isEnabled = state;

    repaint();
}

void OscilloscopeChannelButton::setUserDefinedData(int d)
{
	userDefinedData = d;
}
int OscilloscopeChannelButton::getUserDefinedData()
{
	return userDefinedData;
}

void OscilloscopeChannelButton::resized()
{
	channelNameButton->setBounds(0,0,100,getHeight());
	gainDownButton->setBounds(100,0,40,getHeight());
	gainResetButton->setBounds(140,0,20,getHeight());
	gainUpButton->setBounds(160,0,40,getHeight());
}


/**********************************************************************/


ChannelList::ChannelList(OscilloscopeNode* n, Viewport *p, OscilloscopeCanvas*c) :
	processor(n), viewport(p), canvas(c)
{

	double gainValuesAr[19] = {0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000};
	gainValues.clear();
	for (int k=0;k<19;k++)
		gainValues.push_back(gainValuesAr[k]);

	numTTLchannels = 8;
	defaultTTLgainIndex = 12; // x 100
	defaultCHgainIndex = 6; // x1

	triggerTitle = new Label("TRIGGER","TRIGGER");
	triggerTitle->setBounds(0,0, 200,25);
	triggerTitle->setFont(Font("Default", 25, Font::plain));
	triggerTitle->setJustificationType(Justification::centred);
	triggerTitle->setColour(Label::backgroundColourId, Colours::darkgrey);
	triggerTitle->setColour(Label::textColourId, Colours::white);
	addAndMakeVisible(triggerTitle);


	triggerCombo = new ComboBox("Trigger");
	triggerCombo->setBounds(0,25, 100,25);
	triggerCombo->addListener(this);
	addAndMakeVisible(triggerCombo);

	slopeButton = new ColorButton("Auto", Font("Default", 20, Font::plain));
    slopeButton->setBounds(100,25,100,25);
	slopeButton->setShowEnabled(false);
	slopeButton->addListener(this);
    addAndMakeVisible(slopeButton);


	channelsTitle = new Label("CHANNELS","CHANNELS");
	channelsTitle->setBounds(0,50, 200,25);
	channelsTitle->setFont(Font("Default", 25, Font::plain));
	channelsTitle->setJustificationType(Justification::centred);
	channelsTitle->setColour(Label::textColourId, Colours::white);
	channelsTitle->setColour(Label::backgroundColourId, Colours::darkgrey);
	addAndMakeVisible(channelsTitle);


	freqButton = new ColorButton("Frequency Analyzer", Font("Default", 15, Font::plain));
    freqButton->setBounds(0,75,200,25);
	freqButton->setShowEnabled(false);
	freqButton->addListener(this);
    addAndMakeVisible(freqButton);


	allButton = new ColorButton("All", Font("Default", 20, Font::plain));
    allButton->setBounds(0,100,100,20);
	allButton->addListener(this);
    addAndMakeVisible(allButton);

	noneButton = new ColorButton("None", Font("Default", 20, Font::plain));
    noneButton->setBounds(100,100,100,20);
	noneButton->addListener(this);
    addAndMakeVisible(noneButton);
	
	updateButtons();

}

void ChannelList::comboBoxChanged(ComboBox *b)
{
	if (b == triggerCombo)
	{
		int selectedID = triggerCombo->getSelectedId();
		if (selectedID >= 1 && selectedID <= numTTLchannels)
		{
			int selectedTTLchannel = selectedID-1;
			processor->setTriggerChannel(selectedTTLchannel,true);
		} else {
			int selectedchannel = selectedID-numTTLchannels-1;
			processor->setTriggerChannel(selectedchannel,false);
		}
		updateThresholdLineVisibility();
	}
}

void ChannelList::increaseChannelGain(int channel, bool ttlChannel)
{
	int chIndex = (ttlChannel)?  channel : channel+numTTLchannels;
	channelGain[chIndex]++;
	if (channelGain[chIndex] >= gainValues.size())
		channelGain[chIndex] = gainValues.size()-1;

	repaint();
}

void ChannelList::decreaseChannelGain(int channel, bool ttlChannel)
{
	int chIndex = (ttlChannel)?  channel : channel+numTTLchannels;
	channelGain[chIndex]--;
	if (channelGain[chIndex] < 0)
		channelGain[chIndex] = 0;

	repaint();
}

void ChannelList::setChannelDefaultGain(int channel, bool ttlChannel)
{
	if (ttlChannel)
	{
		channelGain[channel] = defaultTTLgainIndex;
	} else
	{
		channelGain[numTTLchannels+channel] = defaultCHgainIndex;
	}
	repaint();
}


int ChannelList::getNumChannels()
{
	return processor->getNumInputs() +numTTLchannels;
}

std::vector<float> ChannelList::getChannelsGain()
{
	std::vector<float> out;
	out.resize(channelGain.size());
	for (int k=0;k<channelGain.size();k++)
		out[k] = gainValues[channelGain[k]];
	return out;
}

std::vector<bool> ChannelList::getChannelsVisibility()
{
	return channelVisible;
}


void ChannelList::setChannelsVisibility(int channel, bool ttlChannel, bool status)
{
	int chIndex = (ttlChannel)?  channel : channel+numTTLchannels;
	channelVisible[chIndex] = status;
}

bool ChannelList::getChannelsVisibility(int channel, bool ttlChannel)
{
	int chIndex = (ttlChannel)?  channel : channel+numTTLchannels;
	return channelVisible[chIndex] ;
}


juce::Colour ChannelList::getChannelColor(int ch)
{
	return channelButtons[ch]->getDefaultColor(ch);
}

void ChannelList::updateButtons()
{

	int numChannels = processor->getNumInputs();

	channelVisible.resize(numTTLchannels+numChannels);
	channelGain.resize(numTTLchannels+numChannels);
	channelOffsetY.resize(numTTLchannels+numChannels);
	for (int k=0;k<numChannels+numTTLchannels;k++)
	{
		channelVisible[k] = false;
		channelGain[k] = (k < numTTLchannels) ? defaultTTLgainIndex : defaultCHgainIndex; 
		channelOffsetY[k] = 0.0;
	}
	if (numChannels > 0)
		channelVisible[numTTLchannels] = true;

	channelButtons.clear();
	for (int k=0;k<numTTLchannels;k++)
		{
			OscilloscopeChannelButton* channelButton = new OscilloscopeChannelButton(this,"TTL "+String(k+1), Font("Default", 20, Font::plain),true,k,channelVisible[k]);
			channelButton->setBounds(0,125+k*20,200,20);
			channelButton->setUserDefinedData(k);
			addAndMakeVisible(channelButton);
			channelButtons.add(channelButton);
		}	

	for (int k=0;k<numChannels;k++)
		{
			OscilloscopeChannelButton* channelButton = new OscilloscopeChannelButton(this,"CH "+String(k+1), Font("Default", 20, Font::plain),false,k,channelVisible[k+numTTLchannels]);
			channelButton->setBounds(0,125+(k+numTTLchannels)*20,200,20);
			channelButton->setUserDefinedData(k);
			addAndMakeVisible(channelButton);
			channelButtons.add(channelButton);
		}

	triggerCombo->clear();
	for (int k=0;k<numTTLchannels;k++)
		triggerCombo->addItem("TTL "+String(k+1),1+k);

	for (int k=0;k<processor->getNumInputs();k++)
		triggerCombo->addItem("CH "+String(k+1),1+k+numTTLchannels);

	int triggerChannel = processor->getTriggerChannel();
	bool ttlTrigger = processor->isTTLtrigger();
	triggerCombo->setSelectedId(1);

}

void ChannelList::update()
{
	updateButtons();
}


ChannelList::~ChannelList()
{
	
	for (int i = 0; i < channelButtons.size(); i++)
    {
        removeChildComponent(channelButtons[i]);
    }
}
	
void ChannelList::paint(Graphics& g)
{
	g.fillAll(juce::Colours::grey);

}

void ChannelList::updateThresholdLineVisibility()
{
	if (processor->isTTLtrigger() || processor->getTriggerType() == AUTO)
	{
		canvas->oscilloscopePlot->setThresholdLineVisibility(false);
	} else
	{ 
		canvas->oscilloscopePlot->setThresholdLineVisibility(true);
		canvas->oscilloscopePlot->setThresholdLineValue(processor->getTriggerThreshold());
	}
}

void ChannelList::buttonClicked(Button *btn)
{
	// also inform trial circular buffer about visibility change.
	if (btn == slopeButton)
	{

	switch (processor->getTriggerType()) {
		case  AUTO:
			processor->setTriggerType(RISING);
			slopeButton->setLabel("Rising");
			break;
		case RISING:
			slopeButton->setLabel("Falling");
			processor->setTriggerType(FALLING);
			break;
		case FALLING:
			slopeButton->setLabel("Auto");
			processor->setTriggerType(AUTO);
			break;
		}
		updateThresholdLineVisibility();
	} else
	 if (btn == noneButton)
	{
	} else if (btn == allButton)
	{
	} else if (btn == freqButton)
	{
		bool newState = !freqButton->getToggleState();
		freqButton->setToggleState(newState,false);
		if (newState)
		{
			freqButton->setColors(juce::Colours::white,juce::Colours::orange);
			OscilloscopeEditor* ed = (OscilloscopeEditor*)processor->getEditor();
			ed->oscilloscopeCanvas->oscilloscopePlot->getRange(saved_xmin,saved_xmax,saved_ymin,saved_ymax);
			ed->oscilloscopeCanvas->oscilloscopePlot->setRange(0,100,0,50,false);
			ed->oscilloscopeCanvas->oscilloscopePlot->setAutoRescale(true);
		}
		else {
			freqButton->setColors(juce::Colours::white,juce::Colours::darkgrey);
			OscilloscopeEditor* ed = (OscilloscopeEditor*)processor->getEditor();
			ed->oscilloscopeCanvas->oscilloscopePlot->setRange(saved_xmin,saved_xmax,saved_ymin,saved_ymax,false);
			ed->oscilloscopeCanvas->oscilloscopePlot->setAutoRescale(true);
		}

		processor->setFrequencyAnalyzerMode(newState);
	} else 
	{
		OscilloscopeChannelButton *cbtn = (OscilloscopeChannelButton *)btn;
		// probably a condition button
		int conditionID = cbtn->getUserDefinedData();
		cbtn->setEnabledState(!cbtn->getEnabledState());
	}

	repaint();
}


/**********************************************************************/
OscilloscopeEditor::OscilloscopeEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
	: VisualizerEditor(parentNode, useDefaultParameterEditors) ,oscilloscopeCanvas(nullptr)
{
	tabText = "Oscilloscope";
	desiredWidth = 300;
}



void OscilloscopeEditor::saveVisualizerParameters(XmlElement* xml)
{

     XmlElement* xmlNode = xml->createNewChildElement("OSCILLOSCOPE_EDITOR");
}

void OscilloscopeEditor::loadVisualizerParameters(XmlElement* xml)
{
    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("OSCILLOSCOPE_EDITOR"))
		{

		}
	}
}


void OscilloscopeEditor::comboBoxChanged(ComboBox* comboBox)
{

}

void OscilloscopeEditor::buttonEvent(Button* button)
{
	VisualizerEditor::buttonEvent(button);
	if (oscilloscopeCanvas == nullptr)
		return;

	OscilloscopeNode* processor = (OscilloscopeNode*) getProcessor();


}


Visualizer* OscilloscopeEditor::createNewCanvas()
{
	OscilloscopeNode* processor = (OscilloscopeNode*) getProcessor();
	oscilloscopeCanvas = new OscilloscopeCanvas(processor);
	ActionListener* listener = (ActionListener*) oscilloscopeCanvas;
    getUIComponent()->registerAnimatedComponent(listener);
	return oscilloscopeCanvas;
}

void OscilloscopeEditor::updateCanvas()
{
/*	if (periStimulusTimeHistogramCanvas != nullptr) {
		periStimulusTimeHistogramCanvas->updateNeeded = true;
		periStimulusTimeHistogramCanvas->repaint();
	}
	*/
}

OscilloscopeEditor::~OscilloscopeEditor()
{


}


/********************************/

OscilloscopeCanvas::OscilloscopeCanvas(OscilloscopeNode* n) :
	processor(n)
{
	screenWidth = screenHeight = 0;
	oscilloscopePlot = new MatlabLikePlot();
	oscilloscopePlot->setControlButtonsVisibile(true);
	addAndMakeVisible(oscilloscopePlot);

	channelsViewport = new Viewport();
	channelsList = new ChannelList(n, channelsViewport, this);
	channelsViewport->setViewedComponent(channelsList, false);
	channelsViewport->setScrollBarsShown(true, false);
	addAndMakeVisible(channelsViewport);

	resized();
	update();

}


void OscilloscopeCanvas::setThresholdLineVisibility(bool state)
{
	oscilloscopePlot->setThresholdLineVisibility(state);
}

OscilloscopeCanvas::~OscilloscopeCanvas()
{

}

void OscilloscopeCanvas::beginAnimation()
{
	//startCallbacks();
	
}

void OscilloscopeCanvas::buttonClicked(Button* button)
{

}

void OscilloscopeCanvas::endAnimation()
{
	std::cout << "SpikeDisplayCanvas ending animation." << std::endl;

	//stopCallbacks();
}



void OscilloscopeCanvas::refreshState()
{
	resized();
}


void OscilloscopeCanvas::update()
{
/*
	resized();


	psthDisplay->resized();
	psthDisplay->repaint();
	psthDisplay->refresh();
	repaint();

	processor->trialCircularBuffer->unlockPSTH();
	conditionsList->updateConditionButtons();
	*/
}


void OscilloscopeCanvas::resized()
{
	screenWidth = getWidth();
	screenHeight = getHeight();

	/*
	viewport->setBounds(0,0,getWidth()-conditionWidth,getHeight());
	int totalHeight = numRows * heightPerElectrodePix;
	int totalWidth = numCols * widthPerUnit;
	psthDisplay->setBounds(0,0,totalWidth, totalHeight);

	int numConditions = 0;
	if (processor->trialCircularBuffer != nullptr)
	{
		numConditions = processor->trialCircularBuffer->conditions.size();
	}
	*/
	int scrollBarThickness = channelsViewport->getScrollBarThickness();
	int channelsWidth = 200+scrollBarThickness;
	int numChannels = channelsList->getNumChannels();
	channelsViewport->setBounds(getWidth()-channelsWidth,0,channelsWidth,getHeight());
	channelsList->setBounds(0,0,channelsWidth, 250+20*numChannels);
	
	int minSize = min(getHeight(), getWidth()-channelsWidth);
	oscilloscopePlot->setBounds(0,0,minSize,minSize);

}

void OscilloscopeCanvas::paint(Graphics& g)
{
	g.fillAll(Colours::grey);

	if (oscilloscopePlot->eventsAvail())
	{
		String eventFromMlp = oscilloscopePlot->getLastEvent();
		if (eventFromMlp == "ThresholdChange")
		{
			processor->setTriggerThreshold(oscilloscopePlot->getThresholdLineValue());
		}
	}

	if (processor->trialCircularBuffer == nullptr)
		return;

	oscilloscopePlot->clearplot();
	std::vector<int> channels = processor->trialCircularBuffer->getElectrodeChannels(0);
	int numTTLchannels = 8;
	std::vector<float> gains = channelsList->getChannelsGain();
	std::vector<bool> vis = channelsList->getChannelsVisibility();

	int lastID = processor->trialCircularBuffer->getLastTrialID();
	if (lastTrialID != lastID)
	{
		lastTrialID = lastID;
		oscilloscopePlot->setTriggered();
	}


	const ScopedLock myScopedLock (processor->trialCircularBuffer->psthMutex);

	//processor->trialCircularBuffer->lockPSTH();
	int n = channels.size();

	for (int k=0;k<n;k++)
	{
		int nTrials = processor->trialCircularBuffer->getNumTrialsInCondition(0,k,0);
		if (vis[k+numTTLchannels] && nTrials > 0)
		{
			float x0,dx;
			std::vector<float> y;
			processor->trialCircularBuffer->getLastTrial(0,k,0,x0,dx,y);
			XYline l(x0,dx,y,  gains[k+numTTLchannels], channelsList->getChannelColor(k+numTTLchannels));
			if (processor->getFrequencyAnalyzerMode())
			{
				XYline lf = l.getFFT();
				oscilloscopePlot->plotxy(lf);
			} else
			{
				oscilloscopePlot->plotxy(l);
			}

			
		}
	}
	//processor->trialCircularBuffer->unlockPSTH();
}

void OscilloscopeCanvas::refresh()
{
	repaint();

}

