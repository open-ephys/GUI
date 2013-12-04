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


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "AdvancerEditor.h"
#include "../AdvancerNode.h"
#include "SpikeDetectorEditor.h"
#include "../SpikeDetector.h"
#include <stdio.h>


/************/
class AdvancerEditor;





ContainersPlot::ContainersPlot(AdvancerCanvas *cv,AdvancerEditor* ed,AdvancerNode* n) :
		canvas(cv), editor(ed), processor(n)
{
	
	 monkeyImage= ImageCache::getFromMemory(BinaryData::monkey_png, BinaryData::monkey_pngSize);
	 rodentImage = ImageCache::getFromMemory(BinaryData::rodent_png, BinaryData::rodent_pngSize);
	
	 
	 animalPicture = 0;
	 animalButton = new UtilityButton("Switch animal",Font("Default", 15, Font::plain));
	animalButton->addListener(this);
	animalButton->setColour(Label::textColourId, Colours::white);
	animalButton->setBounds(10,10,150,20);
	addAndMakeVisible(animalButton);
}

void ContainersPlot::buttonClicked(juce::Button *button)
{
	if (button == animalButton)
	{
		animalPicture++;
		if (animalPicture >= 2)
			animalPicture = 0;

		repaint();
	}
}

void ContainersPlot::drawPolygon(Graphics &g,Polygon2D *p, float x0, float y0)
{

	g.setColour(juce::Colour(p->color[0],p->color[1],p->color[2]));
	for (int k=0;k<p->points.size()-1;k++)
	{
		float x1 = (x0+p->points[k].x-minX)/(maxX-minX) * scale;
		float x2 = (x0+p->points[k+1].x-minX)/(maxX-minX) * scale;
		float y1 = (y0+p->points[k].y-minY)/(maxY-minY) * scale;
		float y2 = (y0+p->points[k+1].y-minY)/(maxY-minY) * scale;

		g.drawLine(x1,y1,x2,y2,2);
	}
}

void ContainersPlot::resized()
{
		int width = getWidth();
	int height = getHeight();

	if (width > height)
		scale = height;
	else
		scale = width;
}


void ContainersPlot::paint(Graphics &g)
{
	g.fillAll(Colours::white);
	if (animalPicture == 0)
		g.drawImageWithin(monkeyImage,getWidth()/5,0,getWidth(),getHeight(),RectanglePlacement::xLeft);
	else if (animalPicture == 1)
		g.drawImageWithin(rodentImage,getWidth()/5,0,getWidth(),getHeight(),RectanglePlacement::xLeft);

	g.setColour(Colours::lightgrey);

	int selectedContainer = editor->getSelectedContainer()-1;
	if ( selectedContainer  < 0)
		return;
	
	processor->lock.enter();
	AdvancerContainer c = processor->advancerContainers[selectedContainer];
	processor->lock.exit();
	minX = -100;
	maxX = 100;
	minY = -100;
	maxY = 100;

	float x0 = (0-minX)/(maxX-minX) * scale;
	float x1 = 0;
	float x2 = scale;
	float y0 = (0-minY)/(maxY-minY) * scale;
	float y1 = 0;
	float y2 = scale;


	g.drawLine(x1,y0,x2,y0);
	g.drawLine(x0,y1,x0,y2);


	for (int i=0;i<c.model.size();i++)
	{
		drawPolygon(g,&(c.model[i]),c.center.x,c.center.y);
	}

}


/*************************************/
/*************************************/

ContainerPlot::ContainerPlot(AdvancerCanvas *cv,AdvancerEditor* ed,AdvancerNode* n) :
		canvas(cv), editor(ed), processor(n)
{
	

}


void ContainerPlot::drawPolygon(Graphics &g,Polygon2D *p)
{

	g.setColour(juce::Colour(p->color[0],p->color[1],p->color[2]));
	for (int k=0;k<p->points.size()-1;k++)
	{
		float x1 = (p->points[k].x-minX)/(maxX-minX) * scale;
		float x2 = (p->points[k+1].x-minX)/(maxX-minX) * scale;
		float y1 = (p->points[k].y-minY)/(maxY-minY) * scale;
		float y2 = (p->points[k+1].y-minY)/(maxY-minY) * scale;

		g.drawLine(x1,y1,x2,y2);
	}
}

void ContainerPlot::resized()
{
		int width = getWidth();
	int height = getHeight();

	if (width > height)
		scale = height;
	else
		scale = width;
}

void ContainerPlot::mouseDown(const juce::MouseEvent& event)
{
	// if we clicked inside an advancer location, switch the current advancer to this new position

	int selectedContainer = editor->getSelectedContainer()-1;
	int selectedAdvancer =  editor->getSelectedAdvancer()-1;
	if ( selectedContainer  < 0 || selectedAdvancer < 0)
		return;
	
	processor->lock.enter();
	AdvancerContainer c = processor->advancerContainers[selectedContainer];
	processor->lock.exit();

	for (int i=0;i<c.advancerLocations.size();i++)
	{
		float rad = c.advancerLocations[i].rad;
		float x = c.advancerLocations[i].x;
		float y = c.advancerLocations[i].y;

		float x0 = ((x-rad)-minX)/(maxX-minX) * scale;
		float x1 = ((x+rad)-minX)/(maxX-minX) * scale;
		float y0 = ((y-rad)-minY)/(maxY-minY) * scale;
		float y1 = ((y+rad)-minY)/(maxY-minY) * scale;
		float xc = (x0+x1)/2;
		float yc = (y0+y1)/2;

		float sqrMousedistance = ( (event.x - xc)*(event.x - xc)+ (event.y - yc)*(event.y - yc));
		float radpix = (x1-x0)/2;
		if (sqrMousedistance <= radpix*radpix)
		{
			// yes! we hit a potential location

				processor->lock.enter();
				processor->updateAdvancerLocation(selectedContainer, selectedAdvancer, i);
				processor->lock.exit();
				break;
		}
	}

}

void ContainerPlot::paint(Graphics &g)
{
	g.fillAll(Colours::grey);
	
	g.setColour(Colours::lightgrey);

	int selectedContainer = editor->getSelectedContainer()-1;
	int selectedAdvancer =  editor->getSelectedAdvancer()-1;
	if ( selectedContainer  < 0)
		return;
	
	processor->lock.enter();
	AdvancerContainer c = processor->advancerContainers[selectedContainer];
	processor->lock.exit();

	c.getModelRange(minX,maxX,minY,maxY);
	float rangeX = maxX-minX;
	float rangeY = maxY-minY;
	minX-=0.1*rangeX;
	maxX+=0.1*rangeX;

	minY-=0.1*rangeY;
	maxY+=0.1*rangeY;

	float x0 = (0-minX)/(maxX-minX) * scale;
	float x1 = 0;
	float x2 = scale;
	float y0 = (0-minY)/(maxY-minY) * scale;
	float y1 = 0;
	float y2 = scale;


	g.drawLine(x1,y0,x2,y0);
	g.drawLine(x0,y1,x0,y2);



	for (int i=0;i<c.model.size();i++)
	{
		drawPolygon(g,&(c.model[i]));
	}

	for (int i=0;i<c.advancerLocations.size();i++)
	{
		bool advancerInThisLocation = false;
		bool selectedAdvancerInThisLocation  = false;
		for (int j=0;j<c.advancers.size();j++)
		{
			if (c.advancers[j].locationIndex == i)
			{
				advancerInThisLocation = true;
				selectedAdvancerInThisLocation = selectedAdvancer == j;
			}
		}

		float rad = c.advancerLocations[i].rad;
		float x = c.advancerLocations[i].x;
		float y = c.advancerLocations[i].y;

		float x0 = ((x-rad)-minX)/(maxX-minX) * scale;
		float x1 = ((x+rad)-minX)/(maxX-minX) * scale;

		float y0 = ((y-rad)-minY)/(maxY-minY) * scale;
		float y1 = ((y+rad)-minY)/(maxY-minY) * scale;

		if (advancerInThisLocation)
		{
			if (selectedAdvancerInThisLocation)
				g.setColour(Colours::orangered);
			else
				g.setColour(Colours::royalblue);
			g.fillEllipse(x0,y0,x1-x0,y1-y0);
		}
		else
		{
			g.setColour(Colours::aqua);
			g.drawEllipse(x0,y0,x1-x0,y1-y0,1);
		}
	}

	


}


AdvancerCanvas::AdvancerCanvas(AdvancerEditor* ed, AdvancerNode* n) :
    processor(n), editor(ed)
{
    setWantsKeyboardFocus(true);

   containerPlot = new ContainerPlot(this,ed,n);
   addAndMakeVisible(containerPlot);

   containersPlot = new ContainersPlot(this,ed,n);
   addAndMakeVisible(containersPlot);

   update();
	 
}

AdvancerCanvas::~AdvancerCanvas()
{
}

void AdvancerCanvas::paint(Graphics &g)
{


}

void AdvancerCanvas::update()
{
	resized();
	repaint();
}


void AdvancerCanvas::resized()
{
	int width = getWidth()/2;
	int height = getHeight();

	containerPlot->setBounds(0,0,width,height);
	containersPlot->setBounds(width,0,width,height);
}

void AdvancerCanvas::refresh()
{
	repaint();
}

void AdvancerCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    resized();
}

/*************************/
AdvancerEditor::AdvancerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors)
{
	desiredWidth = 280;
	tabText = "Advancers";

	containerCombobox = new ComboBox("AdvancerContainers");

	containerLabel = new Label("ContainerLabel","Containers:");
	containerLabel->setFont(Font("Default", 20, Font::plain));
	containerLabel->setEditable(false);
	containerLabel->setJustificationType(Justification::centredLeft);
	containerLabel->addListener(this);
	containerLabel->setBounds(25,35,100,20);
	addAndMakeVisible(containerLabel);
	
	containerCombobox->setEditableText(false);
	containerCombobox->setJustificationType(Justification::centredLeft);
	containerCombobox->addListener(this);
	containerCombobox->setBounds(120,35,130,20);
	addAndMakeVisible(containerCombobox);

	


	addContainer = new UtilityButton("+", titleFont);
    addContainer->addListener(this);
    addContainer->setRadius(3.0f);
    addContainer->setBounds(10,30,13,13);
    addAndMakeVisible(addContainer);

	removeContainer = new UtilityButton("-", titleFont);
    removeContainer->addListener(this);
    removeContainer->setRadius(3.0f);
    removeContainer->setBounds(10,45,13,13);
    addAndMakeVisible(removeContainer);




	advancerCombobox = new ComboBox("Advancers");
	

	advancerLabel = new Label("AdvancerLabel","Advancers:");
	advancerLabel->setFont(Font("Default", 20, Font::plain));
	advancerLabel->setEditable(false);
	advancerLabel->setJustificationType(Justification::centredLeft);
	advancerLabel->addListener(this);
	advancerLabel->setBounds(25,70,100,20);
	addAndMakeVisible(advancerLabel);
	
	advancerCombobox->setEditableText(false);
	advancerCombobox->setJustificationType(Justification::centredLeft);
	advancerCombobox->addListener(this);
	advancerCombobox->setBounds(120,70,130,20);
	addAndMakeVisible(advancerCombobox);

	
	addAdvancer = new UtilityButton("+", titleFont);
	addAdvancer->setEnabled(false);
    addAdvancer->addListener(this);
    addAdvancer->setRadius(3.0f);
    addAdvancer->setBounds(10,65,13,13);
    addAndMakeVisible(addAdvancer);

	removeAdvancer = new UtilityButton("-", titleFont);
    removeAdvancer->addListener(this);
    removeAdvancer->setRadius(3.0f);
	removeAdvancer->setEnabled(false);
    removeAdvancer->setBounds(10,80,13,13);
    addAndMakeVisible(removeAdvancer);

		
	depthLabel = new Label("DepthLabel","Depth:");
	depthLabel->setFont(Font("Default", 25, Font::plain));
	depthLabel->setEditable(false);
	depthLabel->setJustificationType(Justification::centredLeft);
	depthLabel->addListener(this);
	depthLabel->setBounds(10,100,100,20);
	addAndMakeVisible(depthLabel);

			
	depthEditLabel = new Label("DepthLabel","-");
	depthEditLabel->setFont(Font("Default", 25, Font::plain));
	depthEditLabel->setEditable(true);
	depthEditLabel->setEnabled(false);
	depthEditLabel->setColour(Label::textColourId, Colours::white);
	depthEditLabel->setColour(Label::backgroundColourId, Colours::grey);

	depthEditLabel->setJustificationType(Justification::centredLeft);
	depthEditLabel->addListener(this);
	depthEditLabel->setBounds(110,100,100,20);
	addAndMakeVisible(depthEditLabel);

	/*
	minusButton = new UtilityButton("+", titleFont);
    minusButton->addListener(this);
    minusButton->setRadius(3.0f);
    minusButton->setBounds(15,42,14,14);
    addAndMakeVisible(minusButton);
	*/

	setEnabledState(true);

}


AdvancerEditor::~AdvancerEditor()
{

}

Visualizer* AdvancerEditor::createNewCanvas() 
{
   AdvancerNode* processor = (AdvancerNode*) getProcessor();
    advancerCanvas = new AdvancerCanvas(this,processor);
	return advancerCanvas;
}


void AdvancerEditor::setActiveContainer(int index)
{
	containerCombobox->setSelectedId(index);
	updateFromProcessor();
}

void AdvancerEditor::setActiveAdvancer(int newAdvancerIndex) 
{
		advancerCombobox->setSelectedId(newAdvancerIndex);
		updateFromProcessor();
}

void AdvancerEditor::updateFromProcessor()
{

	AdvancerNode* processor = (AdvancerNode*) getProcessor();
	containerCombobox->clear();
	int numContainers = processor->advancerContainers.size();
	for (int i=0;i<	numContainers;i++)
	{
		containerCombobox->addItem(processor->advancerContainers[i].name, i+1);
	}
	if (numContainers > 0)
	{
		containerCombobox->setEditableText(true);
		addAdvancer->setEnabled(true);
	} else 
	{
		containerCombobox->setEditableText(false);
		removeContainer->setEnabled(false);
	}
	advancerCombobox->clear();
	int numAdvancersInContainer=0;

	selectedContainer = containerCombobox->getSelectedId();
	if (selectedContainer  > 0)
	{
		numAdvancersInContainer = processor->advancerContainers[selectedContainer-1].advancers.size();
		if (numAdvancersInContainer == 0)
		{
			advancerCombobox->setSelectedId(0);
		}
		for (int i=0;i<	numAdvancersInContainer;i++)
		{
			advancerCombobox->addItem(processor->advancerContainers[selectedContainer-1].advancers[i].name,i+1);
		}

	}
	if (numAdvancersInContainer > 0)
	{
		advancerCombobox->setEditableText(true);
		removeAdvancer->setEnabled(true);

		int selectedAdvancer = advancerCombobox->getSelectedId();
		if (selectedAdvancer > 0)
		{
			depthEditLabel->setText(String(processor->advancerContainers[selectedContainer-1].advancers[selectedAdvancer-1].depthMM,4),dontSendNotification);
			depthEditLabel->setEnabled(true);
		}

	} else {
		removeAdvancer->setEnabled(false);
		depthEditLabel->setText("-",dontSendNotification);
		depthEditLabel->setEnabled(false);
	}
	repaint();
	if (canvas != nullptr)
		canvas->repaint();

	
	ProcessorGraph *g = getProcessor()->getProcessorGraph();
	Array<GenericProcessor*> p = g->getListOfProcessors();
	for (int k=0;k<p.size();k++)
	{
		if (p[k]->getName() == "Spike Detector")
		{
			SpikeDetector *node = (SpikeDetector*)p[k];
			if (node != nullptr)
			{
				SpikeDetectorEditor *ed = (SpikeDetectorEditor *)node->getEditor();
				ed->updateAdvancerList();
			}
		}
	}
}

void AdvancerEditor::comboBoxChanged(ComboBox* comboBox)
{
	if (comboBox == containerCombobox)
	{
		// set active container.
		updateFromProcessor();
		if (advancerCombobox->getNumItems() > 0)
			advancerCombobox->setSelectedId(1);
	} else if (comboBox == advancerCombobox)
	{
		// set active advancer.
		updateFromProcessor();
	}
}

void AdvancerEditor::buttonEvent(Button* button)
{
	VisualizerEditor::buttonEvent(button);
		AdvancerNode* processor = (AdvancerNode*) getProcessor();

		//	NetworkEvents *processor  = (NetworkEvents*) getProcessor();
	if (button == addContainer)
	{
		PopupMenu containerMenu;
		PopupMenu gridMenu;
		PopupMenu hyperDriveMenu;
		hyperDriveMenu.addItem(2,"16ch drive");
		hyperDriveMenu.addItem(3,"32ch drive");
		hyperDriveMenu.addItem(4,"64ch drive");

		gridMenu.addItem(5, "Circular grid");
		gridMenu.addItem(6, "Custom grid [Load from file]",false);

		containerMenu.addItem(1,"Cannula");
		containerMenu.addSubMenu("Hyperdrive", hyperDriveMenu);
		containerMenu.addSubMenu("Grid", gridMenu);
		containerMenu.addItem(7,"Other [Load from file]",false);
		const int result = containerMenu.show();

		if (result == 1)
		{
			// build a new circular grid container.
			int newContainer = processor->addContainer("Cannula","");
			updateFromProcessor();
			setActiveContainer(newContainer);
		} else if (result == 2 || result == 3|| result == 4)
		{
			// build a new circular grid container.
			int numTetrodes = 4;
			if (result == 3) 
				numTetrodes = 8;
			if (result == 4) 
				numTetrodes = 16;
			int newContainer = processor->addContainer("Hyperdrive", String(numTetrodes));
			updateFromProcessor();
			setActiveContainer(newContainer);
		} else 	if (result == 5)
		{
			// build a new circular grid container.
			int newContainer = processor->addContainer("StandardGrid","");
			updateFromProcessor();
			setActiveContainer(newContainer);
		}

	} else if (button == removeContainer)
	{
		int sel = containerCombobox->getSelectedId();
		if (sel > 0) {
			int remaining = processor->removeContainer(sel-1);
			updateFromProcessor();
			containerCombobox->setSelectedId(remaining);

		}

	} else if (button == addAdvancer)
	{
		int sel = containerCombobox->getSelectedId();
		if (sel > 0)
		{
			int newAdvancerIndex = processor->addAdvancerToContainer(sel-1);
			updateFromProcessor();
			if (newAdvancerIndex >= 0)
				setActiveAdvancer(newAdvancerIndex);

		}
	} else if (button == removeAdvancer)

	{

	    int selcon = containerCombobox->getSelectedId();
		int sel = advancerCombobox->getSelectedId();
		if (sel > 0 & selcon > 0)
		{
			int remaininAdvancers = processor->removeAdvancer(selcon-1,sel-1);
			updateFromProcessor();
			advancerCombobox->setSelectedId(remaininAdvancers);

		}

	}
}
void AdvancerEditor::labelTextChanged(juce::Label *label)
{
	AdvancerNode* processor = (AdvancerNode*) getProcessor();

	if (label == depthEditLabel)
	{
		Value v = depthEditLabel->getTextValue();
		
		int selectedContainer = containerCombobox->getSelectedId();
		int selectedAdvancer = advancerCombobox->getSelectedId();
		float value = v.getValue();
		processor->updateAdvancerPosition(selectedContainer-1,selectedAdvancer-1,value);
	
	}
}


int AdvancerEditor::getSelectedContainer()
{
	return containerCombobox->getSelectedId();
}


int AdvancerEditor::getSelectedAdvancer()
{
	return advancerCombobox->getSelectedId();
}