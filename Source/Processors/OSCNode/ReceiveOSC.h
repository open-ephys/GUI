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
#include <oscpack/osc/OscOutboundPacketStream.h>
#include <oscpack/ip/IpEndpointName.h>
// OSC receive includes
#include <oscpack/osc/OscReceivedElements.h>
#include <oscpack/osc/OscPacketListener.h>
// OSC shared includes
#include <oscpack/ip/UdpSocket.h>
#include <unordered_map>
#include <memory>
class OSCNode;

//#define PORT 5005
class ReceiveOSC: public osc::OscPacketListener,
        public Thread
{
public:
    ReceiveOSC(int port);
    ReceiveOSC(ReceiveOSC const&) = delete;
    void operator=(ReceiveOSC const&) = delete;
    ~ReceiveOSC();

    static std::shared_ptr<ReceiveOSC> getInstance(int port, bool justDelete = false) {
        // TODO Handle case where port cannot be assigned
        static std::unordered_map<int, std::shared_ptr<ReceiveOSC>> instances;

        std::vector<int> toDelete;
        for(auto r : instances) {
            if(r.first != port && r.second->processors.size() < 1) {
                toDelete.push_back(r.first);
            }
        }
        for(auto port : toDelete) {
            instances.erase(port);
        }
        if(justDelete) {
            // the function was invoked only to delete stale instances
            return nullptr;
        }
        if(instances.count(port) < 1) {
            instances[port] = std::make_shared<ReceiveOSC>(port);
        }
        if(!instances[port]->isThreadRunning()) {
            instances[port]->startThread();
        }
        return instances[port];
    }

    // Start the oscpack OSC Listener Thread
    // NOTE: s.Run() won't return unless we force it to with
    // s.AsynchronousBreak() as is done in the destructor
    void run()
    {
        DBG("Running thread");
        s.Run();
    }

    // getters
    int getIntOSC();
    float getFloatOSC();
    void addProcessor(OSCNode *processor);
    void removeProcessor(OSCNode *processor);
private:



    int incomingPort;
    UdpListeningReceiveSocket s;
    std::vector<OSCNode*> processors;
    String m_address;

protected:
    //this is our main processing function
    //overwrite to to do other things with incoming osc messages
    virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint);
};


#endif  // RECEIVEOSC_H_INCLUDED
