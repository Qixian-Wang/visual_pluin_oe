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
    addIntParameter(Parameter::GLOBAL_SCOPE,
                    "max_rate",
                    "Max tolerable rate in Hz",
                    50, 5, 200); // Default: 50, Min: 5, Max: 200

    addIntParameter(Parameter::GLOBAL_SCOPE,
                    "window_size",
                    "Size of the window in ms",
                    1000, 100, 5000); // Default: 1000, Min: 100, Max: 5000
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
    if (canvas != nullptr)
    {
        parameterValueChanged(getParameter("window_size"));
        parameterValueChanged(getParameter("max_rate"));
    }

}


void RateViewer::process(AudioBuffer<float>& buffer)
{
    checkForEvents(true);
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
   if (param->getName().equalsIgnoreCase("window_size"))
   {
      int windowSize = (int)param->getValue();

      if (canvas != nullptr)
            canvas->setWindowSizeMs(windowSize);  // Update window size in canvas
   }
   else if (param->getName().equalsIgnoreCase("max_rate"))
   {
      int max_rate = (int)param->getValue();

      if (canvas != nullptr)
            canvas->setMaxRate(max_rate);
   }
}


void RateViewer::handleSpike (SpikePtr spike)
{
    int start1, size1, start2, size2;
    spikeFifo.prepareToWrite(1, start1, size1, start2, size2);

    if (size1 > 0)
        spikeBuffer[start1] = {spike->getChannelInfo()->getGlobalIndex()};
    if (size2 > 0)
        spikeBuffer[start2] = {spike->getChannelInfo()->getGlobalIndex()};

    spikeFifo.finishedWrite (size1 + size2);

    triggerAsyncUpdate();
}

void RateViewer::handleAsyncUpdate()
{
    if (! canvas)
        return;

    int start1, size1, start2, size2;
    spikeFifo.prepareToRead(1, start1, size1, start2, size2);

    if (size1 > 0)
    {
        auto& evt = spikeBuffer[start1];
        canvas->addSpike (evt.channel);
    }
    if (size2 > 0)
    {
        auto& evt = spikeBuffer[start2];
        canvas->addSpike (evt.channel);
    }

    spikeFifo.finishedRead (size1 + size2);
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
