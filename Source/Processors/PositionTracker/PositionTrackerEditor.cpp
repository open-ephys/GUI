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
