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

#ifndef __PSTHEDITOR_H
#define __PSTHEDITOR_H


#include "../../../JuceLibraryCode/JuceHeader.h"

#include "GenericEditor.h"
#include "../../UI/UIComponent.h"
#include "../../UI/DataViewport.h"
#include "../Visualization/DataWindow.h"
#include "VisualizerEditor.h"

class Condition;

class TrialCircularBuffer;
class PeriStimulusTimeHistogramNode;

class XYPlot : public Component
{
public:
	XYPlot(bool spikePlot, TrialCircularBuffer *_tcb, int _electrodeID, int _unitID, int _row, int _col);
	void resized();
	void paint(Graphics &g);

	void mouseDown(const juce::MouseEvent& event);

	juce::Font font;
	int row, col;
	int electrodeID, unitID;
	TrialCircularBuffer *tcb;
private:
	bool spikePlot;
	std::vector<int> histc(std::vector<float> xi, std::vector<float> x);
	std::vector<float> diff(std::vector<float> x);
	void interp1(std::vector<float> x, std::vector<float>y, std::vector<float> xi, std::vector<float> &yi, std::vector<bool> &valid);

};
class PeriStimulusTimeHistogramCanvas;

// this component holds all the individual PSTH plots
class PeriStimulusTimeHistogramDisplay : public Component
{
public:
	PeriStimulusTimeHistogramDisplay(PeriStimulusTimeHistogramNode* n, Viewport *p, PeriStimulusTimeHistogramCanvas*c);
	void resized();
	std::vector<XYPlot*> psthPlots;
	void paint(Graphics &g);
	void refresh();

	PeriStimulusTimeHistogramNode* processor;
	Viewport *viewport;
	PeriStimulusTimeHistogramCanvas* canvas;

	juce::Font font;
	  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeriStimulusTimeHistogramDisplay);

};

class PeriStimulusTimeHistogramCanvas: public Visualizer, public Button::Listener
{
public:
    PeriStimulusTimeHistogramCanvas(PeriStimulusTimeHistogramNode* n);
    ~PeriStimulusTimeHistogramCanvas();

    void paint(Graphics& g);

    void refresh();

    void beginAnimation();
    void endAnimation();

	void refreshState();
    void update();

    void resized();
	void buttonClicked(Button* button);


	
	void setParameter(int, float) {}
	void setParameter(int, int, int, float) {}

    void startRecording() { } // unused
    void stopRecording() { } // unused

	int numElectrodes;
	int maxUnitsPerElectrode;
	int heightPerElectrodePix;
	int widthPerUnit;
	bool updateNeeded;

   private:
	PeriStimulusTimeHistogramNode *processor;
    ScopedPointer<Viewport> viewport;
	PeriStimulusTimeHistogramDisplay *psthDisplay;




    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeriStimulusTimeHistogramCanvas);

};



class PeriStimulusTimeHistogramEditor : public VisualizerEditor,
	public Label::Listener, public ComboBox::Listener
{
public:
    PeriStimulusTimeHistogramEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~PeriStimulusTimeHistogramEditor();
    Visualizer* createNewCanvas();
	void labelTextChanged(juce::Label *) {}
	void updateCondition(std::vector<Condition> conditions);
	void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
	void updateCanvas();
private:
	PeriStimulusTimeHistogramCanvas *periStimulusTimeHistogramCanvas;
    Font font;
	ComboBox* visibleConditions;
	ToggleButton *smoothPSTH;
	ToggleButton *autoRescale;
	ToggleButton *lfp, *spikes;
	Label *smoothMS, *condlabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeriStimulusTimeHistogramEditor);

};




#endif  // PSTHEDITOR
