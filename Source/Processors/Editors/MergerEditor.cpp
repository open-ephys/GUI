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

#include "MergerEditor.h"
#include "../Utilities/Merger.h"
#include "../ProcessorGraph.h"
#include "../../UI/EditorViewport.h"

// PipelineSelectorButton::PipelineSelectorButton()
// 	: DrawableButton ("Selector", DrawableButton::ImageFitted)
// {
// 	DrawablePath normal, over, down;

// 	    Path p;
//         p.addTriangle (0.0f, 0.0f, 0.0f, 20.0f, 18.0f, 10.0f);
//         normal.setPath (p);
//         normal.setFill (Colours::lightgrey);
//         normal.setStrokeThickness (0.0f);

//         over.setPath (p);
//         over.setFill (Colours::black);
//         over.setStrokeFill (Colours::black);
//         over.setStrokeThickness (5.0f);

//         setImages (&normal, &over, &over);
//         setBackgroundColours(Colours::darkgrey, Colours::purple);
//         setClickingTogglesState (true);
//         setTooltip ("Toggle a state.");

// }

// PipelineSelectorButton::~PipelineSelectorButton()
// {
// }

MergerEditor::MergerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
    desiredWidth = 85;

    pipelineSelectorA = new ImageButton("Pipeline A");

    Image normalImageA = ImageCache::getFromMemory(BinaryData::MergerB01_png, BinaryData::MergerB01_pngSize);
    Image downImageA = ImageCache::getFromMemory(BinaryData::MergerA01_png, BinaryData::MergerA01_pngSize);
    Image normalImageB = ImageCache::getFromMemory(BinaryData::MergerA02_png, BinaryData::MergerA02_pngSize);
    Image downImageB = ImageCache::getFromMemory(BinaryData::MergerB02_png, BinaryData::MergerB02_pngSize);

    pipelineSelectorA->setImages(true, true, true,
                                 normalImageA, 1.0f, Colours::white.withAlpha(0.0f),
                                 normalImageA, 1.0f, Colours::black.withAlpha(0.0f),
                                 downImageA, 1.0f, Colours::white.withAlpha(0.0f));


    pipelineSelectorA->addListener(this);
    pipelineSelectorA->setBounds(-10,25,95,50);
    pipelineSelectorA->setToggleState(true,false);
    addAndMakeVisible(pipelineSelectorA);

    pipelineSelectorB = new ImageButton("Pipeline B");

    pipelineSelectorB->setImages(true, true, true,
                                 normalImageB, 1.0f, Colours::white.withAlpha(0.0f),
                                 normalImageB, 1.0f, Colours::black.withAlpha(0.0f),
                                 downImageB, 1.0f, Colours::white.withAlpha(0.0f));

    pipelineSelectorB->addListener(this);
    pipelineSelectorB->setBounds(-10,75,95,50);
    pipelineSelectorB->setToggleState(false,false);
    addAndMakeVisible(pipelineSelectorB);

    eventsButtonA = new UtilityButton("E", Font("Small Text", 13, Font::plain));
    eventsButtonA->addListener(this);
    eventsButtonA->setBounds(5,25,15,15);
    eventsButtonA->setClickingTogglesState(true);
    eventsButtonA->setToggleState(true, false);
    addAndMakeVisible(eventsButtonA);

    continuousButtonA = new UtilityButton("C", Font("Small Text", 13, Font::plain));
    continuousButtonA->addListener(this);
    continuousButtonA->setBounds(5,40,15,15);
    continuousButtonA->setClickingTogglesState(true);
    continuousButtonA->setToggleState(true, false);
    addAndMakeVisible(continuousButtonA);

     eventsButtonB = new UtilityButton("E", Font("Small Text", 13, Font::plain));
    eventsButtonB->addListener(this);
    eventsButtonB->setBounds(5,95,15,15);
    eventsButtonB->setClickingTogglesState(true);
    eventsButtonB->setToggleState(true, false);
    addAndMakeVisible(eventsButtonB);
    eventsButtonB->setVisible(false);

    continuousButtonB = new UtilityButton("C", Font("Small Text", 13, Font::plain));
    continuousButtonB->addListener(this);
    continuousButtonB->setBounds(5,110,15,15);
    continuousButtonB->setClickingTogglesState(true);
    continuousButtonB->setToggleState(true, false);
    addAndMakeVisible(continuousButtonB);
    continuousButtonB->setVisible(false);

}

MergerEditor::~MergerEditor()
{

}

void MergerEditor::buttonEvent(Button* button)
{

    Merger* processor = (Merger*) getProcessor();

    if (!acquisitionIsActive)
    {

       if (button == eventsButtonA)
        {
            processor->sendEventsForSourceA = button->getToggleState();

        } else if (button == eventsButtonB)
        {
            processor->sendEventsForSourceB = button->getToggleState();

        } else if (button == continuousButtonA)
        {
            processor->sendContinuousForSourceA = button->getToggleState();

        } else if (button == continuousButtonB)
        {
            processor->sendContinuousForSourceB = button->getToggleState();

        }

        if (button == eventsButtonA || 
            button == eventsButtonB || 
            button == continuousButtonA || 
            button == continuousButtonB)
        {
            getEditorViewport()->makeEditorVisible(this, false, true);
        }


    } else {
        if (button == eventsButtonA || 
            button == eventsButtonB || 
            button == continuousButtonA || 
            button == continuousButtonB)
        {
            button->setToggleState(!button->getToggleState(),false);
        }
    }

    if (button == pipelineSelectorA)
    {
        pipelineSelectorA->setToggleState(true,false);
        pipelineSelectorB->setToggleState(false,false);
        processor->switchIO(0);

        eventsButtonA->setVisible(true);
        continuousButtonA->setVisible(true);

        eventsButtonB->setVisible(false);
        continuousButtonB->setVisible(false);

         getEditorViewport()->makeEditorVisible(this, false);

    }
    else if (button == pipelineSelectorB)
    {
        pipelineSelectorB->setToggleState(true,false);
        pipelineSelectorA->setToggleState(false,false);
        processor->switchIO(1);

        eventsButtonB->setVisible(true);
        continuousButtonB->setVisible(true);

        eventsButtonA->setVisible(false);
        continuousButtonA->setVisible(false);

        getEditorViewport()->makeEditorVisible(this, false);

    }

   


    

    

    
}

void MergerEditor::mouseDown(const MouseEvent& e)
{



    if (e.mods.isRightButtonDown())
    {

        PopupMenu m;
        m.addItem(1, "Choose input 2:",false);

        Array<GenericProcessor*> availableProcessors = getProcessorGraph()->getListOfProcessors();

        for (int i = 0; i < availableProcessors.size(); i++)
        {
            if (!availableProcessors[i]->isSink() && 
                !availableProcessors[i]->isMerger() &&
                !availableProcessors[i]->isSplitter() &&
                availableProcessors[i]->getDestNode() != getProcessor())
            {

                String name = String(availableProcessors[i]->getNodeId());
                name += " - ";
                name += availableProcessors[i]->getName();

                m.addItem(i+2, name);
                //processorsInList.add(availableProcessors[i]);
            }
        }

        const int result = m.show();

        if (result > 1)
        {
            std::cout << "Selected " << availableProcessors[result-2]->getName() << std::endl;
        
            switchSource(1);

            Merger* processor = (Merger*) getProcessor();
            processor->setMergerSourceNode(availableProcessors[result-2]);
            availableProcessors[result-2]->setDestNode(getProcessor());

            getGraphViewer()->updateNodeLocations();

            getEditorViewport()->makeEditorVisible(this, false, true);
        }
    }

    

}

void MergerEditor::switchSource(int source)
{
    if (source == 0)
    {
        pipelineSelectorA->setToggleState(true,false);
        pipelineSelectorB->setToggleState(false,false);
        Merger* processor = (Merger*) getProcessor();
        processor->switchIO(0);

    }
    else if (source == 1)
    {
        pipelineSelectorB->setToggleState(true,false);
        pipelineSelectorA->setToggleState(false,false);
        Merger* processor = (Merger*) getProcessor();
        processor->switchIO(1);

    }
}

Array<GenericEditor*> MergerEditor::getConnectedEditors()
{

    Array<GenericEditor*> editors;

    Merger* processor = (Merger*) getProcessor();
    
    for (int pathNum = 0; pathNum < 2; pathNum++)
    {
        processor->switchIO();

        if (processor->getSourceNode() != nullptr)
            editors.add(processor->getSourceNode()->getEditor());
        else
            editors.add(nullptr);
    }
    
    return editors;

}

int MergerEditor::getPathForEditor(GenericEditor* editor)
{
    Merger* processor = (Merger*) getProcessor();
    
    for (int pathNum = 0; pathNum < 2; pathNum++)
    {
    switchSource(pathNum);
    
    if (processor->getSourceNode() != nullptr)
    {
        if (processor->getEditor() == editor)
            return pathNum;
    }
    }
    
    return -1;

}

void MergerEditor::switchIO(int source)
{
    switchSource(source);

    select();
}


void MergerEditor::switchSource()
{

    bool isBOn = pipelineSelectorB->getToggleState();
    bool isAOn = pipelineSelectorA->getToggleState();

    pipelineSelectorB->setToggleState(!isBOn,false);
    pipelineSelectorA->setToggleState(!isAOn,false);

    Merger* processor = (Merger*) getProcessor();
    processor->switchIO();

}


void MergerEditor::saveCustomParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "MergerEditor");

    XmlElement* textLabelValues = xml->createNewChildElement("SENDS");
    textLabelValues->setAttribute("EventsA",eventsButtonA->getToggleState());
    textLabelValues->setAttribute("EventsB",eventsButtonB->getToggleState());
    textLabelValues->setAttribute("ContinuousA",continuousButtonA->getToggleState());
    textLabelValues->setAttribute("ContinuousB",continuousButtonB->getToggleState());
}

void MergerEditor::loadCustomParameters(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("SENDS"))
        {

            eventsButtonA->setToggleState(xmlNode->getBoolAttribute("EventsA"),true);
            eventsButtonB->setToggleState(xmlNode->getBoolAttribute("EventsB"),true);
            continuousButtonA->setToggleState(xmlNode->getBoolAttribute("ContinuousA"),true);
            continuousButtonB->setToggleState(xmlNode->getBoolAttribute("ContinuousB"),true);
        }
    }
}
