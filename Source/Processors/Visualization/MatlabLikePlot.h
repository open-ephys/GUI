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

#ifndef __MATLAB_LIKE_PLOT_H
#define __MATLAB_LIKE_PLOT_H

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "Visualizer.h"
#include "../Editors/GenericEditor.h"

#include <list>
#include <vector>
#include <queue>

#ifndef MAX
#define MAX(x,y)((x)>(y))?(x):(y)
#endif 

#ifndef MIN
#define MIN(x,y)((x)<(y))?(x):(y)
#endif 
/* A plotting class that you can derive from. Handles all basic pan-zoom, tick marks, tick labels
You only need to implement the drawing of the actual curves in the given range*/
/************************/
class XYline 
{
public:
	//XYline(std::vector<float> x_, std::vector<float> y_, float gain, juce::Colour color_);
	XYline(float x0, float dx, std::vector<float> y_, float gain, juce::Colour color_);

	// for pure vertical lines
	XYline(float x0_, float ymin, float ymax, juce::Colour color_) ;

	void draw(Graphics &g, float xmin, float xmax, float ymin, float ymax, int width, int height, bool showBounds);
	void getYRange(double &lowestValue, double &highestValue);
	void removeMean();
	void smooth(std::vector<float> kernel);
private:
	float interp(float x_sample, bool &inrange);
	float interp_bilinear(float x_sample, bool &inrange);

	float interp_cubic(float x_sample, bool &inrange);
	bool sortedX, fixedDx,  verticalLine;
	float gain,dx, x0,xn,mean;
	int numpts;
	std::vector<float> x;
	std::vector<float> y;
	juce::Colour color;
};

enum DrawComponentMode {ZOOM = 1, PAN = 2, VERTICAL_SHIFT = 3, THRES_UPDATE = 4};
struct range
{
	float xmin,xmax,ymin,ymax;
};
class MatlabLikePlot;

class DrawComponent : public Component
{
public:
	DrawComponent(MatlabLikePlot *mlp);
    ~DrawComponent() {}
	void setMode(DrawComponentMode m);
	void setHorizonal0Visible(bool state);
	void setVertical0Visible(bool state);
	void setTickMarks(std::vector<float> xtick, std::vector<float> ytick);
	void plotxy(XYline l);
	void setRange(float xmin, float xmax, float ymin, float ymax);
	void setYRange(double lowestValue, double highestValue);
	void clearplot();
	void setAutoRescale(bool state);
	void setScaleString(String ampScale, String timeScale);
	void setShowTriggered(bool state);
	void setShowBounds(bool state);
	void setThresholdLineVisibility(bool state);
	void setThresholdLineValue(double Value);
	double getThresholdLineValue();
private:
	void mouseDown(const juce::MouseEvent& event);
	void mouseDrag(const juce::MouseEvent& event);
	void mouseUp(const juce::MouseEvent& event);
	void mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel);
	void mouseMove(const juce::MouseEvent& event);

	void mouseDoubleClick(const juce::MouseEvent& event);
	std::vector<float> computeSamplePositions(int subsample);
	std::list<range> rangeMemory;
	int mouseDragX,mouseDragY,mouseDownX,mouseDownY,mousePrevX,mousePrevY;
	DrawComponentMode mode,savedMode;
	bool panning,zooming, autoRescale ;
	std::vector<float> xtick, ytick;
	double lowestValue, highestValue;
	MatlabLikePlot *mlp;
	String ampScale, timeScale;
	Font font;
	void drawTicks(Graphics &g);
	void paint(Graphics &g);
	std::vector<XYline> lines;
	float xmin,xmax,ymin,ymax;
	bool horiz0,vert0;
	bool thresholdLineVisibility,overThresholdLine;
	double thresholdLineValue;
	bool showBounds;
	bool showTriggered;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrawComponent);
};

class AxesComponent : public Component
{
public:
	AxesComponent(bool horizontal);
    ~AxesComponent() {}
	void setTicks(std::vector<float> ticks_);
	void paint(Graphics &g);
	void setFontHeight(int height);
	String setRange(float minvalue, float maxvalue);
private:
	int numDigits;
	Font font;
	int selectedTimeScale;
	float gain;
	bool horiz;
	float minv,maxv;
	std::vector<float> ticks;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AxesComponent);
};

class MatlabLikePlot : public Component, Button::Listener
{
public:
	MatlabLikePlot(); // use approximate for faster drawing. exact x-axis values will not be accurate...
    ~MatlabLikePlot() {}
	void setControlButtonsVisibile(bool state);
	void setHorizonal0Visible(bool state);
	void setVertical0Visible(bool state);
	void setBorderColor(juce::Colour col);
	void plotxy(XYline l);
	void setAutoRescale(bool state);
	void setRange(float xmin, float xmax, float ymin, float ymax);
	void clearplot();
	void setTitle(String t);
	void paint(Graphics &g);
	void setThresholdLineVisibility(bool state);
	void setThresholdLineValue(double Value);
	double getThresholdLineValue();
	void setTriggered();
	void setShowBounds(bool state);
	
	void addEvent(String e);
	void resized();
	String getLastEvent();
	bool eventsAvail();
	void mouseDoubleClick(const juce::MouseEvent& event);

private:

	void determineTickLocations(float xmin, float xmax,float ymin,float ymax,std::vector<float> &xtick, std::vector<float> &ytick);
	Font font;
	std::list<String> recentEvents;
	std::vector<float> roundlin(float minv, float maxv, int numticks);
	void buttonClicked(Button *btn);
	String title;
	double lowestValue, highestValue;
	bool controlButtonsVisible;
	juce::Colour borderColor;
	int64 triggeredTS;
	ScopedPointer<UtilityButton> zoomButton, panButton, verticalShiftButton, dcRemoveButton, frequencyButton,autoRescaleButton, boundsButton;
	ScopedPointer<AxesComponent> vertAxesComponent,horizAxesComponent;
	ScopedPointer<DrawComponent> drawComponent;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatlabLikePlot);
};
/************************/

#endif