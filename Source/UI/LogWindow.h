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

#ifndef __LOGWINDOW_H__
#define __LOGWINDOW_H__


#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Processors/NetworkEvents.h"
#include <list>

/**

  Displays general instructions about how to use the application.

  Inhabits a tab in the DataViewport.

  @see UIComponent, DataViewport

*/
/*
class LogWindowEditor  : public GenericEditor
{
public:
    LogWindowEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~LogWindowEditor();
};
*/

class LogWindow : public Component,public Timer
{
public:
    LogWindow();
    ~LogWindow();
	void run();

    
	void timerCallback();
    void paint(Graphics& g);
	void addLineToLog(String S);
	void addLineToLog(StringTS S);
private:

    String infoString;
	std::list<String> log;
    Font labelFont;
	bool firstTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogWindow);


};




#endif 
