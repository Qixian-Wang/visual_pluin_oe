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


#include "RateViewerEditor.h"
#include "RateViewerCanvas.h"
#include "RateViewer.h"

#include <yaml-cpp/yaml.h>
#include <juce_core/juce_core.h>

RateViewerEditor::RateViewerEditor(GenericProcessor* p)
    : VisualizerEditor(p, "Rate Viewer", 210)
{
    electrodelayout = std::make_unique<ComboBox>("Electrode Layout List");
    electrodelayout->addListener(this);
    electrodelayout->setBounds(50,40,120,20);

    electrodelayout->addItem("64_intanRHD", 1);
    electrodelayout->addItem("32-electrode RHD", 2);
    electrodelayout->addItem("Custom layout",    3);
    electrodelayout->addListener(this);

    addAndMakeVisible(electrodelayout.get());
}

Visualizer* RateViewerEditor::createNewCanvas()
{
    RateViewer* rateViewerNode = (RateViewer*) getProcessor();
    RateViewerCanvas* rateViewerCanvas = new RateViewerCanvas(rateViewerNode);
    rateViewerNode->canvas = rateViewerCanvas;

    return rateViewerCanvas;
}

void RateViewerEditor::comboBoxChanged(ComboBox* comboBox)
{
    if (comboBox->getSelectedId() == 1)
    {
        juce::URL yamlUrl ("https://github.com/GazzolaLab/MiV-OS/blob/main/miv/mea/electrodes/64_intanRHD.yaml");
        juce::String yamlText = yamlUrl.readEntireTextStream();
        YAML::Node config = YAML::Load(yamlText.toStdString());

        for (auto node : config["pos"])
        {
            if (node.IsSequence() && node.size() >= 2)
            {
                float x = node[0].as<float>();
                float y = node[1].as<float>();
                coords.emplace_back(x, y);
            }
        }
        if (auto* rv = dynamic_cast<RateViewer*>(getProcessor()))
        {
            if (auto* c = rv->canvas)
            {
                c->setCoords(coords);
            }
        }
    }
}