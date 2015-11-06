/*
  ==============================================================================

    PositionTracker.h
    Created: 5 Oct 2015 11:34:58am
    Author:  mikkel

  ==============================================================================
*/

#ifndef POSITIONTRACKER_H_INCLUDED
#define POSITIONTRACKER_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"

//==============================================================================
/*
*/
class PositionTracker : public GenericProcessor
{
public:
    PositionTracker();
    ~PositionTracker();

    AudioProcessorEditor* createEditor();

    virtual void process(AudioSampleBuffer& buffer, MidiBuffer& events);
    virtual void handleEvent(int eventType, MidiMessage &event, int samplePosition);

    float x() const;
    float y() const;
    float width() const;
    float height() const;

    void clearPositionUpdated();
    bool positionIsUpdated() const;

    bool isSink(); //get the color correct

private:
    float m_x;
    float m_y;
    float m_width;
    float m_height;
    bool m_positionIsUpdated;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PositionTracker)
};

inline bool PositionTracker::isSink()
{
    return true;
}


#endif  // POSITIONTRACKER_H_INCLUDED
