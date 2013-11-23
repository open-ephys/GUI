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

#include "AdvancerEditor.h"
#include <stdio.h>

AdvancerEditor::AdvancerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
	desiredWidth = 180;

  

    setEnabledState(false);

}



void AdvancerEditor::buttonEvent(Button* button)
{
		//	NetworkEvents *processor  = (NetworkEvents*) getProcessor();


}
void AdvancerEditor::labelTextChanged(juce::Label *label)
{
	
}


AdvancerEditor::~AdvancerEditor()
{

}


