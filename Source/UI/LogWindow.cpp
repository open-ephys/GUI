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



#include "../../JuceLibraryCode/JuceHeader.h"
#include "LogWindow.h"
#include "../Processors/Visualization/Visualizer.h"
#include "../Processors/NetworkEvents.h"
/*
class LogWindowCanvas : public Visualizer
{
public:
	LogWindowCanvas();
};

class LogWindowDisplay : public Component
{
public:
    LogWindowDisplay(LogWindowCanvas*, Viewport*);
    ~LogWindowDisplay();
};

LogWindowDisplay::LogWindowDisplay(LogWindowCanvas *cv, Viewport *vp)
{
}
*/
LogWindow::LogWindow() 
{
	/*
	LogWindowDisplay* logWindowDisplay = new LogWindowDisplay(
	Viewport* viewport = new Viewport("Log");
    viewport->setViewedComponent(logWindowDisplay, false);
    viewport->setScrollBarsShown(true, false);
	*/
    labelFont = Font("Paragraph", 24, Font::plain);
    infoString = "";
	  //  viewport->setScrollBarsShown(true, false);

    //scrollBarThickness = viewport->getScrollBarThickness();

}


void LogWindow::addLineToLog(StringTS S)
{
	addLineToLog(S.getString());
}

void LogWindow::addLineToLog(String S)
{
	const int maxLines = 15;
	Time t= 	t.getCurrentTime();
	String tm = String(t.getMinutes()) + String(":") + String(t.getSeconds()) + ":"+String(t.getMilliseconds())+" ";
	log.push_back(tm+S);
	if (log.size() > maxLines)
		log.pop_front();

	infoString = "";
	for (std::list<String>::iterator it = log.begin(); it!=log.end();it++) {
		infoString+=(*it)+"\n";
	}
	
}


LogWindow::~LogWindow()
{
int x = 5;
}

void LogWindow::timerCallback()
{
	repaint();
}

void LogWindow::paint(Graphics& g)
{
    g.fillAll(Colours::grey);

    g.setFont(labelFont);

    g.setColour(Colours::black);

    g.drawFittedText(infoString, 10, 10, getWidth()-10, getHeight()-10, Justification::left, 100);

	if (firstTime) {
		startTimer(500);
		firstTime = false;
	}

}
