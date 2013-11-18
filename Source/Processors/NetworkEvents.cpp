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
#include "../UI/UIComponent.h"
const int MAX_MESSAGE_LENGTH = 32000;



NetworkEvents::NetworkEvents(void *zmq_context)
	:Thread("NetworkThread"), GenericProcessor("Network Events"), threshold(200.0), bufferZone(5.0f), state(false)

{
	zmqcontext = zmq_context;
	firstTime = true;

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

void NetworkEvents::initSimulation()
{
	Time t;

	int64 secondsToTicks = t.getHighResolutionTicksPerSecond();
	simulationStartTime=3*secondsToTicks + t.getHighResolutionTicks(); // start 10 seconds after
		
	simulation.push(StringTS("ClearDesign",simulationStartTime));
	simulation.push(StringTS("NewDesign Test",simulationStartTime+0.5*secondsToTicks));
	simulation.push(StringTS("AddCondition Name GoRight TrialTypes 1 2 3",simulationStartTime+0.6*secondsToTicks));
	simulation.push(StringTS("AddCondition Name GoLeft TrialTypes 4 5 6",simulationStartTime+0.6*secondsToTicks));


	int numTrials = 100;
	// trial every 5 seconds
	for (int k=0;k<numTrials;k++) {
		simulation.push(StringTS("TrialStart",simulationStartTime+5*k*secondsToTicks));
		if (k%2 == 0) 
			simulation.push(StringTS("TrialType 2",simulationStartTime+(5*k+0.1)*secondsToTicks)); // 100 ms after trial start
		else
			simulation.push(StringTS("TrialType 4",simulationStartTime+(5*k+0.1)*secondsToTicks)); // 100 ms after trial start

		simulation.push(StringTS("TrialAlign",simulationStartTime+(5*k+0.1)*secondsToTicks)); // 100 ms after trial start
		simulation.push(StringTS("TrialOutcome 1",simulationStartTime+(5*k+0.3)*secondsToTicks)); // 300 ms after trial start
		simulation.push(StringTS("TrialEnd",simulationStartTime+(5*k+0.4)*secondsToTicks)); // 400 ms after trial start

	}
	
}

void NetworkEvents::simulateDesignAndTrials(juce::MidiBuffer& events)
{
	Time t;
	while (simulation.size() > 0) 
	{
		int64 currenttime = t.getHighResolutionTicks();
		StringTS S = simulation.front();
		if (currenttime > S.timestamp) {
			 postTimestamppedStringToMidiBuffer(S,events);
			 getUIComponent()->getLogWindow()->addLineToLog(S.getString());
			simulation.pop();
		} else
			break;
	}

}


void NetworkEvents::handleEvent(int eventType, juce::MidiMessage& event, int samplePosition)
{
    if (eventType == TIMESTAMP)
    {
          const uint8* dataptr = event.getRawData();
	      memcpy(&hardware_timestamp, dataptr + 4, 8); // remember to skip first four bytes
		  memcpy(&software_timestamp, dataptr + 12, 8); // remember to skip first four bytes
    } 
}

void NetworkEvents::postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events)
{
	uint8* msg_with_ts = new uint8[s.len+8]; // for the two timestamps
	memcpy(msg_with_ts, s.str, s.len);	
	memcpy(msg_with_ts+s.len, &s.timestamp, 8);
	addEvent(events, NETWORK,0,0,GENERIC_EVENT,s.len+8,msg_with_ts);
	delete msg_with_ts;
}

void NetworkEvents::handleSpecialMessages(StringTS msg)
{
	if (msg.getString() == "StartRecord")
	{
		getControlPanel()->startRecording();

		getProcessorGraph()->setRecordState(true);

	} else if (msg.getString() == "StopRecord")
	{
		getControlPanel()->stopRecording();
		getProcessorGraph()->setRecordState(false);
	}

}

void NetworkEvents::process(AudioSampleBuffer& buffer,
							MidiBuffer& events,
							int& nSamples)
{
	checkForEvents(events);
	

	if (firstTime) {
		firstTime = false;
		initSimulation();
	}
	simulateDesignAndTrials(events);

	//std::cout << *buffer.getSampleData(0, 0) << std::endl;
	
	 while (!networkMessagesQueue.empty()) {
			 StringTS msg = networkMessagesQueue.front();

			 // handle special messages
			 handleSpecialMessages(msg);

			 postTimestamppedStringToMidiBuffer(msg, events);
			 getUIComponent()->getLogWindow()->addLineToLog(msg);
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
         juce::int64 timestamp_software = timer.getHighResolutionTicks();
		
		 if (result < 0) // will only happen when responder dies.
			return;

		StringTS Msg(buffer, result, timestamp_software);
		networkMessagesQueue.push(Msg);
	     
        zmq_send (responder, "OK", 2, 0);
    }
	delete buffer;
	threadRunning = false;
    return;
}


