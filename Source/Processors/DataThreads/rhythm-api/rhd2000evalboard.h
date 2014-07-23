//----------------------------------------------------------------------------------
// rhd2000evalboard.h
//
// Intan Technoloies RHD2000 Rhythm Interface API
// Rhd2000EvalBoard Class Header File
// Version 1.0 (14 January 2013)
//
// Copyright (c) 2013 Intan Technologies LLC
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any applications that
// use Intan Technologies integrated circuits, and to alter it and redistribute it
// freely.
//
// See http://www.intantech.com for documentation and product information.
//----------------------------------------------------------------------------------

#ifndef RHD2000EVALBOARD_H
#define RHD2000EVALBOARD_H

#define USB_BUFFER_SIZE 2400000
#define RHYTHM_BOARD_ID 500
#define MAX_NUM_DATA_STREAMS 8
#define FIFO_CAPACITY_WORDS 67108864

#include <queue>

using namespace std;

class okCFrontPanel;
class Rhd2000DataBlock;

class Rhd2000EvalBoard
{

public:
    Rhd2000EvalBoard();
	~Rhd2000EvalBoard();

    int open(const char* libname);
    bool uploadFpgaBitfile(string filename);
    void initialize();

    enum AmplifierSampleRate
    {
        SampleRate1000Hz,
        SampleRate1250Hz,
        SampleRate1500Hz,
        SampleRate2000Hz,
        SampleRate2500Hz,
        SampleRate3000Hz,
        SampleRate3333Hz,
        SampleRate4000Hz,
        SampleRate5000Hz,
        SampleRate6250Hz,
        SampleRate8000Hz,
        SampleRate10000Hz,
        SampleRate12500Hz,
        SampleRate15000Hz,
        SampleRate20000Hz,
        SampleRate25000Hz,
        SampleRate30000Hz
    };

    bool setSampleRate(AmplifierSampleRate newSampleRate);
    double getSampleRate() const;
    AmplifierSampleRate getSampleRateEnum() const;

    enum AuxCmdSlot
    {
        AuxCmd1,
        AuxCmd2,
        AuxCmd3
    };

    enum BoardPort
    {
        PortA,
        PortB,
        PortC,
        PortD
    };

    void uploadCommandList(const vector<int> &commandList, AuxCmdSlot auxCommandSlot, int bank);
    void printCommandList(const vector<int> &commandList) const;
    void selectAuxCommandBank(BoardPort port, AuxCmdSlot auxCommandSlot, int bank);
    void selectAuxCommandLength(AuxCmdSlot auxCommandSlot, int loopIndex, int endIndex);

    void resetBoard();
    void resetFpga();
    void setContinuousRunMode(bool continuousMode);
    void setMaxTimeStep(unsigned int maxTimeStep);
    void run();
    bool isRunning() const;
    unsigned int numWordsInFifo() const;
    static unsigned int fifoCapacityInWords();

    void setCableDelay(BoardPort port, int delay);
    void setCableLengthMeters(BoardPort port, double lengthInMeters);
    void setCableLengthFeet(BoardPort port, double lengthInFeet);
    double estimateCableLengthMeters(int delay) const;
    double estimateCableLengthFeet(int delay) const;

    void setDspSettle(bool enabled);

    enum BoardDataSource
    {
        PortA1 = 0,
        PortA2 = 1,
        PortB1 = 2,
        PortB2 = 3,
        PortC1 = 4,
        PortC2 = 5,
        PortD1 = 6,
        PortD2 = 7,
        PortA1Ddr = 8,
        PortA2Ddr = 9,
        PortB1Ddr = 10,
        PortB2Ddr = 11,
        PortC1Ddr = 12,
        PortC2Ddr = 13,
        PortD1Ddr = 14,
        PortD2Ddr = 15
    };

    void setDataSource(int stream, BoardDataSource dataSource);
    void enableDataStream(int stream, bool enabled);
    int getNumEnabledDataStreams() const;

    void clearTtlOut();
    void setTtlOut(int ttlOutArray[]);
    void getTtlIn(int ttlInArray[]);

    enum DacManual
    {
        DacManual1,
        DacManual2
    };

    void setDacManual(DacManual dac, int value);

    void setLedDisplay(int ledArray[]);

    void enableDac(int dacChannel, bool enabled);
    void setDacGain(int gain);
    void setAudioNoiseSuppress(int noiseSuppress);
    void selectDacDataStream(int dacChannel, int stream);
    void selectDacDataChannel(int dacChannel, int dataChannel);

    void flush();
    bool readDataBlock(Rhd2000DataBlock* dataBlock);
    bool readDataBlocks(int numBlocks, queue<Rhd2000DataBlock> &dataQueue);
    int queueToFile(queue<Rhd2000DataBlock> &dataQueue, std::ofstream& saveOut);

private:
    okCFrontPanel* dev;
    AmplifierSampleRate sampleRate;
    int numDataStreams; // total number of data streams currently enabled
    int dataStreamEnabled[MAX_NUM_DATA_STREAMS]; // 0 (disabled) or 1 (enabled)

    // Buffer for reading bytes from USB interface
    unsigned char usbBuffer[USB_BUFFER_SIZE];

    // Opal Kelly module USB interface endpoint addresses
    enum OkEndPoint
    {
        WireInResetRun = 0x00,
        WireInMaxTimeStepLsb = 0x01,
        WireInMaxTimeStepMsb = 0x02,
        WireInDataFreqPll = 0x03,
        WireInMisoDelay = 0x04,
        WireInCmdRamAddr = 0x05,
        WireInCmdRamBank = 0x06,
        WireInCmdRamData = 0x07,
        WireInAuxCmdBank1 = 0x08,
        WireInAuxCmdBank2 = 0x09,
        WireInAuxCmdBank3 = 0x0a,
        WireInAuxCmdLength1 = 0x0b,
        WireInAuxCmdLength2 = 0x0c,
        WireInAuxCmdLength3 = 0x0d,
        WireInAuxCmdLoop1 = 0x0e,
        WireInAuxCmdLoop2 = 0x0f,
        WireInAuxCmdLoop3 = 0x10,
        WireInLedDisplay = 0x11,
        WireInDataStreamSel1234 = 0x12,
        WireInDataStreamSel5678 = 0x13,
        WireInDataStreamEn = 0x14,
        WireInTtlOut = 0x15,
        WireInDacSource1 = 0x16,
        WireInDacSource2 = 0x17,
        WireInDacSource3 = 0x18,
        WireInDacSource4 = 0x19,
        WireInDacSource5 = 0x1a,
        WireInDacSource6 = 0x1b,
        WireInDacSource7 = 0x1c,
        WireInDacSource8 = 0x1d,
        WireInDacManual1 = 0x1e,
        WireInDacManual2 = 0x1f,

        TrigInDcmProg = 0x40,
        TrigInSpiStart = 0x41,
        TrigInRamWrite = 0x42,

        WireOutNumWordsLsb = 0x20,
        WireOutNumWordsMsb = 0x21,
        WireOutSpiRunning = 0x22,
        WireOutTtlIn = 0x23,
        WireOutDataClkLocked = 0x24,
        WireOutBoardId = 0x3e,
        WireOutBoardVersion = 0x3f,

        PipeOutData = 0xa0
    };

    string opalKellyModelName(int model) const;
    double getSystemClockFreq() const;

    bool isDcmProgDone() const;
    bool isDataClockLocked() const;
};

#endif // RHD2000EVALBOARD_H
