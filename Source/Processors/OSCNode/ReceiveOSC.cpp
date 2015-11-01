/*
  ==============================================================================

    ReceiveOSC.cpp
    Created: 1 Oct 2015 11:16:33am
    Author:  mikkel

  ==============================================================================
*/

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "ReceiveOSC.h"
#include "OSCEditor.h"
#include "OSCNode.h"

void ReceiveOSC::ProcessMessage(const osc::ReceivedMessage& m,
                                const IpEndpointName& remoteEndpoint)
{
    (void) remoteEndpoint; // suppress unused parameter warning
    DBG("Received message!");
    DBG(m.AddressPattern());
    try{
        // example of parsing single messages. osc::OsckPacketListener
        // handles the bundle traversal.
        for(OSCNode* processor : processors) {
            String address = processor->address();
            if( std::strcmp( m.AddressPattern(), address.toStdString().c_str() ) == 0 ) {
                // example #1 -- argument stream interface
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
                float floatMessage;
                float floatMessage2;
                args >> floatMessage >> floatMessage2 >> osc::EndMessage;
                DBG("received message with arguments: " << floatMessage << " " << floatMessage2);

                // Check that numbers are valid numbers and not nans or infs and stuff
                if(floatMessage == floatMessage && floatMessage2 == floatMessage2) {
                    processor->receivePosition(floatMessage, floatMessage2);
                }
            }
        }
    } catch( osc::Exception& e ){
        // any parsing errors such as unexpected argument types, or
        // missing arguments get thrown as exceptions.
        DBG("error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n");
    }
}

int ReceiveOSC::getIntOSC()
{
    int test = 1;
    return test;
}

float ReceiveOSC::getFloatOSC()
{
    float test = 2.19;
    return test;
}

void ReceiveOSC::addProcessor(OSCNode *processor)
{
    processors.push_back(processor);
}

void ReceiveOSC::removeProcessor(OSCNode *processor)
{
    processors.erase(std::remove(processors.begin(), processors.end(), processor), processors.end());
}

ReceiveOSC::ReceiveOSC(int port)
    : Thread("OscListener Thread")
    , incomingPort(port)
    , s(IpEndpointName("localhost",
                       incomingPort), this)
{}

ReceiveOSC::~ReceiveOSC()
{
    // stop the OSC Listener thread running
    s.AsynchronousBreak();

    // allow the thread 2 seconds to stop cleanly - should be plenty of time.
    stopThread(2000);
}
