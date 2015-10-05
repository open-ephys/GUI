/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef __SPLITTEREDITOR_H_33F644A8__
#define __SPLITTEREDITOR_H_33F644A8__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"

/**

  User interface for the OSCNode utility.

  @see OSCNode

*/

class OSCEditor : public GenericEditor
{
public:
    OSCEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~OSCEditor();

    void buttonEvent(Button* button);

    void switchDest(int);
    void switchDest();
	void setPositionLabel(float x, float y);

    void switchIO(int i);

    int getPathForEditor(GenericEditor* editor);
    Array<GenericEditor*> getConnectedEditors();


private:
    ScopedPointer<Label> positionLabel;

    ImageButton* pipelineSelectorA;
    ImageButton* pipelineSelectorB;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCEditor);

};


#endif  // __SPLITTEREDITOR_H_33F644A8__
