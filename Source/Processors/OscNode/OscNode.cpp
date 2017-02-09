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

#include "OscNode.h"
#include "OscEditor.h"

#include "../../UI/EditorViewport.h"

OscNode::OscNode()
    : GenericProcessor("OSC Port")
	, timestamp(0)
	, previousEventTime(0)
    , eventId(0)
	, m_positionIsUpdated(false)
	, m_port(27020)
{
    sendSampleCount = false;
	try {
        OscServer::getInstance(m_port)->addProcessor(this);
	} catch(std::runtime_error) {
		DBG("Unable to bind port");
	}
}

OscNode::~OscNode()
{
    OscServer::getInstance(m_port)->removeProcessor(this);
    OscServer::getInstance(0, true);
}

AudioProcessorEditor* OscNode::createEditor()
{
    editor = new OscEditor(this, true);
    return editor;
}

bool OscNode::isSource()
{
    return true;
}

int OscNode::getNumEventChannels()
{
    return 1;
}

void OscNode::updateSettings()
{
    eventChannels[0]->type = EVENT_CHANNEL;
}

void OscNode::setAddress(String address)
{
    m_address = address;
}

String OscNode::address()
{
    return m_address;
}

void OscNode::setPort(int port)
{
	try{
    OscServer::getInstance(m_port)->removeProcessor(this);
    m_port = port;
    OscServer::getInstance(port)->addProcessor(this);
	} catch(std::runtime_error){
		DBG("Unable to bind port");
	}
}

int OscNode::port()
{
    return m_port;
}

void OscNode::process(AudioSampleBuffer& buffer, MidiBuffer& events)
{    
    setTimestamp(events,CoreServices::getGlobalTimestamp());
    checkForEvents(events);

    //std::cout << *buffer.getSampleData(0, 0) << std::endl;
    lock.enter();

	// ADD POSITION string in the message
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
        m_positionIsUpdated = false;
    }
    lock.exit();
}

void OscNode::receiveMessage(std::vector<float> message)
{
    m_positionIsUpdated = true;
    m_message = message;
}

bool OscNode::isReady()
{
    return true;
}

