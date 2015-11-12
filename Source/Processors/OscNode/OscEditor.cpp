/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#include "OscEditor.h"
#include "OscNode.h"
#include "../../AccessClass.h"
#include "../../UI/EditorViewport.h"

OscEditor::OscEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
    desiredWidth = 180;

    OscNode *processor = (OscNode *)getProcessor();

    adrLabel = new Label("Address", "Address:");
    adrLabel->setBounds(10,80,140,25);
    addAndMakeVisible(adrLabel);
    DBG("in editor set default address");
    String defaultAddress = "/red";
    labelAdr = new Label("Address", defaultAddress);
    labelAdr->setBounds(80,85,80,18);
    labelAdr->setFont(Font("Default", 15, Font::plain));
    labelAdr->setColour(Label::textColourId, Colours::white);
    labelAdr->setColour(Label::backgroundColourId, Colours::grey);
    labelAdr->setEditable(true);
    labelAdr->addListener(this);
    addAndMakeVisible(labelAdr);
    processor->setAddress(defaultAddress);

    urlLabel = new Label("Port", "Port:");
    urlLabel->setBounds(10,40,140,25);
    addAndMakeVisible(urlLabel);

    int defaultPort = 27020;
    labelPort = new Label("Port", String(defaultPort));
    labelPort->setBounds(80,45,80,18);
    labelPort->setFont(Font("Default", 15, Font::plain));
    labelPort->setColour(Label::textColourId, Colours::white);
    labelPort->setColour(Label::backgroundColourId, Colours::grey);
    labelPort->setEditable(true);
    labelPort->addListener(this);
    addAndMakeVisible(labelPort);
    processor->setPort(defaultPort);
}

OscEditor::~OscEditor()
{
    // TODO should we delete all children, check JUCE docs
    // PS: Causes segfault if we do right now
//    deleteAllChildren();
}

void OscEditor::labelTextChanged(Label *label)
{
    if (label == labelAdr)
    {
       Value val = label->getTextValue();

        OscNode *p= (OscNode *)getProcessor();
        p->setAddress(val.getValue());
    }
    if (label == labelPort)
    {
       Value val = label->getTextValue();

        OscNode *p= (OscNode *)getProcessor();
        p->setPort(val.getValue());
    }
}



void OscEditor::saveCustomParameters(XmlElement *parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("OSCNODE");
    mainNode->setAttribute("port", labelPort->getText());
    mainNode->setAttribute("address", labelAdr->getText());
}

void OscEditor::loadCustomParameters(XmlElement *parametersAsXml)
{
    if (parametersAsXml != nullptr)
    {
        forEachXmlChildElement(*parametersAsXml, mainNode)
        {
            if (mainNode->hasTagName("OSCNODE"))
            {
                labelPort->setText(mainNode->getStringAttribute("port"),sendNotification);
                labelAdr->setText(mainNode->getStringAttribute("address"),sendNotification);
            }
        }
    }
}
