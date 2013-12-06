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

#include "PeriStimulusTimeHistogramEditor.h"
#include "../../UI/EditorViewport.h"
#include "../TrialCircularBuffer.h"
#include <stdio.h>

//FileSearchPathListComponent::paintListBoxItem	
PeriStimulusTimeHistogramEditor::PeriStimulusTimeHistogramEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
	: VisualizerEditor(parentNode, useDefaultParameterEditors) ,periStimulusTimeHistogramCanvas(nullptr)
{
	tabText = "PSTH";
	desiredWidth = 300;

	visibleConditions = new UtilityButton("Conditions",Font("Default", 15, Font::plain));
	visibleConditions->addListener(this);
	visibleConditions->setColour(Label::textColourId, Colours::white);
	visibleConditions->setBounds(10,30,110,20);
	addAndMakeVisible(visibleConditions);


	saveOptions = new UtilityButton("Save Options",Font("Default", 15, Font::plain));
	saveOptions->addListener(this);
	saveOptions->setColour(Label::textColourId, Colours::white);
	saveOptions->setBounds(160,30,110,20);
	addAndMakeVisible(saveOptions);


	clearDisplay = new UtilityButton("Clear all",Font("Default", 15, Font::plain));
	clearDisplay->addListener(this);
	clearDisplay->setColour(Label::textColourId, Colours::white);
	clearDisplay->setBounds(160,60,110,20);
	addAndMakeVisible(clearDisplay);



	autoRescale = new ToggleButton("Auto Rescale");//, Font("Small Text", 13, Font::plain));
	autoRescale->setBounds(10, 105, 110, 18);
	autoRescale->addListener(this);
	autoRescale->setToggleState(true,false);
	autoRescale->setClickingTogglesState(true);
	addAndMakeVisible(autoRescale);

	smoothPSTH = new ToggleButton("Smooth PSTH");//, Font("Small Text", 13, Font::plain));
	smoothPSTH->setBounds(135, 105, 130, 18);
	smoothPSTH->addListener(this);
	smoothPSTH->setToggleState(true,false);
	smoothPSTH->setClickingTogglesState(true);
	addAndMakeVisible(smoothPSTH);



	matchRangeButton = new ToggleButton("Match Range");//, Font("Small Text", 13, Font::plain));
	matchRangeButton->setBounds(135, 80, 130, 18);
	matchRangeButton->addListener(this);
	matchRangeButton->setToggleState(false,false);
	matchRangeButton->setClickingTogglesState(true);
	addAndMakeVisible(matchRangeButton);


	lfp = new ToggleButton("LFP");//, Font("Small Text", 13, Font::plain));
	lfp->setBounds(10, 55, 60, 18);
	lfp->addListener(this);
	lfp->setToggleState(true,false);
	lfp->setClickingTogglesState(true);
	addAndMakeVisible(lfp);

	compactView = new ToggleButton("CompactView");//, Font("Small Text", 13, Font::plain));
	compactView->setBounds(10, 80, 120, 18);
	compactView->addListener(this);
	compactView->setToggleState(false,false);
	compactView->setClickingTogglesState(true);
	addAndMakeVisible(compactView);
	

	spikes = new ToggleButton("Units");//, Font("Small Text", 13, Font::plain));
	spikes->setBounds(70, 55, 60, 18);
	spikes->addListener(this);
	spikes->setToggleState(true,false);
	spikes->setClickingTogglesState(true);
	addAndMakeVisible(spikes);



	smoothMS = new Label("Smooth MS", "10");
	smoothMS->setBounds(245,105,40,18);
	smoothMS->setFont(Font("Default", 15, Font::plain));
	smoothMS->setColour(Label::textColourId, Colours::white);
	smoothMS->setColour(Label::backgroundColourId, Colours::grey);
	smoothMS->setEditable(true);
	smoothMS->addListener(this);
	addAndMakeVisible(smoothMS);




}


void PeriStimulusTimeHistogramEditor::labelTextChanged(Label* label)
{
	Value val = label->getTextValue();
	double requestedValue = MAX(1, double(val.getValue())); // minimum 1 ms smoothing
	if (periStimulusTimeHistogramCanvas == nullptr)
		return;

	if (label == smoothMS)
	{
		periStimulusTimeHistogramCanvas->setSmoothing(	requestedValue);
	}


}

void PeriStimulusTimeHistogramEditor::buttonEvent(Button* button)
{
	VisualizerEditor::buttonEvent(button);
	if (periStimulusTimeHistogramCanvas == nullptr)
		return;

	PeriStimulusTimeHistogramNode* processor = (PeriStimulusTimeHistogramNode*) getProcessor();

	if (button == clearDisplay)
	{
		if (processor->trialCircularBuffer != nullptr) {
			processor->trialCircularBuffer->clearAll();
			repaint();
		}
	} else if (button == compactView)
	{
		periStimulusTimeHistogramCanvas->setCompactView(compactView->getToggleState());
	}
	if (button == lfp)
	{
		periStimulusTimeHistogramCanvas->setLFPvisibility(lfp->getToggleState());
	} else if (button == matchRangeButton)
	{
		periStimulusTimeHistogramCanvas->setMatchRange(matchRangeButton->getToggleState());
	} else if (button == spikes)
	{
		periStimulusTimeHistogramCanvas->setSpikesVisibility(spikes->getToggleState());
	} else if (button == smoothPSTH)
	{
		periStimulusTimeHistogramCanvas->setSmoothPSTH(smoothPSTH->getToggleState());
	} else if (button == autoRescale)
	{
		periStimulusTimeHistogramCanvas->setAutoRescale(autoRescale->getToggleState());
	}  else if (button == saveOptions)
	{

			PopupMenu m;
			
			
			m.addItem(1,"TTL",true, processor->saveTTLs);
			m.addItem(2,"Network Events",true, processor->saveNetworkEvents);
			m.addItem(3,"Sorted Spikes: TS only ",true, processor->spikeSavingMode == 1);
			m.addItem(4,"Sorted Spikes: TS+waveform",true, processor->spikeSavingMode == 2);
			m.addItem(5,"All Spikes: TS+waveform",true, processor->spikeSavingMode == 3);
			
			const int result = m.show();

			if (result == 1)
			{
				processor->saveTTLs = !processor->saveTTLs;
			} else if (result == 2)
			{
				processor->saveNetworkEvents = !processor->saveNetworkEvents;
			} else if (result == 3)
			{
				if (processor->spikeSavingMode == 1)
					processor->spikeSavingMode = 0;
				else
					processor->spikeSavingMode = 1;
			} else if (result == 4)
			{
				if (processor->spikeSavingMode == 2)
					processor->spikeSavingMode = 0;
				else
					processor->spikeSavingMode = 2;
			} else if (result == 5)
			{
				if (processor->spikeSavingMode == 3)
					processor->spikeSavingMode = 0;
				else
					processor->spikeSavingMode = 3;
			}
			
		} else if (button == visibleConditions)
	{

	
		if ( processor->trialCircularBuffer != nullptr)
		{
			processor->trialCircularBuffer ->lockConditions();
			PopupMenu m;

			for (int i = 0; i < processor->trialCircularBuffer->conditions.size(); i++)
			{          
				{

					String name = processor->trialCircularBuffer->conditions[i].name;
					m.addItem(i+1, name,true,processor->trialCircularBuffer->conditions[i].visible);
				}
			}

			const int result = m.show();

			if (result > 0)
			{
				// update the visibility for all channels and units.
				PeriStimulusTimeHistogramNode* processor = (PeriStimulusTimeHistogramNode*) getProcessor();
				processor->toggleConditionVisibility(result-1);

			}
			processor->trialCircularBuffer ->unlockConditions();
		}

	}



}


Visualizer* PeriStimulusTimeHistogramEditor::createNewCanvas()
{
	PeriStimulusTimeHistogramNode* processor = (PeriStimulusTimeHistogramNode*) getProcessor();
	periStimulusTimeHistogramCanvas = new PeriStimulusTimeHistogramCanvas(processor);
	return periStimulusTimeHistogramCanvas;
}

void PeriStimulusTimeHistogramEditor::updateCanvas()
{
	if (periStimulusTimeHistogramCanvas != nullptr) {
		periStimulusTimeHistogramCanvas->updateNeeded = true;
		periStimulusTimeHistogramCanvas->repaint();
	}
}

PeriStimulusTimeHistogramEditor::~PeriStimulusTimeHistogramEditor()
{


}







/********************************/
#ifndef MAX
#define MAX(x,y)((x)>(y))?(x):(y)
#endif 

#ifndef MIN
#define MIN(x,y)((x)<(y))?(x):(y)
#endif 


PeriStimulusTimeHistogramCanvas::PeriStimulusTimeHistogramCanvas(PeriStimulusTimeHistogramNode* n) :
	processor(n)
{

	inFocusedMode = false;
	showLFP = true;
	showSpikes = true;
	smoothPlots = true;
	autoRescale = true;
	compactView = false;
	gaussianStandardDeviationMS = 10;
	viewport = new Viewport();
	psthDisplay = new PeriStimulusTimeHistogramDisplay(n, viewport, this);
	viewport->setViewedComponent(psthDisplay, false);
	viewport->setScrollBarsShown(true, true);

	addAndMakeVisible(viewport);

	update();

}


PeriStimulusTimeHistogramCanvas::~PeriStimulusTimeHistogramCanvas()
{

}

void PeriStimulusTimeHistogramCanvas::beginAnimation()
{
	startCallbacks();
}

void PeriStimulusTimeHistogramCanvas::buttonClicked(Button* button)
{

}

void PeriStimulusTimeHistogramCanvas::endAnimation()
{
	std::cout << "SpikeDisplayCanvas ending animation." << std::endl;

	stopCallbacks();
}


void PeriStimulusTimeHistogramCanvas::setLFPvisibility(bool visible)
{
	showLFP = visible;
	update();
}

void PeriStimulusTimeHistogramCanvas::setSpikesVisibility(bool visible)
{
	showSpikes = visible;
	update();
}

void PeriStimulusTimeHistogramCanvas::setSmoothing(float _gaussianStandardDeviationMS)
{
	gaussianStandardDeviationMS=_gaussianStandardDeviationMS;
	for (int k=0;k<	psthDisplay->psthPlots.size();k++)
	{
		psthDisplay->psthPlots[k]->buildSmoothKernel(gaussianStandardDeviationMS);
		psthDisplay->psthPlots[k]->repaint();
	}

}

void PeriStimulusTimeHistogramCanvas::setSmoothPSTH(bool smooth)
{
	smoothPlots = smooth;
	for (int k=0;k<	psthDisplay->psthPlots.size();k++)
	{
		psthDisplay->psthPlots[k]->setSmoothState(smoothPlots);
		psthDisplay->psthPlots[k]->repaint();
	}

}

void PeriStimulusTimeHistogramCanvas::setCompactView(bool compact)
{
	compactView = compact;
	update();
}

void PeriStimulusTimeHistogramCanvas::setMatchRange(bool on)
{
	matchRange = on;
	update();
}

void PeriStimulusTimeHistogramCanvas::setAutoRescale(bool state)
{
	autoRescale = state;
	for (int k=0;k<	psthDisplay->psthPlots.size();k++)
	{
		psthDisplay->psthPlots[k]->setAutoRescale(autoRescale);
		psthDisplay->psthPlots[k]->repaint();
	}
}


void PeriStimulusTimeHistogramCanvas::refreshState()
{
	resized();
}


void PeriStimulusTimeHistogramCanvas::update()
{
	//std::cout << "Updating SpikeDisplayCanvas" << std::endl;
	// clear all XY plots and create new ones...
	// delete all existing plots.
	// lock psth

	heightPerElectrodePix = 200;
	widthPerUnit = 200;

	int maxUnitsPerRow = getWidth() / widthPerUnit;
	updateNeeded = false;
	for (int k=0; k < psthDisplay->psthPlots.size();k++)
	{
		delete psthDisplay->psthPlots[k];
	}
	psthDisplay->psthPlots.clear();
	if (processor->trialCircularBuffer == nullptr)
		return;

	processor->trialCircularBuffer->lockPSTH();
	numElectrodes = processor->trialCircularBuffer->electrodesPSTH.size();
	int maxUnitsPerElectrode = 0;
	int row = 0;
	int plotCounter = 0;
	numCols = maxUnitsPerRow;
	numRows = 0;
	int plotID = 0;
	for (int e=0;e<numElectrodes;e++) 
	{
		int offset = 0;
		bool plottedSomething = false;
		if (showLFP) {
			offset = processor->trialCircularBuffer->electrodesPSTH[e].channels.size();
			for (int u=0;u<processor->trialCircularBuffer->electrodesPSTH[e].channels.size();u++)
			{
				XYPlot *newplot;
				if (compactView)
				{
					newplot = new XYPlot(psthDisplay,++plotID,false,processor->trialCircularBuffer,
					processor->trialCircularBuffer->electrodesPSTH[e].electrodeID,
					processor->trialCircularBuffer->electrodesPSTH[e].channels[u],
					plotCounter,row);

					plotCounter++;
					if (plotCounter >= maxUnitsPerRow )
					{
						plotCounter = 0;
						row++;
					}
				} else 
				{
					newplot = new XYPlot(psthDisplay,++plotID,false,processor->trialCircularBuffer,
					processor->trialCircularBuffer->electrodesPSTH[e].electrodeID,
					processor->trialCircularBuffer->electrodesPSTH[e].channels[u],
					u,row);

				}
				newplot->setSmoothState(smoothPlots);
				newplot->setAutoRescale(autoRescale);
				newplot->buildSmoothKernel(gaussianStandardDeviationMS);
				psthDisplay->psthPlots.push_back(newplot);
				addAndMakeVisible(newplot);
				plottedSomething = true;
			}

		}

		if (showSpikes)
		{
			int numUnits = processor->trialCircularBuffer->electrodesPSTH[e].unitsPSTHs.size();
			maxUnitsPerElectrode = MAX(maxUnitsPerElectrode,numUnits );
			if (numUnits > 0) 
			{
				for (int u=0;u<numUnits;u++)
				{
					XYPlot *newplot;
					if (compactView)
					{
					    newplot = new XYPlot(psthDisplay,++plotID,true,processor->trialCircularBuffer,
						processor->trialCircularBuffer->electrodesPSTH[e].electrodeID,
						processor->trialCircularBuffer->electrodesPSTH[e].unitsPSTHs[u].unitID,
						plotCounter,row);
						plotCounter++;
						if (plotCounter >= maxUnitsPerRow )
						{
							plotCounter = 0;
							row++;
						}
					} else
					{
					newplot = new XYPlot(psthDisplay,++plotID,true,processor->trialCircularBuffer,
						processor->trialCircularBuffer->electrodesPSTH[e].electrodeID,
						processor->trialCircularBuffer->electrodesPSTH[e].unitsPSTHs[u].unitID,
						offset+u,row);
					}
					newplot->setSmoothState(smoothPlots);
					newplot->setAutoRescale(autoRescale);
					newplot->buildSmoothKernel(gaussianStandardDeviationMS);

					psthDisplay->psthPlots.push_back(newplot);
					addAndMakeVisible(newplot);
				}
				plottedSomething = true;
			}
		}
		if (!compactView &&  plottedSomething) 
			row++;			
	}
	numRows = row;
	if (maxUnitsPerElectrode == 0 && !showLFP) {
		// nothing to be drawn...
		processor->trialCircularBuffer->unlockPSTH();
		return;		
	}
	resized();


	psthDisplay->resized();
	psthDisplay->repaint();
	processor->trialCircularBuffer->unlockPSTH();
}


void PeriStimulusTimeHistogramCanvas::resized()
{
	screenWidth = getWidth();
	screenHeight = getHeight();
	viewport->setBounds(0,0,getWidth(),getHeight()-20);
	int scrollBarThickness = viewport->getScrollBarThickness();


	int totalHeight = numRows * heightPerElectrodePix;
	int totalWidth = numCols * widthPerUnit;
	psthDisplay->setBounds(0,0,totalWidth-scrollBarThickness, totalHeight);


}

void PeriStimulusTimeHistogramCanvas::paint(Graphics& g)
{
	if (updateNeeded)
		update();
	g.fillAll(Colours::grey);

}

void PeriStimulusTimeHistogramCanvas::refresh()
{
	repaint();
	psthDisplay->refresh();
}

/***********************************************/

PeriStimulusTimeHistogramDisplay::PeriStimulusTimeHistogramDisplay(PeriStimulusTimeHistogramNode* n, Viewport *p, PeriStimulusTimeHistogramCanvas*c) :
	processor(n), viewport(p), canvas(c)
{

	font = Font("Default", 15, Font::plain);
}

PeriStimulusTimeHistogramDisplay::~PeriStimulusTimeHistogramDisplay()
{
	for (int k=0;k<psthPlots.size();k++)
	{
		delete(psthPlots[k]);
	}
	psthPlots.clear();
}

void PeriStimulusTimeHistogramDisplay::refresh()
{
	for (int k=0;k<psthPlots.size();k++)
	{
		psthPlots[k]->repaint();
	}

}


void PeriStimulusTimeHistogramDisplay::paint(Graphics &g)
{
	g.setColour(Colours::white);
	g.drawRect(0,0,getWidth(),getHeight());

	font = Font("Default", 15, Font::plain);

	g.setFont(font);

	g.drawText("Test",10,0,200,20,Justification::left,false);
}  

void PeriStimulusTimeHistogramDisplay::resized()
{
	// draw n by m grid
	for (int k=0;k<psthPlots.size();k++)
	{
		
		psthPlots[k]->setBounds(psthPlots[k]->row * canvas->widthPerUnit,
			psthPlots[k]->col * canvas->heightPerElectrodePix,
			canvas->widthPerUnit,
			canvas->heightPerElectrodePix);

	}
}


void PeriStimulusTimeHistogramDisplay::focusOnPlot(int plotID)
{
	int plotIndex = -1;
	for (int i=0;i<psthPlots.size();i++)
	{
		if (psthPlots[i]->getPlotID() == plotID)
		{
			plotIndex = i;
			break;
		}

	}
	if (plotIndex == -1)
		return;
	if (psthPlots[plotIndex]->isFullScreen())
	{

		psthPlots[plotIndex]->toggleFullScreen(false);
		psthPlots[plotIndex]->setBounds(psthPlots[plotIndex]->row * canvas->widthPerUnit,
			psthPlots[plotIndex]->col * canvas->heightPerElectrodePix,
			canvas->widthPerUnit,
			canvas->heightPerElectrodePix);
		// hide all other plots.
		for (int k=0;k<psthPlots.size();k++) 
		{
				psthPlots[k]->setVisible(true);
				psthPlots[k]->repaint();
		}

	} 
	else 
	{
		// hide all other plots.
		for (int k=0;k<psthPlots.size();k++) 
		{
			if (psthPlots[k]->getPlotID() != plotID)
				psthPlots[k]->setVisible(false);
		}
		psthPlots[plotIndex]->toggleFullScreen(true);
		// make sure its rectangular...?
		int newSize = MIN(canvas->screenWidth,canvas->screenHeight);
		psthPlots[plotIndex]->setBounds(0,0,newSize,newSize);
		psthPlots[plotIndex]->repaint();
	}
	
}

/******************************************/
XYPlot::XYPlot(PeriStimulusTimeHistogramDisplay *dsp, int _plotID, bool _spikePlot, TrialCircularBuffer *_tcb, int _electrodeID, int _unitID, int _row, int _col) :
	tcb(_tcb), electrodeID(_electrodeID), unitID(_unitID), row(_row), col(_col),spikePlot(_spikePlot), plotID(_plotID), display(dsp)
{
	font = Font("Default", 15, Font::plain);
	guassianStandardDeviationMS = 5; // default smoothing
	buildSmoothKernel(guassianStandardDeviationMS); 
	smoothPlot = spikePlot; // don't smooth LFPs
	autoRescale = true;
	firstTime = true;
	fullScreenMode = false;
	zooming = false;
}

void XYPlot::mouseUp(const juce::MouseEvent& event)
{
	zooming = false;
}

void XYPlot::mouseDrag(const juce::MouseEvent& event)
{
	mouseDragX = event.x;
	mouseDragY = event.y;

	repaint();
}


void XYPlot::mouseDown(const juce::MouseEvent& event)
{
	
	if (event.mods.isRightButtonDown() && !event.mods.isShiftDown())
	{

		tcb->lockPSTH();
		for (int electrodeIndex=0;electrodeIndex<	tcb->electrodesPSTH.size();electrodeIndex++)
		{
			if (tcb->electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
			{
				if (spikePlot)
				{
					for (int unitIndex = 0; unitIndex < tcb->electrodesPSTH[electrodeIndex].unitsPSTHs.size();unitIndex++)
					{
						if (tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].unitID == unitID)
						{
							tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].clearStatistics();
							break;
						}
					}
				} else 
				{
					for (int Index = 0; Index < tcb->electrodesPSTH[electrodeIndex].channels.size();Index++)
					{
						if (tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[Index].channelID  == unitID)
						{
							tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[Index].clearStatistics();
							break;
						}
					}

				}
			}
		}
		tcb->unlockPSTH();
	} else if (event.mods.isRightButtonDown() && event.mods.isShiftDown())
	{
		// zoom out
	} else if (event.mods.isLeftButtonDown())
	{
		mouseDownX = event.x;
		mouseDownY = event.y;
		mouseDragX = event.x;
		mouseDragY = event.y;

		zooming = true;
	}
}

void XYPlot::resized()
{

}



std::vector<int> XYPlot::histc(std::vector<float> xi, std::vector<float> x)
{
	std::vector<int> aiInd(xi.size());

	int i = 0;
	int j = 0;
	int N = x.size();
	while (i < xi.size()) 
	{
		if (xi[i] < x[j] || xi[i] > x[N-1])
		{
			aiInd[i] = -1;
			i++;
			continue;
		}
		if (j + 1 < N && xi[i] >= x[j] && xi[i] < x[j + 1])
		{
			aiInd[i] = j;
			i++;
			continue;
		}
		j++;
		if (j > N - 1)
			j = N - 1;
	}
	return aiInd;
}

std::vector<float> XYPlot::diff(std::vector<float> x)
{
	std::vector<float> d(x.size()-1);
	for (int k = 0; k < x.size() - 1; k++)
	{
		d[k] = x[k + 1] - x[k];
	}
	return d;
}


void XYPlot::interp1(std::vector<float> x, std::vector<float>y, std::vector<float> xi, std::vector<float> &yi, std::vector<bool> &valid, float &min, float &max)
{
	// linear interpolate
	int N = x.size();
	int M = xi.size();

	valid.resize(xi.size());
	yi.resize(xi.size());

	if (x.size()== 0)
		return;

	std::vector<int> ind = histc(xi, x);
	std::vector<float> h = diff(x);
	min = 1e10;
	max = -1e10;

	for (int i = 0; i < xi.size(); i++)
	{
		if (ind[i] < 0)
		{
			// invalid entry
			valid[i] = false;
			yi[i] = 0;
			continue;
		}
		valid[i] = true;

		double s = (xi[i] - x[ind[i]]) / h[ind[i]];
		yi[i] = y[ind[i]] + s * (y[ind[i] + 1] - y[ind[i]]);
		min = MIN(min,yi[i]);
		max = MAX(max, yi[i]);
	}



}

bool XYPlot::getSmoothState()
{
	return smoothPlot;
}

void XYPlot::setSmoothState(bool enable)
{
	smoothPlot = enable;
}

bool XYPlot::getAutoRescale()
{
	return autoRescale;
}

void XYPlot::setAutoRescale(bool enable)
{
	autoRescale = enable;
}

std::vector<float> XYPlot::smooth(std::vector<float> x)
{
	std::vector<float> smoothx;
	smoothx.resize(x.size());

	int numKernelBins = smoothKernel.size();
	int zeroIndex = (numKernelBins-1)/2;
	int numXbins = x.size();
	for (int k=0;k<numXbins;k++)
	{
		float response = 0;
		for (int j=-zeroIndex;j<zeroIndex;j++)
		{
			if (k+j >=0 && k+j < numXbins)
				response+=x[k+j] * smoothKernel[j+zeroIndex];
		}
		smoothx[k] = response;
	}
	return smoothx;
}

void XYPlot::toggleFullScreen(bool fullScreenOn)
{
	fullScreenMode = fullScreenOn;
}

bool XYPlot::isFullScreen()
{
	return fullScreenMode;
}



void XYPlot::mouseDoubleClick(const juce::MouseEvent& event)
{
	display->focusOnPlot(plotID);
}

void XYPlot::buildSmoothKernel(float _guassianStandardDeviationMS)
{
	guassianStandardDeviationMS = _guassianStandardDeviationMS;
	// assume each bin correponds to one millisecond.
	// build the gaussian kernel
	int numKernelBins = 2*(int)(guassianStandardDeviationMS*3.5)+1; // +- 3.5 standard deviations.
	int zeroIndex = (numKernelBins-1)/2;
	smoothKernel.resize(numKernelBins); 
	float sumZ = 0;
	for (int k=0;k<numKernelBins;k++) 
	{
		float z = float(k-zeroIndex);
		smoothKernel[k] = exp(- (z*z)/(2*guassianStandardDeviationMS*guassianStandardDeviationMS));
		sumZ+=smoothKernel[k];
	}
	// normalize kernel
	for (int k=0;k<numKernelBins;k++) 
	{
		smoothKernel[k] /= sumZ;
	}
}


bool XYPlot::findIndices(int &electrodeIndex, int &entryindex, bool findUnitOrChannel)
{
	if (findUnitOrChannel)
	{
		// find unit
		for (electrodeIndex=0;electrodeIndex<	tcb->electrodesPSTH.size();electrodeIndex++)
		{
			if (tcb->electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
			{
				for (entryindex = 0; entryindex < tcb->electrodesPSTH[electrodeIndex].unitsPSTHs.size();entryindex++)
				{
					if (tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].unitID == unitID)
					{
						return true;
					}
				}

			}
		}
	} else 
	{
		// find channel
		for (electrodeIndex=0;electrodeIndex<	tcb->electrodesPSTH.size();electrodeIndex++)
		{
			if (tcb->electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
			{
				for (entryindex = 0; entryindex < tcb->electrodesPSTH[electrodeIndex].channelsPSTHs.size();entryindex++)
				{
					if (tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].channelID == unitID)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

void XYPlot::paintPlotNameAndRect(Graphics &g)
{

	 w = getWidth();
	 h = getHeight();
	// draw a bounding box of the plot.
	 x0;

	if (w >= 300) {
		font = Font("Default", 15, Font::plain);
		x0 = 60;
	} else if (w >= 250)
	{
		font = Font("Default", 10, Font::plain);
		x0 = 50;
	} else 
	{
		font = Font("Default", 8, Font::plain);
		x0 = 30;
	}

	 y0 = 30;

	 plotWidth  = getWidth()-1.5*x0;
	 plotHeight = getHeight()-2*y0;

	g.setColour(Colours::black);
	g.fillRect(x0,y0, plotWidth,plotHeight);
	g.setColour(Colours::white);
	g.drawRect(x0,y0, plotWidth,plotHeight);

	g.setFont(font);
	String axesName;
	if (spikePlot) 
	{
		axesName = String("Unit ")+String(tcb->electrodesPSTH[electrodeIndex].electrodeID)+":"+ 
			String(tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].unitID);
	} else 
	{
		axesName = String("LFP ")+String(tcb->electrodesPSTH[electrodeIndex].electrodeID)+":"+
			String(tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].channelID);
	}

	g.drawText(axesName,plotWidth/2,10,plotWidth/2,20,Justification::centred,false);


	// keep a fixed amount of pixels for axes labels
	if (spikePlot) 
	{
		g.setColour(juce::Colour(tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].colorRGB[0],
			tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].colorRGB[1],
			tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].colorRGB[2]));
	} else
	{
		g.setColour(Colours::white);
	}

	g.drawRect(0,0,w,h,2);


	g.setColour(Colours::white);
	// plot the x axis
	g.drawLine(x0,h-y0,x0+plotWidth,h-y0, 1);
	// plot the y axis
	g.drawLine(x0,h-y0,x0,h-(y0+plotHeight),1);
}


void XYPlot::computeSamplePositions(float &xmin, float &xmax)
{
	 subsample = 5; // subsample every 5 pixels to speed things up.

	 // reduce range so we don't see the effects of convolving with the kernel outside function values.

	if (autoRescale && smoothPlot && (xmax-xmin > 14*guassianStandardDeviationMS/1e3)) {
			xmin += 7*guassianStandardDeviationMS/1e3;
			xmax -= 7*guassianStandardDeviationMS/1e3;
	}

	// finally, draw the function....
	// first, generate the sample positions
	int numSamplePoints = plotWidth/subsample;
	samplePositions.clear();
	samplePositions.resize(numSamplePoints);

	for (int k=0;k<numSamplePoints;k++)
	{
		samplePositions[k] = (float(k)/(numSamplePoints-1)) * (xmax-xmin) + xmin;
		// which corresponds to pixel location subsample*k
	}
}


void XYPlot::sampleConditionsForLFP(float &minY, float &maxY)
{

	std::vector<float> smooth_res;
	int numConditions = tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs.size();

	interpolatedConditions.clear();
	interpolatedConditions.resize(numConditions);
	interpolatedConditionsValid.resize(numConditions);
	conditionMaxY.clear();
	conditionMinY.clear();
	conditionMaxY.resize(numConditions);
	conditionMinY.resize(numConditions);
	minY = 1e10;
	maxY = -1e10;
	
	for (int cond=0;cond<numConditions;cond++)
	{

		if (tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].numTrials > 0 &&
			tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].visible)
		{
			
			if (smoothPlot) {
				smooth_res = smooth(tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].avgResponse);

				interp1(tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].binTime, 
					smooth_res,
					samplePositions,  interpolatedConditions[cond],  interpolatedConditionsValid[cond],conditionMinY[cond],conditionMaxY[cond]);
			}
			else 
			{
				interp1(tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].binTime, 
					tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].avgResponse,
					samplePositions,  interpolatedConditions[cond],  interpolatedConditionsValid[cond],conditionMinY[cond],conditionMaxY[cond]);
			}
			minY = MIN(minY, conditionMinY[cond]);
			maxY = MAX(maxY, conditionMaxY[cond]);
		}
	}
}


void XYPlot::sampleConditions(float &minY, float &maxY)
{
	std::vector<float> smooth_res;
	int numConditions = tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs.size();

	interpolatedConditions.clear();
	interpolatedConditions.resize(numConditions);
	interpolatedConditionsValid.resize(numConditions);
	conditionMaxY.clear();
	conditionMinY.clear();
	conditionMaxY.resize(numConditions);
	conditionMinY.resize(numConditions);
	minY = 1e10;
	maxY = -1e10;

	for (int cond=0;cond<numConditions;cond++)
	{

		if (tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].numTrials > 0 &&
			tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].visible)
		{
			
			if (smoothPlot) {
				smooth_res = smooth(tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].avgResponse);

				interp1(tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].binTime, 
					smooth_res,
					samplePositions,  interpolatedConditions[cond],  interpolatedConditionsValid[cond],conditionMinY[cond],conditionMaxY[cond]);
			}
			else 
			{
				interp1(tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].binTime, 
					tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].avgResponse,
					samplePositions,  interpolatedConditions[cond],  interpolatedConditionsValid[cond],conditionMinY[cond],conditionMaxY[cond]);
			}
			minY = MIN(minY, conditionMinY[cond]);
			maxY = MAX(maxY, conditionMaxY[cond]);
		}
	}
}

void XYPlot::plotTicks(Graphics &g, float xmin, float xmax, float ymin, float ymax)
{
	// determine tick position
	axesRange[0] = xmin;
	axesRange[1] = ymin;
	axesRange[2] = xmax;
	axesRange[3] = ymax;
	
	rangeX = (axesRange[2]-axesRange[0]);
	rangeY = (axesRange[3]-axesRange[1]);
	int numXTicks = 5;
	int numYTicks = 5;

	// determine tick positions.
	std::vector<float> tickX,tickY;
	tickX.resize(numXTicks);
	tickY.resize(numYTicks);
	for (int k=0;k<numXTicks;k++)
	{
		tickX[k] = float(k)/(numXTicks-1) * rangeX +axesRange[0];
	}
	for (int k=0;k<numYTicks;k++)
	{
		tickY[k] = float(k)/(numYTicks-1) * rangeY +axesRange[1];
	}

	int tickHeight = 6;
	float tickThickness = 2;


	float ticklabelWidth = float(plotWidth)/numXTicks;
	int tickLabelHeight = 20;
	// plot the tick marks and corresponding text.
	for (int k=0;k<numXTicks;k++)
	{
		// convert to screen coordinates.
		float tickloc = x0+(tickX[k]- axesRange[0]) / rangeX * plotWidth;
		g.drawLine(tickloc,h-y0,tickloc,h-(y0+tickHeight),tickThickness);
		String tickLabel(tickX[k],1);;

		if (k > 0)
			g.drawText(tickLabel,tickloc-ticklabelWidth/2,h-(y0),ticklabelWidth,tickLabelHeight,Justification::centred,false);
		else
			g.drawText(tickLabel,tickloc,h-(y0),ticklabelWidth,tickLabelHeight,Justification::left,false);

	}
	for (int k=1;k<numYTicks;k++)
	{
		// convert to screen coordinates.
		float tickloc = y0+(tickY[k]- axesRange[1]) / rangeY * plotHeight;
		g.drawLine(x0,h-tickloc,x0+tickHeight,h-tickloc, tickThickness);

		String tickLabel(tickY[k],1);;
		g.drawText(tickLabel,x0-ticklabelWidth-3,h-tickloc-tickLabelHeight/2,ticklabelWidth,tickLabelHeight,Justification::right,false);

	}
}

int XYPlot::getPlotID()
{
	return plotID;
}


void XYPlot::paintSpikes(Graphics &g)
{
	// 1. Find the corresponding data.
	// 2. smooth if needed.
	// 3. find the drawing range.
	// 4. interpolate
	// 5. draw.
	bool newdata = false;
	tcb->lockPSTH();
	if (!findIndices(electrodeIndex, entryindex,true))
	{
		tcb->unlockPSTH();
		return;
	}
	paintPlotNameAndRect(g);

	if (autoRescale)
	{
		xmin=0;
		xmax=0;
		ymax=-1e10;
		ymin = 1e10;
		tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].getRange(xmin, xmax, ymin,ymax);
	}

	// use only xmin and xmax to reduce the sampling interval. xmax will be determined by the longest observed trial.
	if (fabs(xmax-xmin)<1e-4   || fabs(ymax-ymin) < 1e-4)
	{
		tcb->unlockPSTH();
		return;
	}

	computeSamplePositions(xmin, xmax);

	float minConditionValue, maxConditionValue;
	sampleConditions(minConditionValue,maxConditionValue);
	if (autoRescale)
	{
		ymin = 0 ;
		ymax = maxConditionValue ;
	}

	if (fabs(ymax-ymin) < 1e-4)
	{
		tcb->unlockPSTH();
		return;
	}

	plotTicks(g,xmin, xmax, ymin, ymax);

//	newdata = tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].isNewDataAvailable();

	int numSamplePoints = plotWidth/subsample;
	for (int cond=0;cond<tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs.size();cond++)
	{
		if (interpolatedConditions[cond].size() == 0)
			continue;

			g.setColour(juce::Colour(tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].colorRGB[0],
				tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].colorRGB[1],
				tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].colorRGB[2]));

			for (int k=0;k<numSamplePoints-1;k++) 
			{
				// remap f_xi to pixels!
				float fx_pix = MIN(plotHeight, MAX(0,(interpolatedConditions[cond][k]-axesRange[1])/rangeY * plotHeight));
				float fxp1_pix = MIN(plotHeight,MAX(0,(interpolatedConditions[cond][k+1]-axesRange[1])/rangeY * plotHeight));
				if (interpolatedConditionsValid[cond][k] && interpolatedConditionsValid[cond][k+1])
					g.drawLine(x0+subsample*k, h-fx_pix-y0, x0+subsample*(k+1), h-fxp1_pix-y0);
			}

	}
	tcb->unlockPSTH();
	repaint();
}


void XYPlot::paintLFP(Graphics &g)
{
	// 1. Find the corresponding data.
	// 2. smooth if needed.
	// 3. find the drawing range.
	// 4. interpolate
	// 5. draw.
	bool newdata = false;
	tcb->lockPSTH();
	if (!findIndices(electrodeIndex, entryindex,false))
	{
		tcb->unlockPSTH();
		return;
	}
	paintPlotNameAndRect(g);

	if (autoRescale)
	{
		xmin=0;
		xmax=0;
		ymax=-1e10;
		ymin = 1e10;
		
		tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].getRange(xmin, xmax, ymin,ymax);
		// use only xmin and xmax to reduce the sampling interval. xmax will be determined by the longest observed trial.
	}
	if (fabs(xmax-xmin)<1e-4   || fabs(ymax-ymin) < 1e-4)
		{
			tcb->unlockPSTH();
			return;
		}

	computeSamplePositions(xmin, xmax);

	float minConditionValue, maxConditionValue;
	sampleConditionsForLFP(minConditionValue,maxConditionValue);
	if (autoRescale)
	{
		ymin =minConditionValue;
		ymax = maxConditionValue ;
	}

	if (fabs(ymax-ymin) < 1e-4)
	{
		tcb->unlockPSTH();
		return;
	}


	plotTicks(g,xmin, xmax, ymin, ymax);

//	newdata = tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].isNewDataAvailable();

	int numSamplePoints = plotWidth/subsample;
	for (int cond=0;cond<tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs.size();cond++)
	{
		if (interpolatedConditions[cond].size() == 0)
			continue;

			g.setColour(juce::Colour(tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].colorRGB[0],
				tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].colorRGB[1],
				tcb->electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].colorRGB[2]));

			for (int k=0;k<numSamplePoints-1;k++) 
			{
				// remap f_xi to pixels!
				float fx_pix = MIN(plotHeight, MAX(0,(interpolatedConditions[cond][k]-axesRange[1])/rangeY * plotHeight));
				float fxp1_pix = MIN(plotHeight,MAX(0,(interpolatedConditions[cond][k+1]-axesRange[1])/rangeY * plotHeight));
				if (interpolatedConditionsValid[cond][k] && interpolatedConditionsValid[cond][k+1])
					g.drawLine(x0+subsample*k, h-fx_pix-y0, x0+subsample*(k+1), h-fxp1_pix-y0);
			}

	}
	tcb->unlockPSTH();


	repaint();
}


void XYPlot::paint(Graphics &g)
{
	if (spikePlot)
		paintSpikes(g);
	else
		paintLFP(g);

	if (zooming)
	{
		g.setColour(juce::Colours::white);
		int width = abs(mouseDownX-mouseDragX);
		int height= abs(mouseDownY-mouseDragY);
		if (width > 0 & height > 0)
			g.drawRect(MIN(mouseDownX,mouseDragX),MIN(mouseDownY,mouseDragY),width,height,2);
	}

}
