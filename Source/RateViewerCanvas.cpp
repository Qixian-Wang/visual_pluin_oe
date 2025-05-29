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

void RateViewerCanvas::setMaxRate(int maxRate_)
{
    maxRate = maxRate_;
}

void RateViewerCanvas::updateLayout()
{
    auto plotArea = Rectangle<int>(5, 5, windowSize, windowSize);
    const float margin = 50.0f;
    plotArea = plotArea.reduced(margin);

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

    // Calculate minimum distances between electrodes
    float min_dx = std::numeric_limits<float>::max();
    float min_dy = std::numeric_limits<float>::max();
    
    for (const auto& [id1, coord1] : electrode_map) {
        for (const auto& [id2, coord2] : electrode_map) {
            if (id1 != id2) {
                float dx = std::abs(coord1.first - coord2.first);
                float dy = std::abs(coord1.second - coord2.second);
                if (dx > 0) min_dx = std::min(min_dx, dx);
                if (dy > 0) min_dy = std::min(min_dy, dy);
            }
        }
    }

    // Calculate scaling factors
    float dx = plotArea.getWidth() / (max_x - min_x);
    float dy = plotArea.getHeight() / (max_y - min_y);
    
    // Calculate electrode size based on minimum distances
    electrode_width = (min_dx / (max_x - min_x)) * plotArea.getWidth();
    electrode_height = (min_dy / (max_y - min_y)) * plotArea.getHeight();

    int dx_text = 80 * electrode_width / 100.0f;
    
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
        rate_text->setFont(Font(20.0f * electrode_width / 100.0f));
        rate_text->setColour(Label::textColourId, Colours::white);
        rate_text->setBounds((int)(screen_x + electrode_width/2 - dx_text/2),
                       (int)(screen_y + electrode_height * 0.8),
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
        g.drawRect(screen_coord.first, 
                  screen_coord.second, 
                  electrode_width, 
                  electrode_height,
                  2.0f);
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
    
    const float margin = 5.0f * electrode_width / 100.0f;

    if (useHeatmap)
    { 
        g.setColour(getHeatMapColor(channelRates));
    }
    else
    {
        g.setColour(Colours::red);
    }

    for (auto& kv : flashingflag)
    {
        int ch = kv.first;
        bool flash = kv.second;
        
        if (flash && screenCoordinates.find(ch) != screenCoordinates.end())
        {
            g.fillRect(screenCoordinates[ch].first + margin,
                      screenCoordinates[ch].second + margin,
                      electrode_width - 2 * margin,
                      electrode_height - 10 * margin);
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
    int64 windowStart = currentTime - 1000;

    for (auto& [channelId, timestamps] : spikeTimestamps)
    {
        while (!timestamps.empty() && timestamps.front() < windowStart)
        {
            timestamps.pop_front();
        }

        float weightedSpikes = 0.0f;
        
        // Calculate weighted sum with exponential decay
        for (const auto& timestamp : timestamps)
        {
            float timeDiff = (currentTime - timestamp) / 1000.0f; // Convert to seconds
            float weight = std::exp(-timeDiff); // Exponential decay with time constant of 1 second
            weightedSpikes += weight;
        }

        channelRates[channelId] = weightedSpikes;
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

Colour RateViewerCanvas::getHeatMapColor(const std::map<int, float>& rates)
{
    float maxRate_ = 0.0f;
    for (const auto& [_, rate] : rates)
    {
        maxRate_ = std::max(maxRate_, rate);
    }

    if (maxRate_ >= maxRate){
        maxRate_ = maxRate;
    }
    
    float totalRate = 0.0f;
    for (const auto& [_, rate] : rates)
    {
        if(rate > maxRate){
            totalRate += maxRate;
        }
        else{
            totalRate += rate;
        }
        
    }
    float avgRate = totalRate / rates.size();

    float normalizedRate = avgRate / maxRate_;
    float hue = (1.0f - normalizedRate) * 0.7f;
    return Colour::fromHSV(hue, 1.0f, 1.0f, 1.0f);
}

