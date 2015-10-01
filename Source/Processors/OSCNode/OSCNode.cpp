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

#include "OSCNode.h"
#include "OSCEditor.h"

#include "../../UI/EditorViewport.h"

OSCNode::OSCNode()
    : GenericProcessor("OSCNode"),
      destNodeA(0), destNodeB(0), activePath(0)
{
    sendSampleCount = false;
}

OSCNode::~OSCNode()
{

}

AudioProcessorEditor* OSCNode::createEditor()
{
    editor = new OSCEditor(this, true);
    //tEditor(editor);

    //std::cout << "Creating editor." << std::endl;
    return editor;
}

bool OSCNode::isReady()
{
    DBG("Is ready!");
    osc.startThread();
    return true;
}

void OSCNode::setPathToProcessor(GenericProcessor* p)
{

    if (destNodeA == p)
    {
        switchIO(0);

    }
    else if (destNodeB == p)
    {
        switchIO(1);
    }


}

void OSCNode::setOSCNodeDestNode(GenericProcessor* dn)
{
    destNode = dn;

    if (activePath == 0)
    {
        std::cout << "Setting destination node A." << std::endl;
        destNodeA = dn;
    }
    else
    {
        destNodeB = dn;
        std::cout << "Setting destination node B." << std::endl;

    }
}

void OSCNode::switchIO(int destNum)
{

    //std::cout << "Switching to dest number " << destNum << std::endl;

    activePath = destNum;

    if (destNum == 0)
    {
        destNode = destNodeA;
        // std::cout << "Dest node: " << getDestNode() << std::endl;
    }
    else
    {
        destNode = destNodeB;
        // std::cout << "Dest node: " << getDestNode() << std::endl;
    }

    // getEditorViewport()->makeEditorVisible(getEditor(), false);

}

void OSCNode::switchIO()
{

    //std::cout << "OSCNode switching source." << std::endl;

    if (activePath == 0)
    {
        activePath = 1;
        destNode = destNodeB;
    }
    else
    {
        activePath = 0;
        destNode = destNodeA;
    }

}

int OSCNode::getPath()
{
    return activePath;
}

//void OSCNode::process(AudioSampleBuffer&, MidiBuffer& events)
//{
//    int bytesAvailable = 0;

//    if (bytesAvailable == OF_SERIAL_ERROR)
//    {
//        // ToDo: Properly warn about problem here!
//        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "SerialInput device access error!", "Could not access serial device.");
//        return;
//    }

//    if (bytesAvailable > 0)
//    {

//        unsigned char buffer[10000];
//        int bytesRead = serial.readBytes(buffer, bytesAvailable);

//        if (bytesRead > 0)
//        {
//            addEvent(events,    // MidiBuffer
//                     BINARY_MSG,    // eventType
//                     0,         // sampleNum
//                     nodeId,    // eventID
//                     0,         // eventChannel
//                     bytesRead, // numBytes
//                     buffer);   // data
//        }
//        else if (bytesRead < 0)
//        {
//            // ToDo: Properly warn about problem here!
//            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "SerialInput device read error!", "Could not read serial input, even though data should be available.");
//            return;
//        }
//    }
//}
