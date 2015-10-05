/*
  ==============================================================================

    ReceiveOSC.h
    Created: 1 Oct 2015 11:16:33am
    Author:  mikkel

  ==============================================================================
*/

#ifndef RECEIVEOSC_H_INCLUDED
#define RECEIVEOSC_H_INCLUDED

#include "../../JuceLibraryCode/JuceHeader.h"

// OSC send includes
#include "osc/OscOutboundPacketStream.h"
#include "ip/IpEndpointName.h"
// OSC receive includes
#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
// OSC shared includes
#include "ip/UdpSocket.h"
class OSCNode;

#define PORT 7000
class ReceiveOSC: public osc::OscPacketListener,
        public Thread
{
public:
    // Constructor
    ReceiveOSC(OSCNode* node);//, MainContentComponent* const owner); //from mlrVSTAudioProcessor * const owner);
    ~ReceiveOSC()
    {
        // stop the OSC Listener thread running
        s.AsynchronousBreak();

        // allow the thread 2 seconds to stop cleanly - should be plenty of time.
        stopThread(2000);
    }

    // Start the oscpack OSC Listener Thread
    // NOTE: s.Run() won't return unless we force it to with
    // s.AsynchronousBreak() as is done in the destructor
    void run()
    {
        DBG("calling run");
        s.Run();
    }

    // getters
    int getIntOSC();
    float getFloatOSC();

private:
    int incomingPort;
    UdpListeningReceiveSocket s;
	OSCNode* processor;

protected:
    //this is our main processing function
    //overwrite to to do other things with incoming osc messages
    virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint);
};


#endif  // RECEIVEOSC_H_INCLUDED
