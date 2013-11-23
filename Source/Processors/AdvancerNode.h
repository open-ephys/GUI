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

#include <list>

class Advancer
{
public:
	Advancer() {}
	String name; // official name of the advancer
	String description; // what is this advancer targeting?
	String manipulator;  // screw/narashige drive/...
	float depthMM;
	int locationIndex; // location in advancerLocations
};


class Circle
{
public:
	Circle(float _x, float _y, float _rad) : x(_x),y(_y),rad(_rad) {}

	float x,y,rad;
};

class Point2D
{
public:
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
	String name; 
	String type; // hyperdrive / grid / cannule / ...?
	std::vector<Advancer> advancers;

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
	std::vector<String> splitString(String S, char sep);
	bool disable();
	int addContainer(String type);
	int addAdvancerToContainer(int containerIndex);
	void updateAdvancerPosition(int container, int advancer, float value);
	std::vector<AdvancerContainer> advancerContainers;
private:
	void handleEvent(int eventType, MidiMessage& event, int samplePos);
	int getAdvancerCount();
	AdvancerContainer createStandardGridContainer();
	Polygon2D createCircle(float diameterMM);
	int getGridCount();
	Time timer;
	std::list<float> advancerDepth;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdvancerNode);
};

#endif  
