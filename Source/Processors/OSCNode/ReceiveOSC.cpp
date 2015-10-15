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

ReceiveOSC::ReceiveOSC(OSCNode *node)
	: Thread("OscListener Thread")
    , incomingPort(PORT)
    , s(IpEndpointName("localhost",
                     incomingPort), this)
	, processor(node)
{

    DBG("Now called the Constructor");
}


void ReceiveOSC::ProcessMessage(const osc::ReceivedMessage& m,
                                const IpEndpointName& remoteEndpoint)
{
    DBG("Process message!");
    DBG(m.AddressPattern());
    (void) remoteEndpoint; // suppress unused parameter warning
    try{
        // example of parsing single messages. osc::OsckPacketListener
        // handles the bundle traversal.
        if( std::strcmp( m.AddressPattern(), "/debug" ) == 0 ) {
            // example #1 -- argument stream interface
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            osc::int32 intMessage;
            float floatMessage;
            float floatMessage2;
            // const char *a4;
			args >> floatMessage >> floatMessage2 >> osc::EndMessage;
            // args >>  intMessage >> floatMessage >> a4 >> osc::EndMessage;
            DBG("received message with arguments: " << floatMessage << " " << floatMessage2);

			// Check that numbers are valid numbers and not nans or infs and stuff
			if(floatMessage == floatMessage && floatMessage2 == floatMessage2) {
				processor->receivePosition(floatMessage, floatMessage2);
			}
            //                        customOSCCall.setFloatTextField(floatMessage);

            // MainContentComponent::setIntTextField(intMessage);
        }
    }catch( osc::Exception& e ){
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
