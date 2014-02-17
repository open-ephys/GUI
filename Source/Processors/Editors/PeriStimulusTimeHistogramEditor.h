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
#include "../Visualization/MatlabLikePlot.h"
#include "../PeriStimulusTimeHistogramNode.h"
#include "GenericEditor.h"
#include "../../UI/UIComponent.h"
#include "../../UI/DataViewport.h"
#include "../Visualization/DataWindow.h"
#include "VisualizerEditor.h"

class Condition;

class PeriStimulusTimeHistogramNode;
class PeriStimulusTimeHistogramDisplay;
class TrialCircularBuffer;
class MatlabLikePlot;

struct zoom
{
	float xmin,xmax,ymin,ymax;
};


    enum xyPlotTypes
    {
        SPIKE_PLOT = 0,
        LFP_PLOT = 1,
		EYE_PLOT = 2
    };
	/*

class XYPlot : public Component
{
public:
	XYPlot(PeriStimulusTimeHistogramDisplay* dsp, int _plotID, xyPlotTypes plotType, 
		TrialCircularBuffer *_tcb, int _electrodeID, int _unitID, int _row, int _col, bool _rasterMode);
	void resized();
	void paint(Graphics &g);
	void setSmoothState(bool enable);
	bool getSmoothState();

	void setAutoRescale(bool enable);
	bool getAutoRescale();

	void mouseDown(const juce::MouseEvent& event);
	void mouseDrag(const juce::MouseEvent& event);
	void mouseUp(const juce::MouseEvent& event);

	void buildSmoothKernel(float guassianKernelSizeMS);
	 void mouseDoubleClick(const juce::MouseEvent& event);
	int getPlotID();

	void paintSpikes(Graphics &g);
	void paintLFP(Graphics &g);
	void toggleFullScreen(bool fullScreenOn);
	bool isFullScreen();
	juce::Font font;
	int row, col;
	int electrodeID, unitID;
	TrialCircularBuffer *tcb;
private:
	void paintSpikeRaster(Graphics &g);
	void paintLFPraster(Graphics &g);
	void paintPlotNameAndRect(Graphics &g);
	void computeSamplePositions(float &xmin, float &xmax);
	void plotTicks(Graphics &g,float xmin, float xmax, float ymin, float ymax);

	void sampleConditions(float &minY, float &maxY);
	
	void sampleConditionsForLFP(float &minY, float &maxY);

	std::vector<int> histc(std::vector<float> xi, std::vector<float> x);
	std::vector<float> diff(std::vector<float> x);
	void interp1(std::vector<float> x, std::vector<float>y, std::vector<float> xi, std::vector<float> &yi, std::vector<bool> &valid, float &min, float &max);
	std::vector<float> smooth(std::vector<float> x);
	
	bool findIndices(int &electrodeIndex, int &entryindex, bool findUnitOrChannel);

	xyPlotTypes plotType;
	bool  smoothPlot,autoRescale,firstTime,rasterMode;
	float axesRange[4];

	std::vector<float> smoothKernel; 
	float guassianStandardDeviationMS;
	int electrodeIndex, entryindex,w,h,x0,y0,plotWidth, plotHeight;
	int subsample;
	std::vector<float> samplePositions;
	std::vector<std::vector<float>> interpolatedConditions;
	std::vector<std::vector<bool>> interpolatedConditionsValid;
	std::vector<float> conditionMaxY;
	std::vector<float> conditionMinY;

	
    Image rasterImage;

	float rangeX,rangeY;
	int plotID;
	bool fullScreenMode;
	float xmin, xmax, ymax, ymin;
	int mouseDownX, mouseDownY,mouseDragX,mouseDragY;
	bool zooming;
	std::list<zoom> zoomMemory;
	PeriStimulusTimeHistogramDisplay* display;
};*/

class PeriStimulusTimeHistogramCanvas;
class GenericPlot;
// this component holds all the individual PSTH plots
class PeriStimulusTimeHistogramDisplay : public Component
{
public:
	PeriStimulusTimeHistogramDisplay(PeriStimulusTimeHistogramNode* n, Viewport *p, PeriStimulusTimeHistogramCanvas*c);
	~PeriStimulusTimeHistogramDisplay();
	

	void setAutoRescale(bool state);
	void resized();

	std::vector<GenericPlot*> psthPlots;
	void paint(Graphics &g);
	void refresh();
	void focusOnPlot(int plotIndex);

	PeriStimulusTimeHistogramNode* processor;
	Viewport *viewport;
	PeriStimulusTimeHistogramCanvas* canvas;

	juce::Font font;

	
	  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeriStimulusTimeHistogramDisplay);

};







class ProcessorListItem;
class UIComponent;


class ConditionList : public Component,
	public AccessClass, Button::Listener

{
public:

    ConditionList(PeriStimulusTimeHistogramNode* n, Viewport *p, PeriStimulusTimeHistogramCanvas*c);
    ~ConditionList();

	/** Draws the ConditionList. */
    void paint(Graphics& g);
	void buttonClicked(Button *btn);
	void updateConditionButtons();

private:
	PeriStimulusTimeHistogramNode* processor;
	Viewport *viewport;
	PeriStimulusTimeHistogramCanvas *canvas;

	ScopedPointer<ColorButton> titleButton;
	ScopedPointer<ColorButton> allButton,noneButton;
	OwnedArray<ColorButton> conditionButtons;
    /** The main method for drawing the ProcessorList.*/
 //   void drawItems(Graphics& g);

	/** Called when a mouse click begins within the boundaries of the ProcessorList.*/
    //void mouseDown(const MouseEvent& e);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConditionList);

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

	void setRasterMode(bool rasterModeActive);
	void setLFPvisibility(bool visible);
	void setSpikesVisibility(bool visible);
	void setSmoothPSTH(bool smooth);
	void setSmoothing(float _gaussianStandardDeviationMS);
	void setAutoRescale(bool state);
	void setCompactView(bool compact);
	void setMatchRange(bool on);
	void setParameter(int, float) {}
	void setParameter(int, int, int, float) {}

    void startRecording() { } // unused
    void stopRecording() { } // unused

	int numElectrodes;
	int maxUnitsPerElectrode;
	int heightPerElectrodePix;
	int widthPerUnit;
	bool updateNeeded;
		int screenHeight, screenWidth;

   private:
	int conditionWidth;

    bool showLFP, showSpikes, smoothPlots, autoRescale,compactView, matchRange, inFocusedMode,rasterMode;
	PeriStimulusTimeHistogramNode *processor;
    ScopedPointer<Viewport> viewport, conditionsViewport;
	ScopedPointer<PeriStimulusTimeHistogramDisplay> psthDisplay;
	ScopedPointer<ConditionList> conditionsList;
	ScopedPointer<UtilityButton> visualizationButton, clearAllButton;
	float gaussianStandardDeviationMS;
	int numRows,numCols;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeriStimulusTimeHistogramCanvas);

};



class PeriStimulusTimeHistogramEditor : public VisualizerEditor,
	 public ComboBox::Listener
{
public:
    PeriStimulusTimeHistogramEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~PeriStimulusTimeHistogramEditor();
    Visualizer* createNewCanvas();
	void comboBoxChanged(ComboBox* comboBox);
	void updateCanvas();
	void buttonEvent(Button* button);
	bool showSortedUnits,showLFP,showCompactView,showSmooth,showAutoRescale,showMatchRange,showRasters;
	int TTLchannelTrialAlignment;
	int smoothingMS;

	void saveVisualizerParameters(XmlElement* xml);
	void loadVisualizerParameters(XmlElement* xml);
	void visualizationMenu();
private:
	PeriStimulusTimeHistogramCanvas *periStimulusTimeHistogramCanvas;
    Font font;
	ScopedPointer<ComboBox> hardwareTrialAlignment;
	ScopedPointer<UtilityButton> visibleConditions, saveOptions, clearDisplay,visualizationOptions;
	ScopedPointer<Label> hardwareTrigger;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PeriStimulusTimeHistogramEditor);

};


class GenericPlot : public Component
{
public:
	GenericPlot(String name,	PeriStimulusTimeHistogramDisplay* dsp, int plotID_, xyPlotTypes plotType, 
					TrialCircularBuffer *tcb_, int electrodeID_, int subID_, int row_, int col_, bool _rasterMode);
	void resized();
	void paint(Graphics &g);
	int getRow() {return row;}
	int getCol() {return col;}
	int getPlotID() {return plotID;}
	bool isFullScreen() {return fullScreenMode;}
	void toggleFullScreen(bool state) {fullScreenMode = state;}
	void setSmoothState(bool state);
	void setAutoRescale(bool state);
	void buildSmoothKernel(float gaussianStandardDeviationMS);
	
	void handleEventFromMatlabLikePlot(String event);
private:
	void paintSpikeRaster(Graphics &g);
	void paintSpikes(Graphics &g);
	void paintLFPraster(Graphics &g);
	void paintLFP(Graphics &g);


	ScopedPointer<MatlabLikePlot> mlp;
	PeriStimulusTimeHistogramDisplay* display;
	TrialCircularBuffer *tcb;

	int plotID;
	xyPlotTypes plotType; 
	int electrodeID;
	int subID;
	int row, col;
	bool rasterMode;
	bool fullScreenMode;
	bool smoothPlot;
	bool autoRescale;
	float guassianStandardDeviationMS;
	String plotName;
	std::vector<float> smoothKernel; 
};




#endif  // PSTHEDITOR
