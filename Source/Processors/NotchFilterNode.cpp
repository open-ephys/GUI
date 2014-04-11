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
#include "NotchFilterNode.h"
#include "Editors/NotchFilterEditor.h"

NotchFilterNode::NotchFilterNode()
    : GenericProcessor("Notch Filter"), defaultCenterFreq(60.0f), defaultBandwidth(2.0f), active(true)

{
	applyOnADC = false;
}

NotchFilterNode::~NotchFilterNode()
{

}

AudioProcessorEditor* NotchFilterNode::createEditor()
{
    editor = new NotchFilterEditor(this, true);

    NotchFilterEditor* ed = (NotchFilterEditor*) getEditor();
	ed->setDefaults(defaultCenterFreq, defaultBandwidth);

    std::cout << "Creating editor." << std::endl;

    return editor;
}

// ----------------------------------------------------
// From the filter library documentation:
// ----------------------------------------------------
//
// each family of filters is given its own namespace
// RBJ: filters from the RBJ cookbook
// Butterworth
// ChebyshevI: ripple in the passband
// ChebyshevII: ripple in the stop band
// Elliptic: ripple in both the passband and stopband
// Bessel: theoretically with linear phase
// Legendre: "Optimum-L" filters with steepest transition and monotonic passband
// Custom: Simple filters that allow poles and zeros to be specified directly

// within each namespace exists a set of "raw filters"
// Butterworth::LowPass
//				HighPass
// 				BandPass
//				BandStop
//				LowShelf
// 				HighShelf
//				BandShelf
//
//	class templates (such as SimpleFilter) which require FilterClasses
//    expect an identifier of a raw filter
//  raw filters do not support introspection, or the Params style of changing
//    filter settings; they only offer a setup() function for updating the IIR
//    coefficients to a given set of parameters
//

// each filter family namespace also has the nested namespace "Design"
// here we have all of the raw filter names repeated, except these classes
//  also provide the Design interface, which adds introspection, polymorphism,
//  the Params style of changing filter settings, and in general all fo the features
//  necessary to interoperate with the Filter virtual base class and its derived classes

// available methods:
//
// filter->getKind()
// filter->getName()
// filter->getNumParams()
// filter->getParamInfo()
// filter->getDefaultParams()
// filter->getParams()
// filter->getParam()

// filter->setParam()
// filter->findParamId()
// filter->setParamById()
// filter->setParams()
// filter->copyParamsFrom()

// filter->getPoleZeros()
// filter->response()
// filter->getNumChannels()
// filter->reset()
// filter->process()

void NotchFilterNode::updateSettings()
{
	int id = nodeId;
	int numInputs = getNumInputs();
	int numfilt = filters.size();
    if (numInputs < 1024 && numInputs != numfilt)
    {
		// SO fixed this. I think values were never restored correctly because you cleared lowCuts.
	    Array<double> oldCenter, oldBandwidth;
		oldCenter = centerFreq;
		oldBandwidth = bandwidth;

        filters.clear();
        centerFreq.clear();
        bandwidth.clear();
        shouldFilterChannel.clear();

        for (int n = 0; n < getNumInputs(); n++)
        {

            // std::cout << "Creating filter number " << n << std::endl;

            filters.add(new Dsp::SmoothedFilterDesign
                        <Dsp::Butterworth::Design::BandStop 	// design type
                        <3>,								 	// order
                        1,										// number of channels (must be const)
                        Dsp::DirectFormII>						// realization
                        (1));


             // restore defaults

            shouldFilterChannel.add(true);

            float cn, bw;

            if (oldCenter.size() > n)
            {
                cn = oldCenter[n];
                bw = oldBandwidth[n];
            } else {
                cn = defaultCenterFreq;
                bw = defaultBandwidth;
            }

			centerFreq.add(cn);
			bandwidth.add(bw);

            setFilterParameters(cn, bw, n);
        }

    }

    setApplyOnADC(applyOnADC);

}

double NotchFilterNode::getCenterFreqValueForChannel(int chan)
{
    return centerFreq[chan];
}


double NotchFilterNode::getBandWidthValueForChannel(int chan)
{
    return bandwidth[chan];
}

bool NotchFilterNode::getBypassStatusForChannel(int chan)
{
    return shouldFilterChannel[chan];
}

void NotchFilterNode::setFilterParameters(double centerF, double width, int chan)
{

    Dsp::Params params;
    params[0] = getSampleRate(); // sample rate
    params[1] = 2; // order
    params[2] = centerF; // center frequency
    params[3] = width; // bandwidth

    if (filters.size() > chan)
        filters[chan]->setParams(params);

}

void NotchFilterNode::setParameter(int parameterIndex, float newValue)
{

    if (parameterIndex < 2) // change filter settings
    {

        if (newValue <= 0.01 || newValue >= 10000.0f)
            return;

        //std::cout << "Setting channel " << currentChannel;// << std::endl;

        if (parameterIndex == 0)
        {
           // std::cout << " low cut to " << newValue << std::endl;
            centerFreq.set(currentChannel,newValue);
        }
        else if (parameterIndex == 1)
        {
            //std::cout << " high cut to " << newValue << std::endl;
            bandwidth.set(currentChannel,newValue);
        }

          setFilterParameters(centerFreq[currentChannel],
                        bandwidth[currentChannel],
                        currentChannel);

        editor->updateParameterButtons(parameterIndex);

    } else // change channel bypass state
    {
        if (newValue == 0)
        {
            shouldFilterChannel.set(currentChannel, false);
        } else {
            shouldFilterChannel.set(currentChannel, true);
        }
        
    }
}

void NotchFilterNode::process(AudioSampleBuffer& buffer,
                         MidiBuffer& midiMessages,
                         int& nSamples)
{

    for (int n = 0; n < getNumOutputs(); n++)
    {
		if (shouldFilterChannel[n])
		{
			float* ptr = buffer.getSampleData(n);
			filters[n]->process(nSamples, &ptr);
		}
    }

}

void NotchFilterNode::setApplyOnADC(bool state)
{

    for (int n = 0; n < channels.size(); n++)
    {
		if (channels[n]->getType() == ADC_CHANNEL)
        {
            setCurrentChannel(n);

            if (state)
                setParameter(2,1.0);
            else
                setParameter(2,0.0);
        }
    }
}

void NotchFilterNode::saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNumber, bool isEventChannel)
{

    //std::cout << "CHANNEL: " << channelNumber << std::endl;

	if (!isEventChannel && channelNumber > -1 && channelNumber < centerFreq.size())
    {
        //std::cout << "Saving custom parameters for filter node." << std::endl;

        XmlElement* channelParams = channelInfo->createNewChildElement("PARAMETERS");
        channelParams->setAttribute("center",centerFreq[channelNumber]);
        channelParams->setAttribute("bandwidth",bandwidth[channelNumber]);
        channelParams->setAttribute("shouldFilter",shouldFilterChannel[channelNumber]);
    }

}

void NotchFilterNode::loadCustomChannelParametersFromXml(XmlElement* channelInfo, bool isEventChannel)
{

    int channelNum = channelInfo->getIntAttribute("number");

    if (!isEventChannel)
    {
        forEachXmlChildElement(*channelInfo, subNode)
        {
            if (subNode->hasTagName("PARAMETERS"))
            {
                centerFreq.set(channelNum, subNode->getDoubleAttribute("center",defaultCenterFreq));
                bandwidth.set(channelNum, subNode->getDoubleAttribute("bandwidth",defaultBandwidth));
                shouldFilterChannel.set(channelNum, subNode->getBoolAttribute("shouldFilter",true));

                setFilterParameters(centerFreq[channelNum],
                                    bandwidth[channelNum],
                                    channelNum);

            }
        }
    }


}
