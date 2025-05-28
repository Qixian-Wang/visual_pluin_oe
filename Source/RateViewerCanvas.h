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

#ifndef VISUALIZERCANVAS_H_INCLUDED
#define VISUALIZERCANVAS_H_INCLUDED

#include <VisualizerWindowHeaders.h>
#include <JuceHeader.h>

class RateViewer;

/**
* 
	Draws data in real time

*/
class RateViewerCanvas : public Visualizer
{
public:

	/** Constructor */
	RateViewerCanvas(RateViewer* processor);

	/** Destructor */
	~RateViewerCanvas();

	/** Updates boundaries of sub-components whenever the canvas size changes */
	void resized() override;

	/** Called when the visualizer's tab becomes visible again */
	void refreshState() override;

	/** Updates settings */
	void update() override;

	/** Called instead of "repaint()" to avoid re-painting sub-components*/
	void refresh() override;

	/** Draws the canvas background */
	void paint(Graphics& g) override;

	/** Change the plot title*/
	void setPlotTitle(const String& title);

	/** Adds a spike sample number */
	void addSpike(int channelId);

	void paintOverChildren(Graphics& g) override;

	void setWindowSizeMs(int windowSize_);
    void setBinSizeMs(int binSize_);
	
	OwnedArray<Label> electrodeLabels;

	std::map<int, std::pair<float, float>> electrode_map;

private:
	/** Pointer to the processor class */
	RateViewer* processor;

	/** Class for plotting data */
	InteractivePlot plt;

	/** Generates an assertion if this class leaks */
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RateViewerCanvas);

	void updateElectrodeLabels();
	void updateLayout();

	int windowSize = 1000;  // ms
	int binSize = 50;      // ms

	std::map<int, std::deque<int64_t>> spikeTimestamps;
	std::map<int, float> channelRates;
	std::map<int,uint32_t> flashEndTime;
    std::map<int,bool> flashingflag;
	std::map<int, std::pair<float, float>> screenCoordinates;
	
	Image electrodeImage;
};

#endif // SPECTRUMCANVAS_H_INCLUDED