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
    : GenericProcessor("OSCNode")
	, timestamp(0)
	, previousEventTime(0)
    , eventId(0)
	, m_positionIsUpdated(false)
	, m_port(27020)
{
    sendSampleCount = false;
	try {
		ReceiveOSC::getInstance(m_port)->addProcessor(this);
	} catch(std::runtime_error) {
		DBG("Unable to bind port");
	}
}

OSCNode::~OSCNode()
{
    ReceiveOSC::getInstance(m_port)->removeProcessor(this);
    ReceiveOSC::getInstance(0, true);
}

AudioProcessorEditor* OSCNode::createEditor()
{
    editor = new OSCEditor(this, true);
    return editor;
}

bool OSCNode::isSource()
{
    return true;
}

int OSCNode::getNumEventChannels()
{
    return 1;
}

void OSCNode::updateSettings()
{
    eventChannels[0]->type = EVENT_CHANNEL;
}

void OSCNode::setAddress(String address)
{
    m_address = address;
}

String OSCNode::address()
{
    return m_address;
}

void OSCNode::setPort(int port)
{
	try{
    ReceiveOSC::getInstance(m_port)->removeProcessor(this);
    m_port = port;
    ReceiveOSC::getInstance(port)->addProcessor(this);
	} catch(std::runtime_error){
		DBG("Unable to bind port");
	}
}

int OSCNode::port()
{
    return m_port;
}

void OSCNode::process(AudioSampleBuffer& buffer, MidiBuffer& events) 
{    
    setTimestamp(events,CoreServices::getGlobalTimestamp());
    checkForEvents(events);

//    int samplesNeeded = (int) float(buffer.getNumSamples()) * (getDefaultSampleRate()/44100.0f);

    //std::cout << *buffer.getSampleData(0, 0) << std::endl;
    lock.enter();

    int argumentCount = m_message.size();

    if(m_positionIsUpdated) {
        addEvent(events, // MidiBuffer
                 BINARY_MSG,    // eventType
                 0,      // sampleNum
                 eventId,	     // eventID
                 0,		 // eventChannel
                 sizeof(float)*argumentCount,
                 (uint8*)&(m_message[0])
                 );
//        previousEventTime = timestamp;
        m_positionIsUpdated = false;
    }
//    timestamp += samplesNeeded;
//    setNumSamples(events, samplesNeeded);
    lock.exit();
}

void OSCNode::receiveMessage(std::vector<float> message)
{
    m_positionIsUpdated = true;
    m_message = message;
}

bool OSCNode::isReady()
{
    return true;
}

