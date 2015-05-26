/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#include "UIComponent.h"
#include <stdio.h>

UIComponent::UIComponent(MainWindow* mainWindow_, ProcessorGraph* pgraph, AudioComponent* audio_)
    : mainWindow(mainWindow_), processorGraph(pgraph), audio(audio_)

{

    processorGraph->createDefaultNodes();

    messageCenterEditor = (MessageCenterEditor*) processorGraph->getMessageCenter()->createEditor();
    addActionListener(messageCenterEditor);
    addAndMakeVisible(messageCenterEditor);

    infoLabel = new InfoLabel();
    std::cout << "Created info label." << std::endl;

    graphViewer = new GraphViewer();
    std::cout << "Created graph viewer." << std::endl;

    dataViewport = new DataViewport();
    addChildComponent(dataViewport);
    dataViewport->addTabToDataViewport("Info", infoLabel,0);
    dataViewport->addTabToDataViewport("Graph", graphViewer,0);

    std::cout << "Created data viewport." << std::endl;

    editorViewport = new EditorViewport();

    addAndMakeVisible(editorViewport);

    std::cout << "Created filter viewport." << std::endl;

    editorViewportButton = new EditorViewportButton(this);
    addAndMakeVisible(editorViewportButton);

    controlPanel = new ControlPanel(processorGraph, audio);
    addAndMakeVisible(controlPanel);

    std::cout << "Created control panel." << std::endl;

    processorList = new ProcessorList();
    processorListViewport.setViewedComponent(processorList,false);
    processorListViewport.setScrollBarsShown(true,false);
    addAndMakeVisible(&processorListViewport);
    processorList->setVisible(true);
    processorList->setBounds(0,0,195,processorList->getTotalHeight());
    std::cout << "Created filter list." << std::endl;

    std::cout << "Created message center." << std::endl;

    setBounds(0,0,500,400);

    AccessClass::setUIComponent(this);
    controlPanel->updateChildComponents();

    processorGraph->updatePointers(); // needs to happen after processorGraph gets the right pointers

#if JUCE_MAC
    MenuBarModel::setMacMainMenu(this);
    mainWindow->setMenuBar(0);
#else
    mainWindow->setMenuBar(this);
#endif

}

UIComponent::~UIComponent()
{
    dataViewport->destroyTab(0); // get rid of tab for InfoLabel
    AccessClass::shutdownBroadcaster();
}

void UIComponent::resized()
{

    int w = getWidth();
    int h = getHeight();

    if (editorViewportButton != 0)
    {
        editorViewportButton->setBounds(w-230, h-40, 225, 35);

        if (h < 300 && editorViewportButton->isOpen())
            editorViewportButton->toggleState();

        if (h < 200)
            editorViewportButton->setBounds(w-230,h-40+200-h,225,35);
        //else
        //    editorViewportButton->setVisible(true);
    }

    if (editorViewport != 0)
    {
        //if (h < 400)
        //    editorViewport->setVisible(false);
        //else
        //    editorViewport->setVisible(true);

        if (editorViewportButton->isOpen() && !editorViewport->isVisible())
            editorViewport->setVisible(true);
        else if (!editorViewportButton->isOpen() && editorViewport->isVisible())
            editorViewport->setVisible(false);

        editorViewport->setBounds(6,h-190,w-11,150);


    }

    if (controlPanel != 0)
    {

        int controlPanelWidth = w-210;
        int addHeight = 0;
        int leftBound;

        if (w >= 460)
        {
            leftBound = 202;
        }
        else
        {
            leftBound = w-258;
            controlPanelWidth = w-leftBound;
        }

        if (controlPanelWidth < 750)
        {
            addHeight = 750-controlPanelWidth;

            if (addHeight > 32)
                addHeight = 32;
        }

        if (controlPanelWidth < 570)
        {
            addHeight = 32 + 570-controlPanelWidth;

            if (addHeight > 64)
                addHeight = 64;
        }

        if (controlPanel->isOpen())
            controlPanel->setBounds(leftBound,6,controlPanelWidth,64+addHeight);
        else
            controlPanel->setBounds(leftBound,6,controlPanelWidth,32+addHeight);
    }

    if (processorList != 0)
    {
        if (processorList->isOpen())
        {
            if (editorViewportButton->isOpen())
                processorListViewport.setBounds(5,5,195,h-200);
            else
                processorListViewport.setBounds(5,5,195,h-50);

            processorListViewport.setScrollBarsShown(true,false);

        }
        else
        {
            processorListViewport.setBounds(5,5,195,34);
            processorListViewport.setScrollBarsShown(false,false);
            processorListViewport.setViewPosition(0, 0);
        }

        if (w < 460)
            processorListViewport.setBounds(5-460+getWidth(),5,195,processorList->getHeight());
    }

    if (dataViewport != 0)
    {
        int left, top, width, height;
        left = 6;
        top = 40;

        if (processorList->isOpen())
            left = processorListViewport.getX()+processorListViewport.getWidth()+2;
        else
            left = 6;

        top = controlPanel->getHeight()+8;

        if (editorViewportButton->isOpen())
            height = h - top - 195;
        else
            height = h - top - 45;

        width = w - left - 5;

        dataViewport->setBounds(left, top, width, height);

        if (h < 200)
            dataViewport->setVisible(false);
        else
            dataViewport->setVisible(true);

    }



    if (messageCenterEditor != 0)
    {
        messageCenterEditor->setBounds(6,h-35,w-241,30);
        if (h < 200)
            messageCenterEditor->setBounds(6,h-35+200-h,w-241,30);
        //  else
        //      messageCenter->setVisible(true);
    }

    // for debugging purposes:
    if (false)
    {
        dataViewport->setVisible(false);
        editorViewport->setVisible(false);
        processorList->setVisible(false);
        messageCenterEditor->setVisible(false);
        controlPanel->setVisible(false);
        editorViewportButton->setVisible(false);
    }

}

void UIComponent::disableCallbacks()
{
    //sendActionMessage("Data acquisition terminated.");
    controlPanel->disableCallbacks();
}

void UIComponent::disableDataViewport()
{
    dataViewport->disableConnectionToEditorViewport();
}

void UIComponent::childComponentChanged()
{
    resized();
}




//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// MENU BAR METHODS

StringArray UIComponent::getMenuBarNames()
{

    const char* const names[] = { "File", "Edit", "View", "Help", 0 };

    return StringArray(names);

}

PopupMenu UIComponent::getMenuForIndex(int menuIndex, const String& menuName)
{
    ApplicationCommandManager* commandManager = &(mainWindow->commandManager);

    PopupMenu menu;

    if (menuIndex == 0)
    {
        menu.addCommandItem(commandManager, openConfiguration);
        menu.addCommandItem(commandManager, saveConfiguration);
        menu.addCommandItem(commandManager, saveConfigurationAs);
        menu.addSeparator();
        menu.addCommandItem(commandManager, reloadOnStartup);

#if !JUCE_MAC
        menu.addSeparator();
        menu.addCommandItem(commandManager, StandardApplicationCommandIDs::quit);
#endif

    }
    else if (menuIndex == 1)
    {
        menu.addCommandItem(commandManager, undo);
        menu.addCommandItem(commandManager, redo);
        menu.addSeparator();
        menu.addCommandItem(commandManager, copySignalChain);
        menu.addCommandItem(commandManager, pasteSignalChain);
        menu.addSeparator();
        menu.addCommandItem(commandManager, clearSignalChain);

    }
    else if (menuIndex == 2)
    {

        menu.addCommandItem(commandManager, toggleProcessorList);
        menu.addCommandItem(commandManager, toggleSignalChain);
        menu.addCommandItem(commandManager, toggleFileInfo);
        menu.addSeparator();
        menu.addCommandItem(commandManager, resizeWindow);

    }
    else if (menuIndex == 3)
    {
        menu.addCommandItem(commandManager, showHelp);
    }

    return menu;

}

void UIComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{

    //
}

// ApplicationCommandTarget methods

ApplicationCommandTarget* UIComponent::getNextCommandTarget()
{
    // this will return the next parent component that is an ApplicationCommandTarget (in this
    // case, there probably isn't one, but it's best to use this method anyway).
    return findFirstTargetParentComponent();
}

void UIComponent::getAllCommands(Array <CommandID>& commands)
{
    const CommandID ids[] = {openConfiguration,
                             saveConfiguration,
                             saveConfigurationAs,
                             reloadOnStartup,
                             undo,
                             redo,
                             copySignalChain,
                             pasteSignalChain,
                             clearSignalChain,
                             toggleProcessorList,
                             toggleSignalChain,
                             toggleFileInfo,
                             showHelp,
                             resizeWindow
                            };

    commands.addArray(ids, numElementsInArray(ids));

}

void UIComponent::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{

    bool acquisitionStarted = getAudioComponent()->callbacksAreActive();

    switch (commandID)
    {
        case openConfiguration:
            result.setInfo("Open...", "Load a saved processor graph.", "General", 0);
            result.addDefaultKeypress('O', ModifierKeys::commandModifier);
            result.setActive(!acquisitionStarted);
            break;

        case saveConfiguration:
            result.setInfo("Save", "Save the current processor graph.", "General", 0);
            result.addDefaultKeypress('S', ModifierKeys::commandModifier);
            break;

        case saveConfigurationAs:
            result.setInfo("Save as...", "Save the current processor graph with a new name.", "General", 0);
            result.addDefaultKeypress('S', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
            break;

        case reloadOnStartup:
            result.setInfo("Reload on startup", "Load the last used configuration on startup.", "General", 0);
            result.setActive(!acquisitionStarted);
            result.setTicked(mainWindow->shouldReloadOnStartup);
            break;

        case undo:
            result.setInfo("Undo", "Undo the last action.", "General", 0);
            result.addDefaultKeypress('Z', ModifierKeys::commandModifier);
            result.setActive(false);
            break;

        case redo:
            result.setInfo("Redo", "Undo the last action.", "General", 0);
            result.addDefaultKeypress('Y', ModifierKeys::commandModifier);
            result.setActive(false);
            break;

        case copySignalChain:
            result.setInfo("Copy", "Copy a portion of the signal chain.", "General", 0);
            result.addDefaultKeypress('C', ModifierKeys::commandModifier);
            result.setActive(false);
            break;

        case pasteSignalChain:
            result.setInfo("Paste", "Paste a portion of the signal chain.", "General", 0);
            result.addDefaultKeypress('V', ModifierKeys::commandModifier);
            result.setActive(false);
            break;

        case clearSignalChain:
            result.setInfo("Clear signal chain", "Clear the current signal chain.", "General", 0);
            result.addDefaultKeypress(KeyPress::backspaceKey, ModifierKeys::commandModifier);
            result.setActive(!getEditorViewport()->isSignalChainEmpty() && !acquisitionStarted);
            break;

        case toggleProcessorList:
            result.setInfo("Processor List", "Show/hide Processor List.", "General", 0);
            result.addDefaultKeypress('P', ModifierKeys::shiftModifier);
            result.setTicked(processorList->isOpen());
            break;

        case toggleSignalChain:
            result.setInfo("Signal Chain", "Show/hide Signal Chain.", "General", 0);
            result.addDefaultKeypress('S', ModifierKeys::shiftModifier);
            result.setTicked(editorViewportButton->isOpen());
            break;

        case toggleFileInfo:
            result.setInfo("File Info", "Show/hide File Info.", "General", 0);
            result.addDefaultKeypress('F', ModifierKeys::shiftModifier);
            result.setTicked(controlPanel->isOpen());
            break;

        case showHelp:
            result.setInfo("Show help...", "Take me to the GUI wiki.", "General", 0);
            result.setActive(true);
            break;

        case resizeWindow:
            result.setInfo("Reset window bounds", "Reset window bounds", "General", 0);
            break;

        default:
            break;
    };

}

bool UIComponent::perform(const InvocationInfo& info)
{

    switch (info.commandID)
    {
        case openConfiguration:
            {
                FileChooser fc("Choose a file to load...",
                               File::getCurrentWorkingDirectory(),
                               "*",
                               true);

                if (fc.browseForFileToOpen())
                {
                    currentConfigFile = fc.getResult();
                    sendActionMessage(getEditorViewport()->loadState(currentConfigFile));
                }
                else
                {
                    sendActionMessage("No configuration selected.");
                }

                break;
            }
        case saveConfiguration:
            {

                if (currentConfigFile.exists())
                {
                    sendActionMessage(getEditorViewport()->saveState(currentConfigFile));
                }
                else
                {
                    FileChooser fc("Choose the file name...",
                                   File::getCurrentWorkingDirectory(),
                                   "*",
                                   true);

                    if (fc.browseForFileToSave(true))
                    {
                        currentConfigFile = fc.getResult();
                        std::cout << currentConfigFile.getFileName() << std::endl;
                        sendActionMessage(getEditorViewport()->saveState(currentConfigFile));
                    }
                    else
                    {
                        sendActionMessage("No file chosen.");
                    }
                }

                break;
            }

        case saveConfigurationAs:
            {

                FileChooser fc("Choose the file name...",
                               File::getCurrentWorkingDirectory(),
                               "*",
                               true);

                if (fc.browseForFileToSave(true))
                {
                    currentConfigFile = fc.getResult();
                    std::cout << currentConfigFile.getFileName() << std::endl;
                    sendActionMessage(getEditorViewport()->saveState(currentConfigFile));
                }
                else
                {
                    sendActionMessage("No file chosen.");
                }

                break;
            }

        case reloadOnStartup:
            {
                mainWindow->shouldReloadOnStartup = !mainWindow->shouldReloadOnStartup;

            }
            break;

        case clearSignalChain:
            {
                getEditorViewport()->clearSignalChain();
                break;
            }

        case showHelp:
            {
                URL url = URL("https://open-ephys.atlassian.net/wiki/display/OEW/Open+Ephys+GUI");
                url.launchInDefaultBrowser();
                break;
            }

        case toggleProcessorList:
            processorList->toggleState();
            break;

        case toggleFileInfo:
            controlPanel->toggleState();
            break;

        case toggleSignalChain:
            editorViewportButton->toggleState();
            break;

        case resizeWindow:
            mainWindow->centreWithSize(800, 600);
            break;

        default:
            break;

    }

    return true;

}


void UIComponent::saveStateToXml(XmlElement* xml)
{
    XmlElement* uiComponentState = xml->createNewChildElement("UICOMPONENT");
    uiComponentState->setAttribute("isProcessorListOpen",processorList->isOpen());
    uiComponentState->setAttribute("isEditorViewportOpen",editorViewportButton->isOpen());
}

void UIComponent::loadStateFromXml(XmlElement* xml)
{
    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("UICOMPONENT"))
        {

            bool isProcessorListOpen = xmlNode->getBoolAttribute("isProcessorListOpen");
            bool isEditorViewportOpen = xmlNode->getBoolAttribute("isEditorViewportOpen");

            if (!isProcessorListOpen)
            {
                processorList->toggleState();
            }

            if (!isEditorViewportOpen)
            {
                editorViewportButton->toggleState();
            }

        }
    }
}

StringArray UIComponent::getRecentlyUsedFilenames()
{
    return controlPanel->getRecentlyUsedFilenames();
}

void UIComponent::setRecentlyUsedFilenames(const StringArray& filenames)
{
    controlPanel->setRecentlyUsedFilenames(filenames);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

EditorViewportButton::EditorViewportButton(UIComponent* ui) : UI(ui)
{
    open = true;

    buttonFont = Font("Default Light", 25, Font::plain);

    // MemoryInputStream mis1(BinaryData::cpmonolightserialized,
    //                        BinaryData::cpmonolightserializedSize,
    //                        false);
    // Typeface::Ptr tp1 = new CustomTypeface(mis1);
    // buttonFont = Font(tp1);
    // buttonFont.setHeight(25);

}

EditorViewportButton::~EditorViewportButton()
{

}


void EditorViewportButton::paint(Graphics& g)
{

    g.fillAll(Colour(58,58,58));

    g.setColour(Colours::white);
    g.setFont(buttonFont);
    g.drawText("SIGNAL CHAIN", 10, 0, getWidth(), getHeight(), Justification::left, false);

    g.setColour(Colours::white);

    Path p;

    float h = getHeight();
    float w = getWidth()-5;

    if (open)
    {
        p.addTriangle(w-h+0.3f*h, 0.7f*h,
                      w-h+0.5f*h, 0.3f*h,
                      w-h+0.7f*h, 0.7f*h);
    }
    else
    {
        p.addTriangle(w-h+0.3f*h, 0.5f*h,
                      w-h+0.7f*h, 0.3f*h,
                      w-h+0.7f*h, 0.7f*h);
    }

    PathStrokeType pst = PathStrokeType(1.0f, PathStrokeType::curved, PathStrokeType::rounded);

    g.strokePath(p, pst);

}


void EditorViewportButton::mouseDown(const MouseEvent& e)
{
    open = !open;
    UI->childComponentChanged();
    repaint();

}

void EditorViewportButton::toggleState()
{
    open = !open;
    UI->childComponentChanged();
    repaint();
}
