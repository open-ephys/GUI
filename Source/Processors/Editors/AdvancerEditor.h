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


#ifndef __ADVANCER_EDITOR_H
#define __ADVANCER_EDITOR_H

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"
#include "VisualizerEditor.h"
#include "../AdvancerNode.h"

class AdvancerNode;
class AdvancerEditor;

class AdvancerCanvas;


class ContainersPlot : public Component, public Button::Listener
{
public:
	ContainersPlot(AdvancerCanvas *cv,AdvancerEditor* ed,AdvancerNode* n);
	void drawPolygon(Graphics &g,Polygon2D *p, float x0, float y0);

	void paint(Graphics &g);
	void resized();
	void buttonClicked(juce::Button *button);
private:
	int animalPicture;
	float minX,minY,maxX,maxY,scale;
	AdvancerCanvas* canvas;
	AdvancerEditor* editor;
	AdvancerNode* processor;
	Image monkeyImage, rodentImage;
	UtilityButton* animalButton;
};

class ContainerPlot : public Component
{
public:
	ContainerPlot(AdvancerCanvas *cv,AdvancerEditor* ed,AdvancerNode* n);
	void paint(Graphics &g);
	void drawPolygon(Graphics &g,Polygon2D *p);
	void resized();
	void mouseDown(const juce::MouseEvent& event);

private:
	float minX,minY,maxX,maxY,scale;
	AdvancerCanvas* canvas;
	AdvancerEditor* editor;
	AdvancerNode* processor;

};

class AdvancerCanvas : public Visualizer

{
public:
    AdvancerCanvas(AdvancerEditor* ed,AdvancerNode* n);
    ~AdvancerCanvas();

    void paint(Graphics& g);

  //  void refresh();

	void beginAnimation() {}
	void endAnimation() {}
	
    void setParameter(int, float) {}
    void setParameter(int, int, int, float) {}
	
	void update() ;
	void refreshState() ;
	void refresh() ;

	void resized() ;

    void startRecording() { } // unused
    void stopRecording() { } // unused
    
    AdvancerNode* processor;
	AdvancerEditor* editor;
private:
	ContainerPlot* containerPlot;
	ContainersPlot* containersPlot;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdvancerCanvas);

};




class AdvancerEditor : public VisualizerEditor,public Label::Listener,  public ComboBox::Listener

{
public:
    AdvancerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~AdvancerEditor();

    void buttonEvent(Button* button);
	void labelTextChanged(juce::Label *);
	void comboBoxChanged(ComboBox* comboBox);
	Visualizer* createNewCanvas() ;
	void updateFromProcessor();

	void saveCustomParametersToXml(XmlElement* parentElement);
	void loadCustomParametersFromXml();
	
	int getSelectedContainer();
	void setActiveContainer(int index);
	int getSelectedAdvancer();

private:
	void setActiveAdvancer(int newAdvancerIndex) ;

	int selectedContainer;
	ComboBox* advancerCombobox;
	ComboBox* containerCombobox;
	Label *containerLabel, *advancerLabel, *depthLabel, *depthEditLabel;
	UtilityButton *addContainer, *removeContainer;
	UtilityButton *addAdvancer, *removeAdvancer;
	
	AdvancerCanvas* advancerCanvas;
	
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdvancerEditor);

};



#endif 
