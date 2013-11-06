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

#include "NetworkEventsEditor.h"


#include <stdio.h>

NetworkEventsEditor::NetworkEventsEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{

    urlButton = new UtilityButton("Select port",Font("Small Text", 13, Font::plain));
    urlButton->addListener(this);
    urlButton->setBounds(30,50,120,25);
    addAndMakeVisible(urlButton);

    urlLabel = new Label("FileNameLabel", "No port defined.");
    urlLabel->setBounds(20,80,140,25);
    addAndMakeVisible(urlLabel);

    desiredWidth = 180;

    setEnabledState(false);

}

NetworkEventsEditor::~NetworkEventsEditor()
{

}


void NetworkEventsEditor::buttonEvent(Button* button)
{

    if (!acquisitionIsActive)
    {

        if (button == urlButton)
        {
            //std::cout << "Button clicked." << std::endl;
			/*
            FileChooser chooseFileReaderFile("Please select the file you want to load...",
                                             lastFilePath,
                                             "*");
											 */
           
                // fileNameLabel->setText(fileToRead.getFileName(),false);
           
        }

    }
}
