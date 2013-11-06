/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2013 Open Ephys

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
#include "Editors/NetworkEventsEditor.h"

const int MAX_MESSAGE_LENGTH = 32000;



NetworkEvents::NetworkEvents(void *zmq_context)
	:Thread("NetworkThread"), GenericProcessor("Network Events"), threshold(200.0), bufferZone(5.0f), state(false)

{
	zmqcontext = zmq_context;
	/*
	void *p2 = zmqcontext;
	void * p3;
	memcpy(&p3,&zmqcontext, sizeof(void*));

	memcpy(zmqcontext,zmq_context,sizeof(void*));*/
	// Create a socket to handle communication
	opensocket();
	//parameters.add(Parameter("thresh", 0.0, 500.0, 200.0, 0));

}

NetworkEvents::~NetworkEvents()
{
	//			sd->thread_running = false;
	//		Sleep(1000);


}


AudioProcessorEditor* NetworkEvents::createEditor()
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
    if (eventType == TIMESTAMP)
    {
          const uint8* dataptr = event.getRawData();
		  int64 hardware_timestamp;
		  int64 software_timestamp;
	      memcpy(&hardware_timestamp, dataptr + 4, 8); // remember to skip first four bytes
		  memcpy(&software_timestamp, dataptr + 12, 8); // remember to skip first four bytes
    } 
}

void NetworkEvents::process(AudioSampleBuffer& buffer,
							MidiBuffer& events,
							int& nSamples)
{
	
	//std::cout << *buffer.getSampleData(0, 0) << std::endl;
	checkForEvents(events);
	 while (!networkMessagesQueue.empty()) {

			 StringTS msg = networkMessagesQueue.front();
			 // Handle incoming messages.
			 // If this a special message, act accordingly (start/sto recording, etc).
			 // otherwise, just put it in the midi message queue
			 uint8* msg_with_ts = msg.packWithTS();
			 addEvent(events, NETWORK,0,0,GENERIC_EVENT,msg.len+8,msg_with_ts);
			 delete msg_with_ts;
		     networkMessagesQueue.pop();
	 }
	
}


void NetworkEvents::opensocket()
{
	startThread();
}

void NetworkEvents::run() {
  void *responder = zmq_socket (zmqcontext, ZMQ_REP);
  int rc = zmq_bind (responder, "tcp://*:5556");
  if (rc != 0) {
	  // failed to open socket?
	  return;
  }
  threadRunning = true;
	 unsigned char *buffer = new unsigned char[MAX_MESSAGE_LENGTH];

	while (threadRunning) {
         int result = zmq_recv (responder, buffer, MAX_MESSAGE_LENGTH-1, 0); // blocking
         juce::int64 timestamp = timer.getHighResolutionTicks();

		 if (result < 0)
			return;
		StringTS Msg(buffer, result, timestamp);
		networkMessagesQueue.push(Msg);
	     
        zmq_send (responder, "OK", 2, 0);
    }
	delete buffer;
	threadRunning = false;
    return;
}


