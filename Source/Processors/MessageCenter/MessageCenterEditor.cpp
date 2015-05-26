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

#include "MessageCenterEditor.h"

MessageCenterEditor::MessageCenterEditor(MessageCenter* owner) :
    AudioProcessorEditor(owner),
    acquisitionIsActive(false),
    messageCenter(owner),
    incomingBackground(100, 100, 100),
    outgoingBackground(100, 100, 100)
{

    incomingMessageDisplayArea = new MessageLabel("Message Display Area","No new messages.");
    outgoingMessageDisplayArea = new MessageLabel("Message Display Area","Type a new message here.");
    outgoingMessageDisplayArea->setEditable(true);

    addAndMakeVisible(incomingMessageDisplayArea);
    addAndMakeVisible(outgoingMessageDisplayArea);

    sendMessageButton = new UtilityButton("Save", Font("Small Text", 0, Font::plain));
    sendMessageButton->addListener(this);
    sendMessageButton->setTooltip("Send a message to be saved by the record node");
    addAndMakeVisible(sendMessageButton);

    sourceMenu = new PopupMenu();

}

MessageCenterEditor::~MessageCenterEditor()
{

}


void MessageCenterEditor::buttonClicked(Button* button)
{
    messageCenter->setParameter(1,1);

}

void MessageCenterEditor::labelTextChanged(Label* label)
{

}

void MessageCenterEditor::timerCallback()
{

    bool shouldRepaint = false;

    float incomingAlpha = incomingBackground.getFloatAlpha();

    if (incomingAlpha > 0)
    {
        incomingAlpha -= 0.05;

        if (incomingAlpha < 0)
            incomingAlpha = 0;

        incomingBackground = incomingBackground.withAlpha(incomingAlpha);

        shouldRepaint = true;
    }

    if (shouldRepaint)
        repaint();
    else
        stopTimer();

}

bool MessageCenterEditor::keyPressed(const KeyPress& key)
{
    return false;
}

String MessageCenterEditor::getLabelString()
{
    return outgoingMessageDisplayArea->getText();
}

void MessageCenterEditor::startAcquisition()
{
    acquisitionIsActive = true;
}

void MessageCenterEditor::stopAcquisition()
{
    acquisitionIsActive = false;
}

void MessageCenterEditor::enable()
{
    //sendMessageButton->setVisible(true);
}

void MessageCenterEditor::disable()
{
    //sendMessageButton->setVisible(false);
}

void MessageCenterEditor::messageReceived(bool state)
{
    if (!state)
    {
        incomingMessageDisplayArea->setText("Cannot save messages when recording is not active.", sendNotification);
        incomingBackground = Colours::red;
    }
    else
    {
        incomingMessageDisplayArea->setText("Message sent.", sendNotification);
        incomingBackground = Colours::green;
    }

    startTimer(75);
}

void MessageCenterEditor::paint(Graphics& g)
{

    g.setColour(Colour(58,58,58));

    g.fillRect(0, 0, getWidth(), getHeight());

    g.setColour(Colour(100,100,100));

    g.fillRect(5, 5, getWidth()/2-5, getHeight()-10);
    g.fillRect(getWidth()/2+5, 5, getWidth()/2-60, getHeight()-10);

    g.setColour(incomingBackground);

    g.fillRect(5, 5, getWidth()/2-5, getHeight()-10);

    g.setColour(outgoingBackground);

    g.fillRect(getWidth()/2+5, 5, getWidth()/2-60, getHeight()-10);

}

void MessageCenterEditor::resized()
{
    if (incomingMessageDisplayArea != 0)
        incomingMessageDisplayArea->setBounds(5,0,getWidth()/2-5,getHeight());

    if (outgoingMessageDisplayArea != 0)
        outgoingMessageDisplayArea->setBounds(getWidth()/2+5,0,getWidth()/2-60, getHeight());

    if (sendMessageButton != 0)
        sendMessageButton->setBounds(getWidth()-50, 5, 45, getHeight()-10);
}

int64 MessageCenterEditor::getTimestamp(bool softwareTimestamp)
{
    return messageCenter->getTimestamp(softwareTimestamp);
}

void MessageCenterEditor::actionListenerCallback(const String& message)
{

    incomingMessageDisplayArea->setText(message, sendNotification);

    incomingBackground = Colours::orange;
    startTimer(75);

}

void MessageCenterEditor::saveStateToXml(XmlElement* xml)
{
    XmlElement* messageEditorState = xml->createNewChildElement("MESSAGECENTER");
    messageEditorState->setAttribute("sourceNodeId",messageCenter->getSourceNodeId());
}

void MessageCenterEditor::loadStateFromXml(XmlElement* xml)
{
    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("MESSAGECENTER"))
        {
            messageCenter->setSourceNodeId(xmlNode->getIntAttribute("sourceNodeId"));
        }
    }
}

void MessageCenterEditor::addSourceProcessor(GenericProcessor* p)
{
    sourcesList.add(p);
}

void MessageCenterEditor::removeSourceProcessor(GenericProcessor* p)
{
    sourcesList.removeAllInstancesOf(p);
}

void MessageCenterEditor::mouseDown(const MouseEvent& event)
{
    int res;
    if (event.mods.isPopupMenu())
    {
        if (acquisitionIsActive)
        {
            CoreServices::sendStatusMessage("Cannot change timestamp source while acquisition is active");
            return;
        }
        PopupMenu::dismissAllActiveMenus();
        sourceMenu->clear();
        sourceMenu->addItem(1,"Software timer",true,messageCenter->getSourceNodeId() == 0);
        for (int i=0; i < sourcesList.size(); i++)
        {
            GenericProcessor* p = sourcesList[i];
            sourceMenu->addItem(i+2,p->getName(),true,(p->nodeId == messageCenter->getSourceNodeId()));
        }
        res = sourceMenu->show(0,50,0,0);

        if (res > 1)
        {
            GenericProcessor* p = sourcesList[res-2];
            std::cout << "Selecting " << p->getName() << " with id " << p->nodeId << " as message source" << std::endl;
            messageCenter->setSourceNodeId(p->nodeId);
        }
        else if (res == 1)
        {
            messageCenter->setSourceNodeId(0);
        }
    }
}

MessageLabel::MessageLabel(const String& componentName, const String& labelText)
    : Label(componentName,labelText)
{
}

void MessageLabel::mouseDown(const MouseEvent& event)
{
    if (event.mods.isPopupMenu())
    {
        getParentComponent()->mouseDown(event.getEventRelativeTo(getParentComponent()));
    }
}
