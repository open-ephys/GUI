/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __OSCILLOSCOPE_EDITOR_H
#define __OSCILLOSCOPE_EDITOR_H


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Visualization/MatlabLikePlot.h"
#include "GenericEditor.h"
#include "../OscilloscopeNode.h"
#include "../../UI/UIComponent.h"
#include "../../UI/DataViewport.h"
#include "../Visualization/DataWindow.h"
#include "VisualizerEditor.h"

class TrialCircularBuffer;
class ProcessorListItem;
class UIComponent;
class OscilloscopeCanvas;

class OscilloscopeNode;

class OscilloscopeChannelButton;

class ChannelList : public Component,
	public AccessClass, Button::Listener, ComboBox::Listener

{
public:
    ChannelList(OscilloscopeNode* n, Viewport *p, OscilloscopeCanvas*c);
    ~ChannelList();

    void paint(Graphics& g);
	void buttonClicked(Button *btn);
	void update();
	void updateButtons();
	int getNumChannels();
	void comboBoxChanged(ComboBox *b);
	juce::Colour getChannelColor(int ch);
	bool getChannelsVisibility(int channel, bool ttlChannel);
	void setChannelsVisibility(int channel, bool ttlChannel, bool status);
	void increaseChannelGain(int channel, bool ttlChannel);
	void decreaseChannelGain(int channel, bool ttlChannel);
	void setChannelDefaultGain(int channel, bool ttlChannel);
	std::vector<float> getChannelsGain();
	std::vector<bool> getChannelsVisibility();

private:
	void updateThresholdLineVisibility();
	float saved_xmin,saved_xmax,saved_ymin,saved_ymax;
	int defaultTTLgainIndex,defaultCHgainIndex;
	std::vector<float> gainValues;

	std::vector<bool> channelVisible;
	std::vector<int> channelGain;
	std::vector<float> channelOffsetY;
	float defaultTTLgain;

	int numTTLchannels;
	OscilloscopeNode* processor;
	Viewport *viewport;
	OscilloscopeCanvas *canvas;

	ScopedPointer<Label> triggerTitle,channelsTitle;
	ScopedPointer<ComboBox> triggerCombo;
	ScopedPointer<ColorButton> allButton,noneButton, slopeButton,freqButton;
	OwnedArray<OscilloscopeChannelButton> channelButtons;
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelList);
};

class OscilloscopeChannelButton : public Component, public AccessClass, Button::Listener
{
public:
    OscilloscopeChannelButton(ChannelList* cl, String label_, Font font_, bool ttlChannel, int channel, bool initialState);
    ~OscilloscopeChannelButton() {}
	Colour getDefaultColor(int ID);

    void setEnabledState(bool);
    bool getEnabledState()
    {
        return isEnabled;
    }
	void buttonClicked(Button *btn);
	void setUserDefinedData(int d);
	int getUserDefinedData();
	void resized();
private:
	ChannelList* channelList;
	ScopedPointer<ColorButton> gainUpButton,gainDownButton,gainResetButton,channelNameButton;
	bool ttlChannel;
	int channel;
    String label;
	int userDefinedData;
	Font font;
    bool isEnabled;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeChannelButton);
};


class OscilloscopeCanvas: public Visualizer, public Button::Listener
{
public:
    OscilloscopeCanvas(OscilloscopeNode* n);
    ~OscilloscopeCanvas();

    void paint(Graphics& g);

    void refresh();
	

	void setParameter(int a,int b,int c,float d) {}
	void setParameter(int a,float d) {}
    void beginAnimation();
    void endAnimation();

	void refreshState();
    void update();

    void resized();
	void buttonClicked(Button* button);
	void setThresholdLineVisibility(bool state);
    void startRecording() { } // unused
    void stopRecording() { } // unused

	int screenHeight, screenWidth;
	ScopedPointer<MatlabLikePlot> oscilloscopePlot;

   private:
	int lastTrialID;
	OscilloscopeNode *processor;
    ScopedPointer<Viewport> viewport, channelsViewport;
	ScopedPointer<ChannelList> channelsList;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeCanvas);

};



class OscilloscopeEditor : public VisualizerEditor,
	 public ComboBox::Listener
{
public:
    OscilloscopeEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~OscilloscopeEditor();
    Visualizer* createNewCanvas();
	void comboBoxChanged(ComboBox* comboBox);
	void updateCanvas();
	void buttonEvent(Button* button);
	void saveVisualizerParameters(XmlElement* xml);
	void loadVisualizerParameters(XmlElement* xml);
	OscilloscopeCanvas *oscilloscopeCanvas;
	void update();
private:
    Font font;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeEditor);

};



#endif  // PSTHEDITOR
