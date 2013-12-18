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

#ifndef __ADVANCERNODE_H
#define __ADVANCERNODE_H

#include "../../JuceLibraryCode/JuceHeader.h"
#include "GenericProcessor.h"
#include "NetworkEvents.h"
#include <list>

#ifndef MAX
#define MAX(a,b)((a)<(b)?(b):(a))
#endif

#ifndef MIN
#define MIN(a,b)((a)<(b)?(a):(b))
#endif


/* a class that represents a device that moves a probe. 
in the most basic scenario, this is either a micromanipulator or a screw that advances
the probe into the brain. It is very useful to keep track of electrodes position since
it makes post processing analysis easier and fully automated */
class Advancer
{
public:
	Advancer() {}
	String name; // official name of the advancer
	String description; // what is this advancer targeting?
	String probeType;  // screw/narashige drive/...
	float depthMM;
	int ID; // unique identifier.
	int locationIndex; // location in advancerLocations
};


class Circle
{
public:
	Circle() {}
	Circle(float _x, float _y, float _rad) : x(_x),y(_y),rad(_rad) {}

	float x,y,rad;
};

class Point2D
{
public:
	Point2D() {}
	float x,y;
};

class Polygon2D
{
public:
	Polygon2D();
	Polygon2D(int N);
	std::vector<Point2D> points;
	uint8 color[3];
};

class AdvancerContainer
{
public:
	AdvancerContainer() {}
	int ID;
	String name; 
	String type; // hyperdrive / grid / cannule / ...?
	std::vector<Advancer> advancers;

	void getModelRange(float &minX, float &maxX, float &minY, float &maxY);
	std::vector<Polygon2D> model;
	std::vector<Circle> advancerLocations;
	Point2D center; // anterior-posterior and medial-lateral coordinates and radius
	};


class AdvancerNode : public GenericProcessor
{
public:
    AdvancerNode();
    ~AdvancerNode();
	AudioProcessorEditor* createEditor();
    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages, int& nSamples);
	bool disable();
	bool enable();

	int addContainer(String type, String parameters);
	int addContainer(AdvancerContainer c);
	bool isUtility();
	int addAdvancerToContainer(int containerIndex);
	void updateAdvancerPosition(int container, int advancer, float value);
	Array<String> getAdvancerNames(bool addContainer = true);
	void postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events);
	void addMessageToMidiQueue(StringTS S);
	double setAdvancerPosition(int AdvancerID, double newDepth, bool add);
	void saveCustomParametersToXml(XmlElement* parentElement);
	double getAdvancerPosition(String advancerName);
	String interProcessorCommunication(String command);
	double getAdvancerPosition(int advancerID);
	void loadCustomParametersFromXml();
	Array<int> getAdvancerIDs();
	int removeContainer(int containerIndex);
	int removeAdvancer(int containerIndex, int advancerIndex);
	void updateAdvancerLocation(int selectedContainer, int selectedAdvancer, int newLocation);
	int addContainerUsingXmlFile(File xmlfile);
	std::vector<AdvancerContainer> advancerContainers;
	CriticalSection lock;

private:
		
	String DropStringFrom(std::vector<String> input, int index);

	void handleEvent(int eventType, MidiMessage& event, int samplePos);
	int getAdvancerCount();
	AdvancerContainer createStandardGridContainer();
	AdvancerContainer createStandardCannulaContainer();
	AdvancerContainer createStandardHyperDriveContainer(int numTetrodes);
	Polygon2D createCircle(float diameterMM);
	
	int getContainerCount(String containerType);
	std::queue<StringTS> messageQueue;
	Time timer;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdvancerNode);
};

#endif  
