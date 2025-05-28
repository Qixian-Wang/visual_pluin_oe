/*
------------------------------------------------------------------

This file is part of a plugin for the Open Ephys GUI
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

#include "RateViewerCanvas.h"
#include "RateViewerEditor.h"
#include "RateViewer.h"


RateViewerCanvas::RateViewerCanvas(RateViewer* processor_)
	: processor(processor_)
{
	plt.setBounds(5, 5, 1500, 1000);
    refreshRate = 30;
}


RateViewerCanvas::~RateViewerCanvas()
{

}

void RateViewerCanvas::setWindowSizeMs(int windowSize_)
{
    windowSize = windowSize_;
    updateLayout();
}

void RateViewerCanvas::setBinSizeMs(int binSize_)
{
    binSize = binSize_;
}

void RateViewerCanvas::updateLayout()
{
    auto plotArea = Rectangle<int>(5, 5, windowSize, windowSize);
    const float margin = 50.0f;
    plotArea = plotArea.reduced(margin);
    const float radius = 10.0f * windowSize / 1000.0f;

    float max_x = std::numeric_limits<float>::min();
    float min_x = std::numeric_limits<float>::max();
    float max_y = std::numeric_limits<float>::min();
    float min_y = std::numeric_limits<float>::max();

    for (const auto& [_, coord] : electrode_map) {
        max_x = std::max(max_x, coord.first);
        min_x = std::min(min_x, coord.first);
        max_y = std::max(max_y, coord.second);
        min_y = std::min(min_y, coord.second);
    }

    float dx = plotArea.getWidth();
    float dy = plotArea.getHeight();

    int dx_text = 80 * windowSize / 1000.0f;
    
    screenCoordinates.clear();
    while (electrodeLabels.size() < electrode_map.size())
        electrodeLabels.add(new Label());
    while (electrodeLabels.size() > electrode_map.size())
        electrodeLabels.removeLast();

    int labelIndex = 0;
    for (const auto& [idx, coord] : electrode_map) {
        float norm_x = (coord.first - min_x) / (max_x - min_x);
        float norm_y = (coord.second - min_y) / (max_y - min_y);
        float screen_x = plotArea.getX() + norm_x * plotArea.getWidth();
        float screen_y = plotArea.getY() + norm_y * plotArea.getHeight();
        screenCoordinates[idx] = {screen_x, screen_y};

        auto* rate_text = electrodeLabels[labelIndex++];
        rate_text->setJustificationType(Justification::centred);
        rate_text->setFont(Font(14.0f * windowSize / 1000.0f));
        rate_text->setColour(Label::textColourId, Colours::white);
        rate_text->setBounds((int)(screen_x - dx_text/2),
                       (int)(screen_y + radius + 2),
                       dx_text,
                       (int)rate_text->getFont().getHeight());
        addAndMakeVisible(rate_text);
    }

    electrodeImage = Image(Image::ARGB, getWidth(), getHeight(), true);
    Graphics g(electrodeImage);
    g.fillAll(Colours::transparentBlack);
    
    g.setColour(Colours::white.withAlpha(0.8f));
    for (const auto& [idx, screen_coord] : screenCoordinates)
    {
        g.drawEllipse(screen_coord.first - radius, 
                     screen_coord.second - radius, 
                     radius*2.0f, radius*2.0f, 2.0f);
    }
    
    repaint();
}

void RateViewerCanvas::resized()
{

}

void RateViewerCanvas::refreshState()
{

}

void RateViewerCanvas::paintOverChildren(Graphics& g)
{
    g.drawImageAt(electrodeImage, 0, 0);

    for (auto& kv : flashingflag)
    {
        int   ch   = kv.first;
        bool  flash= kv.second;
        const float radius = 5.0f * windowSize / 1000.0f;

        if (flash && screenCoordinates.find(ch) != screenCoordinates.end())
        {
            g.setColour(Colours::red);
            g.fillEllipse(screenCoordinates[ch].first - radius,
                screenCoordinates[ch].second - radius,
                radius * 2.0f, radius * 2.0f);
        }
    }
}

void RateViewerCanvas::update()
{

}


void RateViewerCanvas::paint(Graphics& g)
{

	g.fillAll(Colours::black);

}

void RateViewerCanvas::setPlotTitle(const String& title)
{
   plt.title(title);
}

void RateViewerCanvas::addSpike(int channelId)
{
    int64 currentTime = Time::getMillisecondCounter();
    spikeTimestamps[channelId].push_back(currentTime);
    
    flashingflag[channelId] = true;
    flashEndTime[channelId] = Time::getMillisecondCounter() + 200;
}


void RateViewerCanvas::updateElectrodeLabels()
{
    for (int i = 0; i < electrodeLabels.size(); ++i)
    {
        if (channelRates.find(i) != channelRates.end())
        {
            float rate = channelRates[i];
            electrodeLabels[i]->setText(String(rate, 1),
                                   NotificationType::dontSendNotification);
        }
        else
        {
            electrodeLabels[i]->setText("0.0",
                                   NotificationType::dontSendNotification);
        }
    }
}

void RateViewerCanvas::refresh()
{
    int64 currentTime = Time::getMillisecondCounter();
    int64 windowStart = currentTime - windowSize;

    for (auto& [channelId, timestamps] : spikeTimestamps)
    {
        while (!timestamps.empty() && timestamps.front() < windowStart)
        {
            timestamps.pop_front();
        }

        float spikesInWindow = timestamps.size();
        float windowSizeInSeconds = binSize / 1000.0f;
        float rate = spikesInWindow / windowSizeInSeconds;  // Hz
        
        channelRates[channelId] = rate;
    }
    
    for (auto it = flashEndTime.begin(); it != flashEndTime.end();)
    {
        if (currentTime >= it->second)
        {
            flashingflag[it->first] = false;
            it = flashEndTime.erase(it);
        }
        else ++it;
    }

    updateElectrodeLabels();
    repaint();
}

