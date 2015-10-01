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

ReceiveOSC::ReceiveOSC():
    Thread("OscListener Thread"),
    incomingPort(PORT),
    s(IpEndpointName("localhost",
                     incomingPort), this)
{

    DBG("Now called the Constructor");
}


void ReceiveOSC::ProcessMessage(const osc::ReceivedMessage& m,
                                const IpEndpointName& remoteEndpoint)
{
    DBG("Process message!");
    DBG(m.AddressPattern());
//    DBG(m.ArgumentStream());
    (void) remoteEndpoint; // suppress unused parameter warning
    try{
        // example of parsing single messages. osc::OsckPacketListener
        // handles the bundle traversal.
        if( std::strcmp( m.AddressPattern(), "/a_float" ) == 0 ) {
            // example #1 -- argument stream interface
            osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
            osc::int32 intMessage;
            float floatMessage;
            // const char *a4;
            args >> floatMessage >> osc::EndMessage;
            // args >>  intMessage >> floatMessage >> a4 >> osc::EndMessage;
            DBG("received '/test1' message with arguments: " << intMessage << " " << floatMessage);

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
