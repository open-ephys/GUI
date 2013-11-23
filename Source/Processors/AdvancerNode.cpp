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

