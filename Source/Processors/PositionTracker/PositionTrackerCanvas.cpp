/*
  ==============================================================================

    PositionTrackerCanvas.cpp
    Created: 5 Oct 2015 12:09:00pm
    Author:  mikkel

  ==============================================================================
*/

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "PositionTrackerCanvas.h"
#include "PositionTracker.h"

Position::Position(float xin, float yin)
    : x(xin)
    , y(yin) {

}

PositionTrackerCanvas::PositionTrackerCanvas(PositionTracker *positionTracker)
    : processor(positionTracker)
    , m_x(0.0)
    , m_y(0.0)
{
    startCallbacks();
    update();
}

PositionTrackerCanvas::~PositionTrackerCanvas()
{
}

void PositionTrackerCanvas::paint (Graphics& g)
{
    g.fillAll (Colours::white);   // clear the background

    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (Colours::lightblue);
    g.setFont (14.0f);
    g.drawText ("PositionTrackerCanvas", getLocalBounds(),
                Justification::centred, true);   // draw some placeholder text

    for(auto position : m_positions) {
        float x = position.x - 5.0;
        float y = position.y - 5.0;
        g.drawRect(x, y, 10.0, 10.0, 1.0);
    }
}

void PositionTrackerCanvas::resized()
{
}


bool PositionTrackerCanvas::keyPressed(const KeyPress &key, Component *originatingComponent)
{
}

void PositionTrackerCanvas::buttonClicked(Button *)
{
}

void PositionTrackerCanvas::comboBoxChanged(ComboBox *comboBoxThatHasChanged)
{
}

void PositionTrackerCanvas::refreshState()
{
}

void PositionTrackerCanvas::update()
{
}

void PositionTrackerCanvas::refresh()
{
    if(processor->positionIsUpdated()) {
        m_positions.push_back(Position(processor->x(), processor->y()));
        processor->clearPositionUpdated();
        repaint();
    }
}

void PositionTrackerCanvas::beginAnimation()
{
    startCallbacks();
}

void PositionTrackerCanvas::endAnimation()
{
    stopCallbacks();
}

void PositionTrackerCanvas::setParameter(int, float)
{
}

void PositionTrackerCanvas::setParameter(int, int, int, float)
{
}
