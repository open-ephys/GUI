/*
 ------------------------------------------------------------------
 
 This file is part of the Open Ephys GUI
 Copyright (C) 2013 Florian Franzen
 
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

#ifndef __SERIALINPUT_H_B8E3F86B__
#define __SERIALINPUT_H_B8E3F86B__

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../../JuceLibraryCode/JuceHeader.h"
#include "GenericProcessor.h"

#include "Editors/SerialInputEditor.h"
#include "Serial/ofSerial.h"

/**
 
 This source processor allows you to pipe binary serial data input straight to the event cue/buffer.
 
 @see SerialInputEditor
 
 */

class SerialInput : public GenericProcessor

{
public:
    
    /** The class constructor, used to initialize any members. */
    SerialInput();
    
    /** The class destructor, used to deallocate memory */
    virtual ~SerialInput();
    
    /**
     Returns a list of all serial devices that are available on the system.
     
     The list of available devices changes whenever devices are connected or removed.
     */
    StringArray getDevices();
    
    /** 
     Returns a list of all supported baudrates.
     */
    Array<int> getBaudrates();
    
    /** 
     Used by the ProcessorGraph to determines whether the processor should be treated as a source.

     Returns always true, since SerialInput is a source.
     */
    bool isSource()
    {
        return true;
    }
    
    /** 
     This should only be run by the ProcessorGraph, before acquisition will be started.
     
     It tries to open the serial port previsouly specified by the setDevice and setBaudrate setters.
     
     Returns true on success, false if port could not be opened.
     */
    bool isReady();
    
    /** 
     Called immediately after the end of data acquisition by the ProcessorGraph.
     
     It closes the open port serial port.
     */
    bool disable();
    
    /** Setter, that allows you to set the serial device that will be used during acquisition */
    void setDevice(string device);
    
    /** Setter, that allows you to set the baudrate that will be used during acquisition */
    void setBaudrate(int baudrate);

    /** Defines the functionality of the processor.
     
     The process method is called every time a new data buffer is available.
     
     Adds all the new serial data that is available to the event data buffer.
     */
    void process(AudioSampleBuffer& buffer, MidiBuffer& events, int& nSamples);
    
    /**
     Returns the Juce GUI component that should be used with this processor.
     */
    AudioProcessorEditor* createEditor();
    
private:
    
    // The current serial connection
    ofSerial serial;
    
    // The serial device to be used
    string device;
    
    // The baudrate to be used
    int baudrate;

    // List of baudrates that are available by default.
    static const int BAUDRATES[12];
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SerialInput);
};

#endif  // __SERIALINPUT_H_B8E3F86B__
