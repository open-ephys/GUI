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

#include "NotchFilterEditor.h"
#include "../NotchFilterNode.h"
#include <stdio.h>


NotchFilterEditor::NotchFilterEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
    desiredWidth = 150;

    centerString = " ";
    bandwidthString = " ";

    centerLabel = new Label("center freq", "Center Freq:");
    centerLabel->setBounds(10,25,80,20);
    centerLabel->setFont(Font("Small Text", 12, Font::plain));
    centerLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(centerLabel);

    bandwidthLabel = new Label("bandwidth", "Bandwidth:");
    bandwidthLabel->setBounds(10,65,80,20);
    bandwidthLabel->setFont(Font("Small Text", 12, Font::plain));
    bandwidthLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(bandwidthLabel);

    centerValue = new Label("center freq value", centerString);
    centerValue->setBounds(15,42,60,18);
    centerValue->setFont(Font("Default", 15, Font::plain));
    centerValue->setColour(Label::textColourId, Colours::white);
    centerValue->setColour(Label::backgroundColourId, Colours::grey);
    centerValue->setEditable(true);
    centerValue->addListener(this);
    centerValue->setTooltip("Set the center frequency for the selected channels");
    addAndMakeVisible(centerValue);

    bandwidthValue = new Label("bandwidth label", bandwidthString);
    bandwidthValue->setBounds(15,82,60,18);
    bandwidthValue->setFont(Font("Default", 15, Font::plain));
    bandwidthValue->setColour(Label::textColourId, Colours::white);
    bandwidthValue->setColour(Label::backgroundColourId, Colours::grey);
    bandwidthValue->setEditable(true);
    bandwidthValue->addListener(this);
    bandwidthValue->setTooltip("Set the bandwitdh for the selected channels");
    addAndMakeVisible(bandwidthValue);

	applyFilterOnADC = new UtilityButton("+ADCs",Font("Default", 10, Font::plain));
    applyFilterOnADC->addListener(this);
    applyFilterOnADC->setBounds(90,70,40,18);
    applyFilterOnADC->setClickingTogglesState(true);
    applyFilterOnADC->setTooltip("When this button is off, ADC channels will not be filtered");
    addAndMakeVisible(applyFilterOnADC);

    applyFilterOnChan = new UtilityButton("+CH",Font("Default", 10, Font::plain));
    applyFilterOnChan->addListener(this);
    applyFilterOnChan->setBounds(95,95,30,18);
    applyFilterOnChan->setClickingTogglesState(true);
    applyFilterOnChan->setToggleState(true, false);
    applyFilterOnChan->setTooltip("When this button is off, selected channels will not be filtered");
    addAndMakeVisible(applyFilterOnChan);
 
}

NotchFilterEditor::~NotchFilterEditor()
{
    
}   

void NotchFilterEditor::setDefaults(double center, double band)
{
    bandwidthString = String(roundFloatToInt(band));
    centerString = String(roundFloatToInt(center));

    centerValue->setText(centerString, dontSendNotification);
    bandwidthValue->setText(bandwidthString, dontSendNotification);
}


void NotchFilterEditor::labelTextChanged(Label* label)
{
    NotchFilterNode* fn = (NotchFilterNode*) getProcessor();

    Value val = label->getTextValue();
    double requestedValue = double(val.getValue());

    if (requestedValue < 0.01 || requestedValue > 10000) 
    {
        sendActionMessage("Value out of range.");

        if (label == bandwidthValue)
        {
            label->setText(bandwidthString, dontSendNotification);
			bandwidthString = label->getText();
        }
        else
        {
            label->setText(centerString, dontSendNotification);
			centerString = label->getText();
        }

        return;
    }

    Array<int> chans = getActiveChannels();

    // This needs to change, since there's not enough feedback about whether
    // or not individual channel settings were altered:

    for (int n = 0; n < chans.size(); n++)
    {

        if (label == bandwidthValue)
        {
            //double minVal = fn->getBandwidthValueForChannel(chans[n]);

            //if (requestedValue > minVal)
            {
                fn->setCurrentChannel(chans[n]);
                fn->setParameter(1, requestedValue);
            }

            bandwidthString = label->getText();

        }
        else
        {
            //double maxVal = fn->getCenterFreqValueForChannel(chans[n]);

            //if (requestedValue < maxVal)
            {
                fn->setCurrentChannel(chans[n]);
                fn->setParameter(0, requestedValue);
            }

            centerString = label->getText();
        }

    }

}

void NotchFilterEditor::channelChanged(int chan)
{
    NotchFilterNode* fn = (NotchFilterNode*) getProcessor();

    centerValue->setText(String(fn->getCenterFreqValueForChannel(chan)), dontSendNotification);
    bandwidthValue->setText(String(fn->getBandWidthValueForChannel(chan)), dontSendNotification);
    applyFilterOnChan->setToggleState(fn->getBypassStatusForChannel(chan), false);

}

void NotchFilterEditor::buttonEvent(Button* button)
{

    if (button == applyFilterOnADC)
    {
        NotchFilterNode* fn = (NotchFilterNode*) getProcessor();
        fn->setApplyOnADC(applyFilterOnADC->getToggleState());

    } else if (button == applyFilterOnChan)
    {
        NotchFilterNode* fn = (NotchFilterNode*) getProcessor();

        Array<int> chans = getActiveChannels();

        for (int n = 0; n < chans.size(); n++)
        {
            float newValue = button->getToggleState() ? 1.0 : 0.0;

            fn->setCurrentChannel(chans[n]);
            fn->setParameter(2, newValue);
        }
    }
}


void NotchFilterEditor::saveCustomParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "NotchFilterEditor");

    XmlElement* textLabelValues = xml->createNewChildElement("VALUES");
    textLabelValues->setAttribute("BandWidth",bandwidthString);
    textLabelValues->setAttribute("CenterFreq",centerString);
	textLabelValues->setAttribute("ApplyToADC",	applyFilterOnADC->getToggleState());
}

void NotchFilterEditor::loadCustomParameters(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("VALUES"))
        {
            bandwidthValue->setText(xmlNode->getStringAttribute("BandWidth"),dontSendNotification);
            centerValue->setText(xmlNode->getStringAttribute("CenterFreq"),dontSendNotification);
			applyFilterOnADC->setToggleState(xmlNode->getBoolAttribute("ApplyToADC",false),true);
        }
    }
}
