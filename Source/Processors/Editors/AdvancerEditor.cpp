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

#include "AdvancerEditor.h"
#include "../AdvancerNode.h"
#include "SpikeDetectorEditor.h"
#include "../SpikeDetector.h"
#include <stdio.h>



AdvancerDisplay::AdvancerDisplay(AdvancerNode *p, AdvancerCanvas* sdc, Viewport* v) :
    processor(p),canvas(sdc), viewport(v)
{


}

AdvancerDisplay::~AdvancerDisplay()
{
}

void AdvancerDisplay::paint(Graphics &g)
{
}
void AdvancerDisplay::resized()
{
}

/************/

AdvancerCanvas::AdvancerCanvas(AdvancerNode* n) :
    processor(n)
{
    viewport = new Viewport();
	advancerDisplay = new AdvancerDisplay(n,this, viewport);

    viewport->setViewedComponent(advancerDisplay, false);
    viewport->setScrollBarsShown(true, true);

    addAndMakeVisible(viewport);

    setWantsKeyboardFocus(true);

    update();
	 
}

AdvancerCanvas::~AdvancerCanvas()
{
}

void AdvancerCanvas::paint(Graphics &g)
{
}

/*************************/
AdvancerEditor::AdvancerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors)
{
	desiredWidth = 280;

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

	setEnabledState(false);

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


	
	ProcessorGraph *g = getProcessor()->getProcessorGraph();
	Array<GenericProcessor*> p = g->getListOfProcessors();
	for (int k=0;k<p.size();k++)
	{
		if (p[k]->getName() == "Spike Detector")
		{
			SpikeDetector *node = (SpikeDetector*)p[k];
			SpikeDetectorEditor *ed = (SpikeDetectorEditor *)node->getEditor();
			ed->updateAdvancerList();
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
		gridMenu.addItem(6, "Custom grid [Load from file]");

		containerMenu.addItem(1,"Cannula");
		containerMenu.addSubMenu("Hyperdrive", hyperDriveMenu);
		containerMenu.addSubMenu("Grid", gridMenu);
		containerMenu.addItem(4,"Other [Load from file]");
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
		} else 
		if (result == 5)
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


AdvancerEditor::~AdvancerEditor()
{

}

Visualizer* AdvancerEditor::createNewCanvas() 
{
   AdvancerNode* processor = (AdvancerNode*) getProcessor();
    advancerCanvas = new AdvancerCanvas(processor);
	return advancerCanvas;
}

void AdvancerEditor::saveCustomParametersToXml(XmlElement* parentElement)
{
}

void AdvancerEditor::loadCustomParametersFromXml()
{
}

