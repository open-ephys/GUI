/*
  ==============================================================================

    EventBroadcaster.h
    Created: 22 May 2015 3:31:50pm
    Author:  Christopher Stawarz

  ==============================================================================
*/

#ifndef EVENTBROADCASTER_H_INCLUDED
#define EVENTBROADCASTER_H_INCLUDED

#include "../GenericProcessor/GenericProcessor.h"

#ifdef ZEROMQ

#ifdef WIN32
#include "../../../Resources/windows-libs/ZeroMQ/include/zmq.h"
#include "../../../Resources/windows-libs/ZeroMQ/include/zmq_utils.h"
#else
#include <zmq.h>
#endif

#endif


class EventBroadcaster : public GenericProcessor
{
public:
    EventBroadcaster();

    AudioProcessorEditor* createEditor() override;

    int getListeningPort() const;
    void setListeningPort(int port, bool forceRestart = false);

    void process(AudioSampleBuffer& continuousBuffer, MidiBuffer& eventBuffer) override;
    bool isSink() override;
    void handleEvent(int eventType, MidiMessage& event, int samplePosition = 0) override;

    void saveCustomParametersToXml(XmlElement* parentElement) override;
    void loadCustomParametersFromXml() override;

private:
    static std::shared_ptr<void> getZMQContext();
    static void closeZMQSocket(void* socket);
    
    const std::shared_ptr<void> zmqContext;
    std::unique_ptr<void, decltype(&closeZMQSocket)> zmqSocket;
    int listeningPort;
    
    float currentSampleRate;

};


#endif  // EVENTBROADCASTER_H_INCLUDED
