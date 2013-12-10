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

#include "NetworkSink.h"
#include "NetworkSinkProcotol.pb.h"
#include "Channel.h"

#include <stdio.h>


NetworkSinkNode::NetworkSinkNode(void * zmq_context)
	: GenericProcessor("Network Sink")
{
	zmqcontext = zmq_context;
	publisher = nullptr;
	urlport = 5557;
	opensocket();

}

bool NetworkSinkNode::opensocket()
{
	publisher = zmq_socket (zmqcontext, ZMQ_REP);
	String url= String("tcp://*:")+String(urlport);
	int rc = zmq_bind (publisher, url.toRawUTF8());
	return rc == 0;
}

NetworkSinkNode::~NetworkSinkNode()
{
	if (publisher!= nullptr)
		zmq_close(publisher);

}


/*
AudioProcessorEditor* NetworkSinkNode::createEditor()
{
editor = new NetworkSinkEditor(this,true);
return editor;

}*/

void NetworkSinkNode::updateSettings()
{

}


bool NetworkSinkNode::enable()
{

	std::cout << "NetworkSinkNode::enable()" << std::endl;
	NetworkSinkEditor* editor = (NetworkSinkEditor*) getEditor();
	editor->enable();

	return true;

}

bool NetworkSinkNode::disable()
{

	std::cout << "NetworkSinkNode disabled!" << std::endl;
	NetworkSinkEditor* editor = (NetworkSinkEditor*) getEditor();
	editor->disable();

	return true;
}

void NetworkSinkNode::process(AudioSampleBuffer& buffer, MidiBuffer& events, int& nSamples)
{

	// Update internal statistics 
	checkForEvents(events); 


}


void NetworkSinkNode::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{
	if (publisher != nullptr)
	{

		if (eventType == NETWORK)
		{

		}
		if (eventType == TIMESTAMP)
		{


		} 
		if (eventType == TTL)
		{

		}

		if (eventType == SPIKE)
		{

			const uint8_t* dataptr = event.getRawData();
			int bufferSize = event.getRawDataSize();
			if (bufferSize > 0)
			{
				SpikeObject newSpike;
				unpackSpike(&newSpike, dataptr, bufferSize);

				// create a google buffer protocol object and send it away.
				OpenEphys::Spike sp;
				sp.set_timestamp(newSpike.timestamp);
				sp.set_timestamp_software(newSpike.timestamp_software);
				sp.set_source(newSpike.source);
				sp.set_nchannels(newSpike.nChannels);
				sp.set_nsamples(newSpike.nSamples);
				sp.set_sortedid(newSpike.sortedId);

				sp.set_electrodeid(newSpike.electrodeID);
				sp.add_color(newSpike.color[0]);
				sp.add_color(newSpike.color[1]);
				sp.add_color(newSpike.color[2]);

				sp.add_pcproj(newSpike.pcProj[0]);
				sp.add_pcproj(newSpike.pcProj[1]);

				sp.set_samplingfrequencyhz(newSpike.samplingFrequencyHz);

				for (int ch=0;ch<newSpike.nChannels;ch++)
				{
					sp.add_gain(newSpike.gain[ch]);
					sp.add_gain(newSpike.threshold[ch]);
					for (int i=0;i<newSpike.nSamples;i++)
					{
						sp.add_data(newSpike.data[ch*newSpike.nSamples+i]);
					}
				}


				std::string s = sp.SerializeAsString();
				//zmq_send(socket, s.c_str(), s.length(), 0); // 5
			}
		}
	}
}


