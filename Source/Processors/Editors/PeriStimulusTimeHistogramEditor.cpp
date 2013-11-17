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

	  visibleConditions = new ComboBox("Conditions");

    visibleConditions->setEditableText(false);
    visibleConditions->setJustificationType(Justification::centredLeft);
    visibleConditions->addListener(this);
    visibleConditions->setBounds(65,40,110,20);
    addAndMakeVisible(visibleConditions);

   
	//ConditionsList *model = new ConditionList();
/*	ConditionsList *model = new ConditionsList;
	    model->setBounds(65,40,110,60);
	addAndMakeVisible(model);*/

   
}

void PeriStimulusTimeHistogramEditor::updateCondition(std::vector<Condition> conditions)
{
	visibleConditions->clear();
	
    for (int i = 0; i < conditions.size(); i++)
    {
        visibleConditions->addItem(conditions[i].name, i+1);
    }
}
void PeriStimulusTimeHistogramEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged) 
{
	
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
	}
}

PeriStimulusTimeHistogramEditor::~PeriStimulusTimeHistogramEditor()
{

  
}








/*
class ConditionsList : public ListBoxModel, public Component
{
public:
	ConditionsList();
    int getNumRows ();
	void paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected);
	void paint(Graphics &g);
	void changed();

	ListBox listbox;
};

void ConditionsList::paint(Graphics &g)
{
	listbox.paint(g);
}

juce::ComboBox
void ConditionsList::changed()
{
    listbox.repaint();
    
}

int ConditionsList::getNumRows()
{
	return 5;
}
//	FileSearchPathListComponent::paintListBoxItem	

void ConditionsList::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
    //if (rowIsSelected)
    //    g.fillAll (TextEditor::highlightColourId);

	g.setColour (juce::Colour(128,0,0));
    Font f (height * 0.7f);
    f.setHorizontalScale (0.9f);
    g.setFont (f);

	g.drawText ("Hello" + String(rowNumber),
                4, 0, width - 6, height,
                Justification::centredLeft, true);


}

ConditionsList::ConditionsList()
{
	listbox.setModel(this);
	
    listbox.setColour (ListBox::backgroundColourId, Colours::black.withAlpha (0.02f));
    listbox.setColour (ListBox::outlineColourId, Colours::black.withAlpha (0.1f));
    listbox.setOutlineThickness (1);

    listbox.setBounds(65,40,110,60);
    addAndMakeVisible (&listbox);

    //electrodeTypes->setSelectedId(1);
	//visibleConditions->set
    
}
*/


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

void PeriStimulusTimeHistogramCanvas::refreshState()
{
	resized();
}

void PeriStimulusTimeHistogramCanvas::update()
{
	//std::cout << "Updating SpikeDisplayCanvas" << std::endl;
// clear all XY plots and create new ones...
	// delete all existing plots.
	updateNeeded = false;
	psthDisplay->psthPlots.clear();
	if (processor->trialCircularBuffer == nullptr)
		return;

	numElectrodes = processor->trialCircularBuffer->electrodesPSTH.size();
	int maxUnitsPerElectrode = 0;
	int row = 0;
	
	for (int e=0;e<numElectrodes;e++) 
	{
		int numUnits = processor->trialCircularBuffer->electrodesPSTH[e].unitsPSTHs.size();
		maxUnitsPerElectrode = MAX(maxUnitsPerElectrode,numUnits );
		if (numUnits > 0) 
		{
			for (int u=0;u<numUnits;u++)
			{
				

				XYPlot *newplot = new XYPlot(processor->trialCircularBuffer,
					processor->trialCircularBuffer->electrodesPSTH[e].electrodeID,
					processor->trialCircularBuffer->electrodesPSTH[e].unitsPSTHs[u].unitID,
					u,row);
				psthDisplay->psthPlots.push_back(newplot);
				addAndMakeVisible(newplot);
			}
			row++;			
		}
	}
	if (maxUnitsPerElectrode == 0) {
		// nothing to be drawn...
		return;		
	}
    resized();

	heightPerElectrodePix = 300;
	widthPerUnit = 300;

	psthDisplay->resized();
	psthDisplay->repaint();
}


void PeriStimulusTimeHistogramCanvas::resized()
{
    viewport->setBounds(0,0,getWidth(),getHeight()-20);
	int scrollBarThickness = viewport->getScrollBarThickness();


	int totalHeight = numElectrodes * heightPerElectrodePix;
	int totalWidth = maxUnitsPerElectrode * widthPerUnit;
    psthDisplay->setBounds(0,0,totalWidth-scrollBarThickness, totalHeight);

	
}

void PeriStimulusTimeHistogramCanvas::paint(Graphics& g)
{
	if (updateNeeded)
		update();
	g.fillAll(Colours::darkgrey);

}

void PeriStimulusTimeHistogramCanvas::refresh()
{
	// this is called every 10 hz?
	repaint();
	psthDisplay->refresh();
}

/***********************************************/

PeriStimulusTimeHistogramDisplay::PeriStimulusTimeHistogramDisplay(PeriStimulusTimeHistogramNode* n, Viewport *p, PeriStimulusTimeHistogramCanvas*c) :
		processor(n), viewport(p), canvas(c)
{

	font = Font("Default", 15, Font::plain);
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
	int h = getHeight();
	int w = getWidth();

	
	// draw n by m grid
	for (int k=0;k<psthPlots.size();k++)
	{
		psthPlots[k]->setBounds(psthPlots[k]->row * canvas->widthPerUnit,
							    psthPlots[k]->col * canvas->heightPerElectrodePix,
								canvas->widthPerUnit,
								canvas->heightPerElectrodePix);

	}
}


/******************************************/
XYPlot::XYPlot(TrialCircularBuffer *_tcb, int _electrodeID, int _unitID, int _row, int _col) :
	tcb(_tcb), electrodeID(_electrodeID), unitID(_unitID), row(_row), col(_col)
{
	font = Font("Default", 15, Font::plain);
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


void XYPlot::interp1(std::vector<float> x, std::vector<float>y, std::vector<float> xi, std::vector<float> &yi, std::vector<bool> &valid)
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
	}

}


void XYPlot::paint(Graphics &g)
{
	int w = getWidth();
	int h = getHeight();
	// draw a bounding box of the plot.
	int x0;

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

	// keep a fixed amount of pixels for axes labels
	g.setColour(Colours::whitesmoke);
	g.drawRect(0,0,w,h);

	int y0 = 30;

	int plotWidth  = getWidth()-1.5*x0;
	int plotHeight = getHeight()-2*y0;
	g.setColour(Colours::black);
	g.fillRect(x0,y0, plotWidth,plotHeight);
		
	g.setColour(Colours::white);
	g.drawRect(x0,y0, plotWidth,plotHeight);

	float xmin=0, xmax=0, ymax=0;
	bool found = false;
	int electrodeIndex;
	int unitIndex;
	tcb->lockPSTH();
	for (electrodeIndex=0;electrodeIndex<	tcb->electrodesPSTH.size();electrodeIndex++)
	{
		if (tcb->electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (unitIndex = 0; unitIndex < tcb->electrodesPSTH[electrodeIndex].unitsPSTHs.size();unitIndex++)
			{
				if (tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].unitID == unitID)
				{
					found = true;
					tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].getRange(xmin, xmax, ymax);
					break;
				}
			}
		}
		if (found)
			break;
	}
	tcb->unlockPSTH();
	if (!found || xmin == xmax || ymax == 0)
		return; // nothing to draw.

	// determine tick position
	


	float axesRange[4] = {xmin,0, xmax, ymax}; // xmin,ymin, xmax, ymax
	float rangeX = (axesRange[2]-axesRange[0]);
	float rangeY = (axesRange[3]-axesRange[1]);
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
	g.setColour(Colours::white);
	// plot the x axis
	g.drawLine(x0,h-y0,x0+plotWidth,h-y0, 1);
	// plot the y axis
	g.drawLine(x0,h-y0,x0,h-(y0+plotHeight),1);
	

		g.setFont(font);
		String axesName = String("Electrode ")+String(tcb->electrodesPSTH[electrodeIndex].electrodeID)+":"+
			String(tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].unitID);
	g.drawText(axesName,plotWidth/2,10,plotWidth/2,20,Justification::centred,false);
	
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
	
	// finally, draw the function....
	// first, generate the sample positions
	int subsample = 5; // subsample every 5 pixels
	int numSamplePoints = plotWidth/subsample;
	std::vector<float> samplePositions;
	samplePositions.resize(numSamplePoints);
	
	for (int k=0;k<numSamplePoints;k++)
	{
		samplePositions[k] = (float(k)/(numSamplePoints-1)) * rangeX + axesRange[0];
		// which corresponds to pixel location subsample*k
	}

	std::vector<float> f_xi;


	tcb->lockPSTH();

	for (int cond=0;cond<tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].conditionPSTHs.size();cond++)
	{

		std::vector<bool> valid;
		interp1(tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].conditionPSTHs[cond].binTime, 
			tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].conditionPSTHs[cond].avgResponse,
			samplePositions,  f_xi,  valid);

		// and finally.... plot!
		g.setColour(juce::Colour(tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].conditionPSTHs[cond].colorRGB[0],
			tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].conditionPSTHs[cond].colorRGB[1],
			tcb->electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].conditionPSTHs[cond].colorRGB[2]));

		for (int k=0;k<numSamplePoints-1;k++) 
		{
			// remap f_xi to pixels!
			float fx_pix = MIN(plotHeight, MAX(0,(f_xi[k]-axesRange[1])/rangeY * plotHeight));
			float fxp1_pix = MIN(plotHeight,MAX(0,(f_xi[k+1]-axesRange[1])/rangeY * plotHeight));
			if (valid[k] && valid[k+1])
				g.drawLine(x0+subsample*k, h-fx_pix-y0, x0+subsample*(k+1), h-fxp1_pix-y0);
		}

	}


	tcb->unlockPSTH();


	repaint();
	
}
