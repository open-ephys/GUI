/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2015 Open Ephys

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

#include <stdio.h>
#include "NetworkEvents.h"
#include "../Editors/NetworkEventsEditor.h"
#include "../../AccessClass.h"
#include "../MessageCenter/MessageCenterEditor.h"


StringTS::StringTS()
{

    str = nullptr;
    len= 0;
    timestamp = 0;
}


std::vector<String> StringTS::splitString(char sep)
{
    String S((const char*)str,len);
    std::list<String> ls;
    String  curr;
    for (int k=0; k < S.length(); k++)
    {
        if (S[k] != sep)
        {
            curr+=S[k];
        }
        else
        {
            ls.push_back(curr);
            while (S[k] == sep && k < S.length())
                k++;

            curr = "";
            if (S[k] != sep && k < S.length())
                curr+=S[k];
        }
    }
    if (S.length() > 0)
    {
        if (S[S.length()-1] != sep)
            ls.push_back(curr);
    }
    std::vector<String> Svec(ls.begin(), ls.end());
    return Svec;

}

StringTS::StringTS(MidiMessage& event)
{
    const uint8* dataptr = event.getRawData();
    int bufferSize = event.getRawDataSize();
    len = bufferSize-6-8; // -6 for initial event prefix, -8 for timestamp at the end

    memcpy(&timestamp, dataptr + 6+ len, 8); // remember to skip first six bytes
    str = new uint8[len];
    memcpy(str,dataptr + 6, len);
}

StringTS& StringTS::operator=(const StringTS& rhs)
{
    delete(str);
    len = rhs.len;
    str = new uint8[len];
    memcpy(str,rhs.str,len);
    timestamp = rhs.timestamp;

    return *this;
}

String StringTS::getString()
{

    return String((const char*)str,len);
}

StringTS::StringTS(String S)
{
    Time t;
    str = new uint8[S.length()];
    memcpy(str,S.toRawUTF8(),S.length());
    timestamp = t.getHighResolutionTicks();

    len = S.length();
}

StringTS::StringTS(String S, int64 ts_software)
{
    str = new uint8[S.length()];
    memcpy(str,S.toRawUTF8(),S.length());
    timestamp = ts_software;

    len = S.length();
}

StringTS::StringTS(const StringTS& s)
{
    str = new uint8[s.len];
    memcpy(str,s.str,s.len);
    timestamp = s.timestamp;
    len = s.len;
}


StringTS::StringTS(unsigned char* buf, int _len, int64 ts_software) : len(_len),timestamp(ts_software)
{
    str = new juce::uint8[len];
    for (int k=0; k<len; k++)
        str[k] = buf[k];
}

StringTS::~StringTS()
{
    delete str;
}

/*********************************************/


std::shared_ptr<void> NetworkEvents::getZMQContext() {
    // Note: C++11 guarantees that initialization of static local variables occurs exactly once, even
    // if multiple threads attempt to initialize the same static local variable concurrently.
#ifdef ZEROMQ
    static const std::shared_ptr<void> ctx(zmq_ctx_new(), zmq_ctx_destroy);
#else
    static const std::shared_ptr<void> ctx;
#endif
    return ctx;
}


NetworkEvents::NetworkEvents()
    : GenericProcessor("Network Events"), Thread("NetworkThread"), zmqcontext(getZMQContext()), threshold(200.0), bufferZone(5.0f), state(false)

{
    firstTime = true;
    setNewListeningPort(5556);

    sendSampleCount = false; // disable updating the continuous buffer sample counts,
    // since this processor only sends events
}

void NetworkEvents::setNewListeningPort(int port)
{
    // first, close existing thread.
    closesocket();

    urlport = port;
    startThread();
    
    // Wait for thread startup to complete
    if (wait(5000)) {
        std::cout << "Network node started" << std::endl;
    } else {
        std::cout << "Network node failed to start" << std::endl;
    }
}

NetworkEvents::~NetworkEvents()
{
    closesocket();
}

bool NetworkEvents::closesocket()
{
    if (isThreadRunning()) {
        std::cout << "Disabling network node" << std::endl;
        stopThread(5000);  // 5-second timeout
        std::cout << "Network node stopped" << std::endl;
    }
    return true;
}

int NetworkEvents::getNumEventChannels()
{
    return 1;
}

void NetworkEvents::updateSettings()
{
    eventChannels[0]->type = MESSAGE_CHANNEL; // so it's ignored by LFP Viewer
}

AudioProcessorEditor* NetworkEvents::createEditor(
)
{
    editor = new NetworkEventsEditor(this, true);

    return editor;


}


void NetworkEvents::setParameter(int parameterIndex, float newValue)
{
    /*
    	editor->updateParameterButtons(parameterIndex);

    	Parameter& p =  parameters.getReference(parameterIndex);
    	p.setValue(newValue, 0);

    	threshold = newValue;
    	*/
    //std::cout << float(p[0]) << std::endl;

}


void NetworkEvents::handleEvent(int eventType, juce::MidiMessage& event, int samplePosition)
{

}

void NetworkEvents::postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events)
{
    uint8* msg_with_ts = new uint8[s.len+1];//+8]; // for the two timestamps
    memcpy(msg_with_ts, s.str, s.len);
    *(msg_with_ts + s.len) = '\0';
    //memcpy(msg_with_ts+s.len, &s.timestamp, 8);

    addEvent(events,
             (uint8) MESSAGE,
             0,
             1,
             0,
             (uint8) s.len+1,//+8,
             msg_with_ts);

    delete msg_with_ts;
}


String NetworkEvents::handleSpecialMessages(StringTS msg)
{
    /*
    std::vector<String> input = msg.splitString(' ');
    if (input[0] == "StartRecord")
    {
    	 getUIComponent()->getLogWindow()->addLineToLog("Remote triggered start recording");

    	if (input.size() > 1)
    	{
    		getUIComponent()->getLogWindow()->addLineToLog("Remote setting session name to "+input[1]);
    		// session name was also given.
    		getProcessorGraph()->getRecordNode()->setDirectoryName(input[1]);
    	}
        const MessageManagerLock mmLock;
    	getControlPanel()->recordButton->setToggleState(true,true);
    	return String("OK");
    //	getControlPanel()->placeMessageInQueue("StartRecord");
    } if (input[0] == "SetSessionName")
    {
    		getProcessorGraph()->getRecordNode()->setDirectoryName(input[1]);
    } else if (input[0] == "StopRecord")
    {
    	const MessageManagerLock mmLock;
    	//getControlPanel()->placeMessageInQueue("StopRecord");
    	getControlPanel()->recordButton->setToggleState(false,true);
    	return String("OK");
    } else if (input[0] == "ProcessorCommunication")
    {
    	ProcessorGraph *g = getProcessorGraph();
    	Array<GenericProcessor*> p = g->getListOfProcessors();
    	for (int k=0;k<p.size();k++)
    	{
    		if (p[k]->getName().toLowerCase() == input[1].toLowerCase())
    		{
    			String Query="";
    			for (int i=2;i<input.size();i++)
    			{
    				if (i == input.size()-1)
    					Query+=input[i];
    				else
    					Query+=input[i]+" ";
    			}

    			return p[k]->interProcessorCommunication(Query);
    		}
    	}

    	return String("OK");
    }

    */
    return String("NotHandled");
}

void NetworkEvents::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events)
{

    //std::cout << "NETWORK NODE" << std::endl;
    //printf("Entering NetworkEvents::process\n");
    setTimestamp(events,CoreServices::getGlobalTimestamp());
    checkForEvents(events);

    //std::cout << *buffer.getSampleData(0, 0) << std::endl;
    lock.enter();
    while (!networkMessagesQueue.empty())
    {
        StringTS msg = networkMessagesQueue.front();
        postTimestamppedStringToMidiBuffer(msg, events);
        CoreServices::sendStatusMessage("Network event received: " + msg.getString());
        //			 getUIComponent()->getLogWindow()->addLineToLog(msg);
        networkMessagesQueue.pop();
    }
    lock.exit();

}


void NetworkEvents::run()
{

#ifdef ZEROMQ
    const std::unique_ptr<void, int(*)(void*)> responder(zmq_socket(zmqcontext.get(), ZMQ_REP), zmq_close);
    if (!responder) {
        std::cout << "Failed to create socket: " << zmq_strerror(zmq_errno()) << std::endl;
        return;
    }
    
    // Set receive timeout to one second
    const int recvTimeout = 1000;
    if (0 != zmq_setsockopt(responder.get(), ZMQ_RCVTIMEO, &recvTimeout, sizeof(recvTimeout))) {
        std::cout << "Failed to set socket receive timeout: " << zmq_strerror(zmq_errno()) << std::endl;
        return;
    }
    
    String url= String("tcp://*:")+String(urlport);
    if (0 != zmq_bind(responder.get(), url.toRawUTF8()))
    {
        // failed to open socket?
        std::cout << "Failed to open socket: " << zmq_strerror(zmq_errno()) << std::endl;
        return;
    }

    constexpr int MAX_MESSAGE_LENGTH = 64000;
    std::vector<unsigned char> buffer(MAX_MESSAGE_LENGTH);
    notify();  // Signal thread waiting in setNewListeningPort

    while (!threadShouldExit())
    {

        int result = zmq_recv(responder.get(), buffer.data(), MAX_MESSAGE_LENGTH-1, 0);  // blocking with timeout

        juce::int64 timestamp_software = timer.getHighResolutionTicks();

        if (result < 0) {
            if (zmq_errno() != EAGAIN) {
                std::cout << "Socket receive failed: " << zmq_strerror(zmq_errno()) << std::endl;
            }
            continue;
        }

        StringTS Msg(buffer.data(), result, timestamp_software);
        if (result > 0)
        {
            lock.enter();
            networkMessagesQueue.push(Msg);
            lock.exit();

            //std::cout << "Received message!" << std::endl;
            // handle special messages
            String response = handleSpecialMessages(Msg);

            zmq_send(responder.get(), response.getCharPointer(), response.length(), 0);
        }
        else
        {
            String zeroMessageError = "Recieved Zero Message?!?!?";
            //std::cout << "Received Zero Message!" << std::endl;

            zmq_send(responder.get(), zeroMessageError.getCharPointer(), zeroMessageError.length(), 0);
        }
    }
#endif
    
}






bool NetworkEvents::isReady()
{

    return true;

}


float NetworkEvents::getDefaultSampleRate()
{
    return 30000.0f;
}

int NetworkEvents::getDefaultNumOutputs()
{
    return 0;
}

float NetworkEvents::getDefaultBitVolts()
{
    return 0.05f;
}

void NetworkEvents::enabledState(bool t)
{

    isEnabled = t;

}

bool NetworkEvents::isSource()
{
    return true;
}

void NetworkEvents::saveCustomParametersToXml(XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("NETWORKEVENTS");
    mainNode->setAttribute("port", urlport);
}


void NetworkEvents::loadCustomParametersFromXml()
{

    if (parametersAsXml != nullptr)
    {
        forEachXmlChildElement(*parametersAsXml, mainNode)
        {
            if (mainNode->hasTagName("NETWORKEVENTS"))
            {
                setNewListeningPort(mainNode->getIntAttribute("port"));
            }
        }
    }
}
