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

#include "OSCEditor.h"
#include "OSCNode.h"
#include "../../AccessClass.h"
#include "../../UI/EditorViewport.h"

// PipelineSelectorButton::PipelineSelectorButton()
// 	: DrawableButton ("Selector", DrawableButton::ImageFitted)
// {
// 	DrawablePath normal, over, down;

// 	    Path p;
//         p.addTriangle (0.0f, 0.0f, 0.0f, 20.0f, 18.0f, 10.0f);
//         normal.setPath (p);
//         normal.setFill (Colours::lightgrey);
//         normal.setStrokeThickness (0.0f);

//         over.setPath (p);
//         over.setFill (Colours::black);
//         over.setStrokeFill (Colours::black);
//         over.setStrokeThickness (5.0f);

//         setImages (&normal, &over, &over);
//         setBackgroundColours(Colours::darkgrey, Colours::purple);
//         setClickingTogglesState (true);
//         setTooltip ("Toggle a state.");

// }

// PipelineSelectorButton::~PipelineSelectorButton()
// {
// }

OSCEditor::OSCEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
    desiredWidth = 180;

    positionLabel = new Label("PositionLabel", "(X, Y)");
    positionLabel->setBounds(30,25,140,20);
    addAndMakeVisible(positionLabel);

}

OSCEditor::~OSCEditor()
{
    deleteAllChildren();
}

void OSCEditor::buttonEvent(Button* button)
{
    if (button == pipelineSelectorA)
    {
        pipelineSelectorA->setToggleState(true, dontSendNotification);
        pipelineSelectorB->setToggleState(false, dontSendNotification);
        OSCNode* processor = (OSCNode*) getProcessor();
        processor->switchIO(0);

		AccessClass::getEditorViewport()->makeEditorVisible(this, false);

    }
    else if (button == pipelineSelectorB)
    {
        pipelineSelectorB->setToggleState(true, dontSendNotification);
        pipelineSelectorA->setToggleState(false, dontSendNotification);
        OSCNode* processor = (OSCNode*) getProcessor();
        processor->switchIO(1);

		AccessClass::getEditorViewport()->makeEditorVisible(this, false);

    }
}

void OSCEditor::switchDest(int dest)
{
    if (dest == 0)
    {
        pipelineSelectorA->setToggleState(true, dontSendNotification);
        pipelineSelectorB->setToggleState(false, dontSendNotification);
        OSCNode* processor = (OSCNode*) getProcessor();
        processor->switchIO(0);

    }
    else if (dest == 1)
    {
        pipelineSelectorB->setToggleState(true, dontSendNotification);
        pipelineSelectorA->setToggleState(false, dontSendNotification);
        OSCNode* processor = (OSCNode*) getProcessor();
        processor->switchIO(1);

    }

	AccessClass::getEditorViewport()->makeEditorVisible(this, false);
}

void OSCEditor::switchIO(int dest)
{
    switchDest(dest);

    select();
}

int OSCEditor::getPathForEditor(GenericEditor* editor)
{
    OSCNode* processor = (OSCNode*) getProcessor();

    for (int pathNum = 0; pathNum < 2; pathNum++)
    {
        processor->switchIO();

        if (processor->getDestNode() != nullptr)
        {
            if (processor->getDestNode()->getEditor() == editor)
                return processor->getPath();
        }
    }

    return -1;

}


Array<GenericEditor*> OSCEditor::getConnectedEditors()
{

    Array<GenericEditor*> editors;

    OSCNode* processor = (OSCNode*) getProcessor();

    for (int pathNum = 0; pathNum < 2; pathNum++)
    {
        processor->switchIO();

        if (processor->getDestNode() != nullptr)
            editors.add(processor->getDestNode()->getEditor());
        else
            editors.add(nullptr);
    }

    return editors;

}

void OSCEditor::setPositionLabel(float x, float y)
{
	String text = "(" + String(x) + ", " + String(y) + ")";

	positionLabel->setText(text, juce::sendNotificationSync);
}

void OSCEditor::switchDest()
{
    OSCNode* processor = (OSCNode*) getProcessor();
    processor->switchIO();

    int path = processor->getPath();

    if (path == 0)
    {
        pipelineSelectorA->setToggleState(true, dontSendNotification);
        pipelineSelectorB->setToggleState(false, dontSendNotification);

    }
    else if (path == 1)
    {
        pipelineSelectorB->setToggleState(true,dontSendNotification);
        pipelineSelectorA->setToggleState(false, dontSendNotification);

    }
}
