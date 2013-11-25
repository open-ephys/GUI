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
#include "AdvancerNode.h"
#include "Editors/AdvancerEditor.h"
#include "../UI/UIComponent.h"


AdvancerNode::AdvancerNode()
	:GenericProcessor("Advancers")

{

}

Polygon2D::Polygon2D()
{
}

Polygon2D::Polygon2D(int N)
{
	points.resize(N);
}

Polygon2D AdvancerNode::createCircle(float diameterMM)
{
	int N = 20;
	Polygon2D p(N+1);
	for (int k=0;k<N;k++)
	{
		p.points[k].x= cos(float(k)/(N-1) * 2.0 * 3.1415)*diameterMM;
		p.points[k].y= sin(float(k)/(N-1) * 2.0 * 3.1415)*diameterMM;
	}
	p.points[N].x = p.points[0].x;
	p.points[N].y = p.points[0].y;
	return p;
}

int AdvancerNode::getAdvancerCount()
{
	int counter = 1;
	for (int k=0;k<advancerContainers.size();k++)
	{
		counter+=advancerContainers[k].advancers.size();
	}
	return counter;
}

int AdvancerNode::getGridCount()
{
	int counter = 1;
	for (int k=0;k<advancerContainers.size();k++)
	{
		if (advancerContainers[k].type == "Grid")
			counter++;
	}
	return counter;
}


void AdvancerNode::postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events)
{
	uint8* msg_with_ts = new uint8[s.len+8]; // for the two timestamps
	memcpy(msg_with_ts, s.str, s.len);	
	memcpy(msg_with_ts+s.len, &s.timestamp, 8);
	addEvent(events, NETWORK,0,0,GENERIC_EVENT,s.len+8,msg_with_ts);
	delete msg_with_ts;
}

AdvancerContainer AdvancerNode::createStandardGridContainer()
{
	AdvancerContainer c;
	c.type = "Grid";

	c.name = "Grid "+String(getGridCount());

	float gridDiameterMM = 19;
	float interHoleDistanceMM = 1;
	float holeDiameterMM = 0.7;

	c.model.resize(1);
	c.model[0] = createCircle(gridDiameterMM);

	// create all possible advancer locations.
	for (int x = -gridDiameterMM/2; x < gridDiameterMM/2; x+=interHoleDistanceMM)
	{
		for (int y = -gridDiameterMM/2; y < gridDiameterMM/2; y+=interHoleDistanceMM)
		{
			float dist = sqrt( x*x+y*y);
			if (dist <= gridDiameterMM/2) 
			{
				c.advancerLocations.push_back(Circle(x,y,holeDiameterMM/2));
			}
		}
	}

	c.center.x = c.center.y = 0;
	return c;
}


int AdvancerNode::addContainer(String type)
{
	if (type == "StandardGrid") {
		AdvancerContainer c = createStandardGridContainer();
		return addContainer(c);
	}
	return advancerContainers.size();
}

bool AdvancerNode::isUtility()
{
	return true;
}

void AdvancerNode::addMessageToMidiQueue(StringTS S)
{
	messageQueue.push(S);
}

int AdvancerNode::addContainer(AdvancerContainer c)
{
	advancerContainers.push_back(c);
	addMessageToMidiQueue(StringTS("NewAdvancerContainer "+c.name));
	return advancerContainers.size();
}

void AdvancerNode::updateAdvancerPosition(int container, int advancer, float value)
{
	advancerContainers[container].advancers[advancer].depthMM = value;
}

int AdvancerNode::addAdvancerToContainer(int idx)
{
	// cannot add more advancers than the container can hold
	if (advancerContainers[idx].advancers.size() == advancerContainers[idx].advancerLocations.size())
		return -1;


	Advancer advancer;
	advancer.ID = getAdvancerCount();
	advancer.name = "Advancer "+String(advancer.ID);
	advancer.depthMM = 0;
	advancer.description = advancer.name ;
	advancer.probeType = "unknown"; // use to define impedance / optic fiber properties, etc.
	String S = "NewAdvancer "+String(idx) +" "+String(advancer.ID);
	addMessageToMidiQueue(StringTS(S));

	// find first unused location and put the new advancer there.
	std::vector<int> usedLocations;
	for (int k=0;k<	advancerContainers[idx].advancers.size();k++) 
	{
		usedLocations.push_back(advancerContainers[idx].advancers[k].locationIndex);
	}

	int unusedLocation = -1;
	if (usedLocations.size() == 0)
	{
		unusedLocation = 0;
	} else 
	{
		sort(usedLocations.begin(), usedLocations.end());

		for (int k=0; k< usedLocations.size();k++) 
		{
			if (usedLocations[k] != k)
			{
				unusedLocation = k;
				break;
			}
		}
	}

	advancer.locationIndex = unusedLocation;

	advancerContainers[idx].advancers.push_back(advancer);
	return advancerContainers[idx].advancers.size();
}

AdvancerNode::~AdvancerNode()
{

}

bool AdvancerNode::disable()
{
	return true;
}

AudioProcessorEditor* AdvancerNode::createEditor()
{
	editor = new AdvancerEditor(this, true);

	return editor;


}



StringTS AdvancerNode::unpackStringTS(MidiMessage &event) 
{
	const uint8* dataptr = event.getRawData();
	int bufferSize = event.getRawDataSize();
	int string_length = bufferSize-4-8; // -4 for initial event prefix, -8 for timestamp at the end
	int64 timestamp;
	memcpy(&timestamp, dataptr + 4+string_length, 8); // remember to skip first four bytes
	return StringTS((unsigned char *)dataptr+4,string_length,timestamp);
}

void AdvancerNode::handleEvent(int eventType, juce::MidiMessage& event, int samplePosition)
{
	if (eventType == NETWORK)
	{
		StringTS s = unpackStringTS(event);
		std::vector<String> input= splitString(s.getString(),' ');
		if (input[0] == "UpdateAdvancerPosition")
		{
			int advancerID = input[1].getIntValue();
			float newPosition = input[2].getFloatValue();
			// search for this advancer....
			for (int c=0;c<advancerContainers.size();c++)
			{
				for (int a=0;a<advancerContainers[c].advancers.size();a++)
				{
					if (advancerContainers[c].advancers[a].ID == advancerID)
					{
						advancerContainers[c].advancers[a].depthMM = newPosition;
					}					
				}
			}

		}
	}
}


std::vector<String> AdvancerNode::splitString(String S, char sep)
{
	std::list<String> ls;
	String  curr;
	for (int k=0;k < S.length();k++) {
		if (S[k] != sep) {
			curr+=S[k];
		}
		else
		{
			ls.push_back(curr);
			while (S[k] == sep && k < S.length())
				k++;

			curr = "";
			if (S[k] != sep && k < S.length())
				curr+=S[k];
		}
	}
	if (S[S.length()-1] != sep)
		ls.push_back(curr);

	std::vector<String> Svec(ls.begin(), ls.end()); 
	return Svec;

}

void AdvancerNode::process(AudioSampleBuffer& buffer,
						   MidiBuffer& events,
						   int& nSamples)
{
	checkForEvents(events);
	while (messageQueue.size() > 0)
	{
		StringTS S = messageQueue.front();
		postTimestamppedStringToMidiBuffer(S,events);
		messageQueue.pop();
	}
}





void AdvancerNode::saveCustomParametersToXml(XmlElement* parentElement)
{

	for (int i = 0; i < advancerContainers.size(); i++)
	{
		XmlElement* advancerContainerXML = parentElement->createNewChildElement("ADVANCER_CONTAINER");

		advancerContainerXML->setAttribute("type", advancerContainers[i].type);
		advancerContainerXML->setAttribute("name", advancerContainers[i].name);
		advancerContainerXML->setAttribute("numAdvancerLocations", (int)advancerContainers[i].advancerLocations.size());
		advancerContainerXML->setAttribute("numAdvancers", (int)advancerContainers[i].advancers.size());
		advancerContainerXML->setAttribute("centerX", advancerContainers[i].center.x);
		advancerContainerXML->setAttribute("centerY", advancerContainers[i].center.y);
		advancerContainerXML->setAttribute("numModelPolygons", (int)advancerContainers[i].model.size());

		// save container polygonal model
		for (int p=0;p<advancerContainers[i].model.size();p++)
		{
			XmlElement* ModelPolygon = advancerContainerXML->createNewChildElement("POLYGON");
			ModelPolygon->setAttribute("colorR", advancerContainers[i].model[p].color[0]);
			ModelPolygon->setAttribute("colorG", advancerContainers[i].model[p].color[0]);
			ModelPolygon->setAttribute("colorB", advancerContainers[i].model[p].color[0]);

			ModelPolygon->setAttribute("numPoints", (int)advancerContainers[i].model[p].points.size());
			for (int j=0;j<advancerContainers[i].model[p].points.size();j++)
			{
				XmlElement* PolygonPoints = ModelPolygon->createNewChildElement("POLYGON_POINT");
				PolygonPoints->setAttribute("x", advancerContainers[i].model[p].points[j].x);
				PolygonPoints->setAttribute("y", advancerContainers[i].model[p].points[j].y);
			}
		}

		// save all possible advancer locations
		// save container polygonal model
		for (int p=0;p<advancerContainers[i].advancerLocations.size();p++)
		{
			XmlElement* xmlLoc = advancerContainerXML->createNewChildElement("ADVANCER_LOCATION");
			xmlLoc->setAttribute("x",advancerContainers[i].advancerLocations[p].x);
			xmlLoc->setAttribute("y",advancerContainers[i].advancerLocations[p].y);
			xmlLoc->setAttribute("rad",advancerContainers[i].advancerLocations[p].rad);
		}

		// saved advancers
		for (int p=0;p<advancerContainers[i].advancers.size();p++)
		{
				XmlElement* xmlAdv= advancerContainerXML->createNewChildElement("ADVANCER");
				xmlAdv->setAttribute("name",advancerContainers[i].advancers[p].name);
				xmlAdv->setAttribute("probeType",advancerContainers[i].advancers[p].probeType);
				xmlAdv->setAttribute("locationIndex",advancerContainers[i].advancers[p].locationIndex);
				xmlAdv->setAttribute("ID",advancerContainers[i].advancers[p].ID);
				xmlAdv->setAttribute("description",advancerContainers[i].advancers[p].description);
				xmlAdv->setAttribute("depthMM",advancerContainers[i].advancers[p].depthMM);
		}
		
	}


}

Array<String> AdvancerNode::getAdvancerNames()
{
	Array<String> names;
	for (int k=0;k<advancerContainers.size();k++)
	{
		for (int i=0;i<advancerContainers[k].advancers.size();i++)
		{
			names.add(advancerContainers[k].name+"/"+advancerContainers[k].advancers[i].name);
		}
	}
	return names;
}
void AdvancerNode::loadCustomParametersFromXml()
{

	if (parametersAsXml != nullptr)
	{
		advancerContainers.clear();
		AdvancerContainer container;

		int advancerContainerIndex = -1;
		forEachXmlChildElement(*parametersAsXml, xmlNode)
		{
			if (xmlNode->hasTagName("ADVANCER_CONTAINER"))
			{
				advancerContainerIndex++;


				container.type = xmlNode->getStringAttribute("type");
				container.name = xmlNode->getStringAttribute("name");
				container.center.x = xmlNode->getDoubleAttribute("centerX");
				container.center.y = xmlNode->getDoubleAttribute("centerY");

				int numAdvancerLocations = xmlNode->getIntAttribute("numAdvancerLocations");
				int numAdvancers = xmlNode->getIntAttribute("numAdvancers");
				int numPolygons = xmlNode->getIntAttribute("numModelPolygons");

				container.advancerLocations.clear();
				container.advancers.clear();
				container.model.clear();
				container.model.resize(numPolygons);
				container.advancerLocations.resize(numAdvancerLocations);
				container.advancers.resize(numAdvancers);
				int polygonIndex = -1;
				int locationIndex = -1;
				int advancerIndex = -1;
				forEachXmlChildElement(*xmlNode, subNode)
				{
					if (subNode->hasTagName("ADVANCER_LOCATION"))
					{
						locationIndex++;
						container.advancerLocations[locationIndex].x = subNode->getDoubleAttribute("x");
						container.advancerLocations[locationIndex].y = subNode->getDoubleAttribute("y");
						container.advancerLocations[locationIndex].rad = subNode->getDoubleAttribute("rad");
					} else 
						if (subNode->hasTagName("ADVANCER"))
						{
							advancerIndex++;
							container.advancers[advancerIndex].name = subNode->getStringAttribute("name");
							container.advancers[advancerIndex].probeType = subNode->getStringAttribute("probeType");
							container.advancers[advancerIndex].description = subNode->getStringAttribute("description");
							container.advancers[advancerIndex].depthMM = subNode->getDoubleAttribute("depthMM");
							container.advancers[advancerIndex].locationIndex = subNode->getIntAttribute("locationIndex");
							container.advancers[advancerIndex].ID = subNode->getIntAttribute("ID");
						} else  
					if (subNode->hasTagName("POLYGON"))
					{
						polygonIndex++;
						container.model[polygonIndex].color[0] = subNode->getIntAttribute("colorR");
						container.model[polygonIndex].color[1] = subNode->getIntAttribute("colorG");
						container.model[polygonIndex].color[2] = subNode->getIntAttribute("colorB");
						int numPoints = subNode->getIntAttribute("numPoints");
						container.model[polygonIndex].points.resize(numPoints);
						int pointIndex = -1;
						forEachXmlChildElement(*subNode, polygonNode)
						{
							if (polygonNode->hasTagName("POLYGON_POINT"))
							{
								pointIndex++;
								container.model[polygonIndex].points[pointIndex].x= polygonNode->getDoubleAttribute("x");
								container.model[polygonIndex].points[pointIndex].y= polygonNode->getDoubleAttribute("y");
							}
						}

					}

				}

				// add container.
			addContainer(container);
			}

			
		}
	}
	AdvancerEditor *ed = (AdvancerEditor*) getEditor();
	ed->updateFromProcessor();
	
}