/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2022 Open Ephys

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

#include "RateViewer.h"
#include "RateViewerCanvas.h"
#include "RateViewerEditor.h"


RateViewer::RateViewer()
 : GenericProcessor("Rate Viewer")
{

}


RateViewer::~RateViewer()
{

}


AudioProcessorEditor* RateViewer::createEditor()
{
    editor = std::make_unique<RateViewerEditor>(this);
    return editor.get();
}


void RateViewer::updateSettings()
{
    channelList.clear();
    channelIndexMap.clear();

    int idx = 0;
    for (auto channel : spikeChannels)
    {
        if (channel->isValid())
        {
            channelList.add(channel);
            channelIndexMap.set(channel, idx);
            ++idx;
        }
    }

}


void RateViewer::process(AudioBuffer<float>& buffer)
{

    checkForEvents(true);

    int64 mostRecent = 0;
    for (auto stream : getDataStreams())
    {
        int sid = stream->getStreamId();
        auto blockStart = getFirstSampleNumberForBlock(sid);
        auto nSamples   = getNumSamplesInBlock(sid);
        mostRecent = jmax(mostRecent, blockStart + (int64)nSamples);
    }

    if (canvas)
        canvas->setMostRecentSample(mostRecent);
	 
}


void RateViewer::handleTTLEvent(TTLEventPtr event)
{

}


void RateViewer::handleBroadcastMessage(String message)
{

}


void RateViewer::saveCustomParametersToXml(XmlElement* parentElement)
{

}


void RateViewer::loadCustomParametersFromXml(XmlElement* parentElement)
{

}

void RateViewer::parameterValueChanged(Parameter* param)
{

}

void RateViewer::handleSpike(SpikePtr spike)
{
    if (! canvas)
        return;

    auto* chinfo = spike->getChannelInfo();
    auto idx = channelIndexMap[chinfo];
    canvas->addSpike(idx, spike->getSampleNumber());
}

bool RateViewer::startAcquisition()
{
   ((RateViewerEditor*)getEditor())->enable();
   return true;
}

bool RateViewer::stopAcquisition()
{
   ((RateViewerEditor*)getEditor())->disable();
   return true;
}
