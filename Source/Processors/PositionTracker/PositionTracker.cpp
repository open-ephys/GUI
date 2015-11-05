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

using std::cout;
using std::endl;

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
    if(eventType == BINARY_MSG) {
        const uint8* rawData = event.getRawData();
        if(event.getRawDataSize() != 6 + sizeof(float)*2) {
            cout << "Position tracker got wrong event size: " << event.getRawDataSize() << endl;
        }
        const float* message = (float*)(rawData+6);
        m_x = message[0];
        m_y = message[1];
        m_positionIsUpdated = true;
        std::cout << "x " << m_x << " y " << m_y << std::endl;
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
