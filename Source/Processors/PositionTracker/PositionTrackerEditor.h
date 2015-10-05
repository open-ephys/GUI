/*
  ==============================================================================

    PositionTrackerEditor.h
    Created: 5 Oct 2015 11:35:12am
    Author:  mikkel

  ==============================================================================
*/

#ifndef POSITIONTRACKEREDITOR_H_INCLUDED
#define POSITIONTRACKEREDITOR_H_INCLUDED


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"
#include "../Editors/VisualizerEditor.h"


//==============================================================================
/*
*/
class PositionTrackerEditor : public VisualizerEditor
{
public:
    PositionTrackerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    ~PositionTrackerEditor();

    void buttonCallback(Button* button);

    Visualizer* createNewCanvas();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PositionTrackerEditor)
};


#endif  // POSITIONTRACKEREDITOR_H_INCLUDED
