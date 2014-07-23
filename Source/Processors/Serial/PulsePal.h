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

// Modified by JS 1/30/2014: Updated op codes for firmware 0_4, added new functions (indicated in comments below)

#ifndef __PULSEPAL_H_F2B7B63E__
#define __PULSEPAL_H_F2B7B63E__

#include <string.h>

#include "ofSerial.h"

/**
  Interface to PulsePal 
  @see PulsePalOutput

*/

class PulsePal
{
public:

    // Initialization and termination
    PulsePal();
    ~PulsePal();
    void initialize();
    uint32_t getFirmwareVersion();
    void disconnectClient();

    void setDefaultParameters();

    // Program single parameter
    void setBiphasic(uint8_t channel, bool isBiphasic);
    void setPhase1Voltage(uint8_t channel, float voltage);
    void setPhase2Voltage(uint8_t channel, float voltage);
    void setPhase1Duration(uint8_t channel, float timeInSeconds);
    void setInterPhaseInterval(uint8_t channel, float timeInSeconds);
    void setPhase2Duration(uint8_t channel, float timeInSeconds);
    void setInterPulseInterval(uint8_t channel, float timeInSeconds);
    void setBurstDuration(uint8_t channel, float timeInSeconds);
    void setBurstInterval(uint8_t channel, float timeInSeconds);
    void setPulseTrainDuration(uint8_t channel, float timeInSeconds);
    void setPulseTrainDelay(uint8_t channel, float timeInSeconds);
    void setTrigger1Link(uint8_t channel, uint8_t link_state);
    void setTrigger2Link(uint8_t channel, uint8_t link_state);
    void setCustomTrainID(uint8_t channel, uint8_t ID); // ID = 0: no custom train. ID = 1-2: custom trains 1 or 2
    void setCustomTrainTarget(uint8_t channel, uint8_t target); // target = 0: Custom times define pulses Target = 1: They define bursts
    void setCustomTrainLoop(uint8_t channel, uint8_t loop_state); // loop_state = 0: No loop 1: loop

    // Program all parameters
    void programAllParams();

    // Program custom pulse train
    void programCustomTrain(uint8_t ID, uint8_t nPulses, float customPulseTimes[], float customVoltages[]);

    // Operations and settings
    void triggerChannel(uint8_t channel);
    void triggerChannels(uint8_t channel1, uint8_t channel2, uint8_t channel3, uint8_t channel4);
    void updateDisplay(string line1, string line2);
    void setFixedVoltage(uint8_t channel, float voltage);
    void abortPulseTrains();
    void setContinuousLoop(uint8_t channel, uint8_t state);
    void setTriggerMode(uint8_t channel, uint8_t mode);
    
    // Fields
    struct OutputParams {
        int isBiphasic;
        float phase1Voltage;
        float phase2Voltage;
        float phase1Duration;
        float interPhaseInterval;
        float phase2Duration;
        float interPulseInterval;
        float burstDuration;
        float interBurstInterval;
        float pulseTrainDuration;
        float pulseTrainDelay;
        int linkTriggerChannel1;
        int linkTriggerChannel2;
        int customTrainID;
        int customTrainTarget;
        int customTrainLoop;
    } currentOutputParams[5]; // Use 1-indexing for the channels (output channels 1-4 = currentOutputParams[1]-currentOutputParams[4])
    struct InputParams {
        int triggerMode;
    } currentInputParams[3]; // Use 1-indexing for the trigger channels

private:
    void constrain(uint32_t* value, uint32_t min, uint32_t max);
    void program(uint8_t channel, uint8_t paramCode, uint32_t paramValue);
    void program(uint8_t channel, uint8_t paramCode, uint8_t paramValue);
    uint8_t voltageToByte(float voltage);
    ofSerial serial;

};

#endif  // __PULSEPAL_H_F2B7B63E__