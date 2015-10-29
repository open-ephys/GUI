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
    TopLevelWindow::getTopLevelWindow(0)->removeKeyListener(this);
}

void PositionTrackerCanvas::paint (Graphics& g)
{
    g.setColour(Colour(0,18,43)); //background color
    g.fillRect(0, 0, getWidth(), getHeight());

    g.setGradientFill(ColourGradient(Colour(50,50,50),0,0,
                                     Colour(25,25,25),0,30,
                                     false));

    g.fillRect(0, 0, getWidth(), 30);

    g.setColour(Colours::black);

    float x_prev = m_x;
    float y_prev = m_y;
    for(auto position : m_positions) {
        float x = getWidth()*position.x;
        float y = getHeight()*position.y;
        g.drawLine(x_prev, y_prev, x, y, 1.0f);

        g.setFont(Font("Default", 16, Font::plain));

        g.setColour(Colour(100,100,100));
        x_prev = x;
        y_prev = y;

    }
}

void PositionTrackerCanvas::resized()
{
    refresh();

}


bool PositionTrackerCanvas::keyPressed(const KeyPress &key, Component *originatingComponent)
{
	return false;
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
