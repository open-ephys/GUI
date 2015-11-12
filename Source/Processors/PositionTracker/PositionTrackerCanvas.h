/*
  ==============================================================================

    PositionTrackerCanvas.h
    Created: 5 Oct 2015 12:09:00pm
    Author:  mikkel

  ==============================================================================
*/

#ifndef POSITIONTRACKERCANVAS_H_INCLUDED
#define POSITIONTRACKERCANVAS_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Visualization/Visualizer.h"
#include "../Editors/GenericEditor.h"
#include <vector>

class PositionTracker;

class Position
{
public:
    Position(float xin, float yin, float widthin, float heightin);
    float x;
    float y;
    float width;
    float height;
};

//==============================================================================
/*
*/
class PositionTrackerCanvas : public Visualizer,
        public ComboBox::Listener,
        public Button::Listener,
        public KeyListener
{
public:
    PositionTrackerCanvas(PositionTracker* positionTracker);
    ~PositionTrackerCanvas();

    void paint (Graphics&);
    void resized();
    void clear();
    // KeyListener interface
    virtual bool keyPressed(const KeyPress &key, Component *originatingComponent);

    // Listener interface
    virtual void buttonClicked(Button* button);

    // Listener interface
    virtual void comboBoxChanged(ComboBox *comboBoxThatHasChanged);

    // Visualizer interface
    virtual void refreshState();
    virtual void update();
    virtual void refresh();
    virtual void beginAnimation();
    virtual void endAnimation();
    virtual void setParameter(int, float);
    virtual void setParameter(int, int, int, float);

//    void saveVisualizerParameters(XmlElement* xml);
//    void loadVisualizerParameters(XmlElement* xml);

private:
    PositionTracker* processor;
    float m_x;
    float m_y;
    float m_width;
    float m_height;

    ScopedPointer<UtilityButton> clearButton;

    std::vector<Position> m_positions;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PositionTrackerCanvas)
};


#endif  // POSITIONTRACKERCANVAS_H_INCLUDED
