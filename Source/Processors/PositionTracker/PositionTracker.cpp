/*
  ==============================================================================

    PositionTracker.cpp
    Created: 5 Oct 2015 11:34:58am
    Author:  mikkel

  ==============================================================================
*/

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "PositionTracker.h"
#include "PositionTrackerEditor.h"

//==============================================================================
PositionTracker::PositionTracker()
    : GenericProcessor("PositionTracker")
{
}

PositionTracker::~PositionTracker()
{
}

AudioProcessorEditor *PositionTracker::createEditor()
{
    editor = new PositionTrackerEditor(this, true);
    return editor;
}

void PositionTracker::process(AudioSampleBuffer &buffer, MidiBuffer &events)
{
    checkForEvents(events);
}

void PositionTracker::handleEvent(int eventType, MidiMessage &event, int samplePosition)
{
    if(eventType == MESSAGE) {
        const uint8* rawData = event.getRawData();
        const float* message = (float*)(rawData+6);
        m_x = message[0];
        m_y = message[1];
        m_positionIsUpdated = true;
    }
}

float PositionTracker::x() const
{
    return m_x;
}

float PositionTracker::y() const
{
    return m_y;
}

void PositionTracker::clearPositionUpdated()
{
    m_positionIsUpdated = false;
}

bool PositionTracker::positionIsUpdated() const
{
    return m_positionIsUpdated;
}
