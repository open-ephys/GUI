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
	:GenericProcessor("Advancer Node")

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
		advancerContainers.push_back(createStandardGridContainer());
	}
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
	advancer.name = "Advancer "+String(getAdvancerCount());
	advancer.depthMM = 0;
	advancer.description = advancer.name ;
	advancer.manipulator = "unknown";
	
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


void AdvancerNode::handleEvent(int eventType, juce::MidiMessage& event, int samplePosition)
{
    if (eventType == NETWORK)
    {
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
	
}

