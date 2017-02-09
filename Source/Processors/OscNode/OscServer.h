/*
  ==============================================================================

    OscServer.h
    Created: 1 Oct 2015 11:16:33am
    Author:  mikkel

  ==============================================================================
*/

#ifndef OSCSERVER_H_INCLUDED
#define OSCSERVER_H_INCLUDED

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
class OscNode;

//#define PORT 5005
class OscServer: public osc::OscPacketListener,
        public Thread
{
public:
    OscServer(int port);
    ~OscServer();

    static std::shared_ptr<OscServer> getInstance(int port, bool justDelete = false) {
        // TODO Handle case where port cannot be assigned
        static std::unordered_map<int, std::shared_ptr<OscServer>> instances;

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
            try {
                instances[port] = std::make_shared<OscServer>(port);
            }
            catch (std::runtime_error &e) {
                DBG("Error unable to bind port:");
                DBG(port);
            }
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
    void addProcessor(OscNode *processor);
    void removeProcessor(OscNode *processor);
private:
    OscServer(OscServer const&);
    void operator=(OscServer const&);

    int incomingPort;
    UdpListeningReceiveSocket s;
    std::vector<OscNode*> processors;
    String m_address;

protected:
    //this is our main processing function
    //overwrite to to do other things with incoming osc messages
    virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint);
};


#endif  // OSCSERVER_H_INCLUDED
