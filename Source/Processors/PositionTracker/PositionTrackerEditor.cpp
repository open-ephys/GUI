/*
  ==============================================================================

    PositionTrackerEditor.cpp
    Created: 5 Oct 2015 11:35:12am
    Author:  mikkel

  ==============================================================================
*/

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "PositionTrackerEditor.h"
#include "PositionTrackerCanvas.h"
#include "PositionTracker.h"

//==============================================================================
PositionTrackerEditor::PositionTrackerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors)
{
    tabText = "Tracking";
    desiredWidth = 180;
}

PositionTrackerEditor::~PositionTrackerEditor()
{
    deleteAllChildren();
}

Visualizer* PositionTrackerEditor::createNewCanvas()
{
    PositionTracker* processor = (PositionTracker*) getProcessor();
    return new PositionTrackerCanvas(processor);
}

void PositionTrackerEditor::initializeButtons()
{
    int w = 18;
    int h = 18;
    int xPad = 5;
    int yPad = 6;

    int xInitial = 10;
    int yInitial = 25;
    int x = xInitial;
    int y = yInitial;

    clearBtn = new UtilityButton("Clear", titleFont);
    clearBtn->setBounds(x, y, w*2 + xPad, h);
    clearBtn->setClickingTogglesState(false);
    clearBtn->addListener(this);
    addAndMakeVisible(clearBtn);

}

void PositionTrackerEditor::buttonCallback(Button* button)
{

    int gId = button->getRadioGroupId();

    if (gId > 0)
    {
        if (canvas != 0)
        {
            canvas->setParameter(gId-1, button->getName().getFloatValue());
        }

    }

}
