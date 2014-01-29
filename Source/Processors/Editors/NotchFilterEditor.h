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

#ifndef __NOTCH_FILTEREDITOR_H
#define __NOTCH_FILTEREDITOR_H


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

class NotchFilterViewport;

/**

  User interface for the FilterNode processor.

  @see FilterNode

*/

class NotchFilterEditor : public GenericEditor,
	public Label::Listener
{
public:
    NotchFilterEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~NotchFilterEditor();

    void buttonEvent(Button* button);
    void labelTextChanged(Label* label);

    void saveCustomParameters(XmlElement* xml);
    void loadCustomParameters(XmlElement* xml);

    void setDefaults(double center, double bandwidth);

    void channelChanged(int chan);

private:

    String centerString;
    String bandwidthString;

    ScopedPointer<Label> centerLabel;
    ScopedPointer<Label> bandwidthLabel;

    ScopedPointer<Label> centerValue;
    ScopedPointer<Label> bandwidthValue;
	ScopedPointer<UtilityButton> applyFilterOnADC;
    ScopedPointer<UtilityButton> applyFilterOnChan;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NotchFilterEditor);

};



#endif  // __FILTEREDITOR_H_969BDB5__
