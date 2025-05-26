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

#include "RateViewer.h"


RateViewerCanvas::RateViewerCanvas(RateViewer* processor_)
	: processor(processor_)
{
	plt.setBounds(5, 5, 1500, 1000);
}


RateViewerCanvas::~RateViewerCanvas()
{

}


void RateViewerCanvas::resized()
{
    auto plotArea = plt.getBounds().toFloat();

    const float margin = 50.0f;
    plotArea = plotArea.reduced(margin);

    const int numRows = 8;
    const int numCols = 16;
    dx = plotArea.getWidth()  / (numCols - 1);
    dy = plotArea.getHeight() / (numRows);
    const float radius = 15.0f;

	int electrode_index = 0;
    for (int row = 0; row < numRows; ++row)
    {
        for (int col = 0; col < numCols; ++col)
        {
            float x = plotArea.getX() + col * dx;
            float y = plotArea.getY() + row * dy;	
			electrode_map[electrode_index] = {x, y};
			++ electrode_index;
        }

    }

	while (electrodeLabels.size() < numRows * numCols)
		electrodeLabels.add(new Label());
	while (electrodeLabels.size() > numRows * numCols)
		electrodeLabels.removeLast();

	for (int idx = 0; idx < numRows * numCols; ++idx)
	{
    auto* rate_text = electrodeLabels[idx];
    rate_text->setJustificationType(Justification::centred);
    rate_text->setFont(Font(20.0f));
    rate_text->setColour(Label::textColourId, Colours::white);

    auto [x,y] = electrode_map[idx];
    rate_text->setBounds((int)(x - dx/2),
                   (int)(y + radius + 2),
                   (int)dx,
                   (int)rate_text->getFont().getHeight());

    addAndMakeVisible(rate_text);
	}
}

void RateViewerCanvas::refreshState()
{

}

void RateViewerCanvas::paintOverChildren(Graphics& g)
{
    drawElectrodeArray(g);
    for (auto& kv : flashingflag)
    {
        int   ch   = kv.first;
        bool  flash= kv.second;
        auto [x, y] = electrode_map[ch];
		const float r = 20.0f;

        if (flash)
		{
			g.setColour(Colours::red);
			g.drawEllipse(x - r, y - r, r*2.0f, r*2.0f, 2.0f);
		}

    }
	
}

void RateViewerCanvas::drawElectrodeArray(Graphics& g)
{
	const float radius = 15.0f;

    g.setColour(Colours::white.withAlpha(0.8f));
	int idx = 0;
	for (auto& kv : electrode_map)
	{
		g.fillEllipse(kv.second.first - radius,
			kv.second.second - radius,
			radius * 2.0f, radius * 2.0f);
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

void RateViewerCanvas::addSpike(int channelId, int64 sample_num)
{
   	incomingSpikesPerChannel[channelId].push_back(sample_num);
	flashingflag[channelId] = true;
	flashEndTime[channelId] = Time::getMillisecondCounter() + 500;
	repaint();
}

void RateViewerCanvas::setMostRecentSample(int64 sampleNum)
{
//    mostRecentSample = sampleNum;
}

bool RateViewerCanvas::countSpikes()
{

	int elapsedSamples = mostRecentSample - sampleOnLastRedraw;
	float elapsedTimeMs = float(elapsedSamples) / sampleRate * 1000.0f;

	// Only count spikes when the time since the last count is greater than the bin size
	if (elapsedTimeMs < binSize)
		return false;

	spikeCounts.remove(0); // remove oldest count

	int newSpikeCount = incomingSpikesPerChannel.size();

	if (newSpikeCount > maxCount)
		maxCount = newSpikeCount;

	spikeCounts.add(newSpikeCount); // add most recent count

	incomingSpikesPerChannel.clear();
	sampleOnLastRedraw = mostRecentSample;

	return true;
}

void RateViewerCanvas::updateElectrodeLabels()
{
  int total = electrodeLabels.size();
  for (int i = 0; i < total; ++i)
  {
    float rate = channelRates[i];
    electrodeLabels[i]->setText(String(rate, 1) + " Hz",
                               NotificationType::dontSendNotification);
  }
}

void RateViewerCanvas::refresh()
{
    for (auto& kv : incomingSpikesPerChannel)
    {
        int   ch    = kv.first;
        auto& spikes= kv.second; 
        int   count = (int)spikes.size();
        float rate  = count * 1000.0f / binSize;
        channelRates[ch] = rate;
    }

	auto now = Time::getMillisecondCounter();
    for (auto it = flashEndTime.begin(); it != flashEndTime.end(); )
    {
        if (now >= it->second)
        {
            flashingflag[it->first] = false;
            it = flashEndTime.erase(it);
        }
        else ++it;
    }
    incomingSpikesPerChannel.clear();
    updateElectrodeLabels();
	repaint();
}

void RateViewerCanvas::setCoords(const std::vector<juce::Point<float>>& newCoords)
{
    electrode_map.clear();
    for (size_t i = 0; i < newCoords.size(); ++i)
        electrode_map[(int)i] = { newCoords[i].x, newCoords[i].y };
    resized();
    repaint();
}
