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

Position::Position(float xin, float yin, float widthin, float heightin)
    : x(xin)
    , y(yin)
    , width(widthin)
    , height(heightin){

}

PositionTrackerCanvas::PositionTrackerCanvas(PositionTracker *positionTracker)
    : processor(positionTracker)
    , m_x(0.0)
    , m_y(0.0)
    , m_width(1.0)
    , m_height(1.0)
{
    clearButton = new UtilityButton("Clear plot", Font("Small Text", 13, Font::plain));
    clearButton->setRadius(3.0f);
    clearButton->addListener(this);
    addAndMakeVisible(clearButton);

    startCallbacks();
    update();
}

PositionTrackerCanvas::~PositionTrackerCanvas()
{
    TopLevelWindow::getTopLevelWindow(0)->removeKeyListener(this);
}

void PositionTrackerCanvas::paint (Graphics& g)
{
    // set aspect ratio to cam size
    float aC = m_width / m_height;
    float aS = getWidth() / getHeight();
    int camHeight = (aS > aC) ? getHeight() : getHeight() * (aS / aC);
    int camWidth = (aS < aC) ? getWidth() : getWidth() * (aC / aS);

    g.setColour(Colours::black); // backbackround color
    g.fillRect(0, 0, getWidth(), getHeight());

    g.setColour(Colour(0,18,43)); //background color
    g.fillRect((getWidth()-camWidth)/2, (getHeight()-camHeight)/2, camWidth, camHeight);

    float x_prev = m_x;
    float y_prev = m_y;
    for(auto position : m_positions) {
        float x = camWidth*position.x + (getWidth()-camWidth)/2;
        float y = camHeight*position.y + (getHeight()-camHeight)/2;
        g.drawLine(x_prev, y_prev, x, y, 1.0f);

        g.setFont(Font("Default", 16, Font::plain));

        g.setColour(Colour(100,100,100));
        x_prev = x;
        y_prev = y;

    }
}

void PositionTrackerCanvas::resized()
{
    clearButton->setBounds(10, getHeight()-40, 100,20);
    refresh();

}


bool PositionTrackerCanvas::keyPressed(const KeyPress &key, Component *originatingComponent)
{
	return false;
}

void PositionTrackerCanvas::buttonClicked(Button* button)
{
    if (button == clearButton)
    {
        clear();
    }
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
        m_positions.push_back(Position(processor->x(), processor->y(), processor->width(), processor->height()));
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

void PositionTrackerCanvas::clear()
{

    m_positions.clear();
    repaint();
}
