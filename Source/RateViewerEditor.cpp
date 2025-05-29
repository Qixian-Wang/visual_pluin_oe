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

#include "../../../plugin-GUI/Source/Utils/Utils.h"

RateViewerEditor::RateViewerEditor(GenericProcessor* p)
    : VisualizerEditor(p, "Rate Viewer", 210)
{
    electrodelayout = std::make_unique<ComboBox>("Electrode Layout List");
    electrodelayout->addListener(this);
    electrodelayout->setBounds(100,50,100,20);
    addAndMakeVisible(electrodelayout.get());

    loadFileButton = std::make_unique<TextButton>("Load Layout");
    loadFileButton->addListener(this);
    loadFileButton->setBounds(15, 50, 75, 20);
    addAndMakeVisible(loadFileButton.get());

    fileChooser = std::make_unique<FilenameComponent>("File Chooser",
                                                     File(),
                                                     false,
                                                     false,
                                                     false,
                                                     "*.yaml",
                                                     String(),
                                                     "Choose a layout file");
    fileChooser->addListener(this);
    fileChooser->setBounds(50, 110, 120, 20);
    fileChooser->setVisible(false);

    addTextBoxParameterEditor("window_size", 15, 70);
    addTextBoxParameterEditor("max_rate", 120, 70);
 
    heatmapToggle = std::make_unique<ToggleButton>("Heatmap");
    heatmapToggle->addListener(this);
    heatmapToggle->setBounds(70, 25, 100, 20);
    heatmapToggle->setToggleState(false, juce::dontSendNotification);
    addAndMakeVisible(heatmapToggle.get());

    initDebugLog();
}

RateViewerEditor::~RateViewerEditor()
{
    if (debugLogFile.is_open())
        debugLogFile.close();
}

void RateViewerEditor::initDebugLog()
{
    Time now = Time::getCurrentTime();
    String timestamp = now.formatted("%Y%m%d_%H%M%S");
    
    File logDir = File::getSpecialLocation(File::userDocumentsDirectory)
                    .getChildFile("Open Ephys")
                    .getChildFile("RateViewer_logs");
    
    if (!logDir.exists())
        logDir.createDirectory();
    
    File logFile = logDir.getChildFile("rateviewer_debug_" + timestamp + ".log");
    debugLogFile.open(logFile.getFullPathName().toStdString(), std::ios::out | std::ios::app);
    
    if (debugLogFile.is_open())
    {
        writeToDebugLog("=== Log Started at " + now.toString(true, true, true, true) + " ===");
    }
}

void RateViewerEditor::writeToDebugLog(const String& message)
{
    if (debugLogFile.is_open())
    {
        debugLogFile << " - " << message.toStdString() << std::endl;
        debugLogFile.flush(); 
    }
}

Visualizer* RateViewerEditor::createNewCanvas()
{
    RateViewer* rateViewerNode = (RateViewer*) getProcessor();
    RateViewerCanvas* rateViewerCanvas = new RateViewerCanvas(rateViewerNode);
    rateViewerNode->canvas = rateViewerCanvas;
    rateViewerCanvas->setWindowSizeMs(rateViewerNode->getParameter("window_size")->getValue());
    rateViewerCanvas->setMaxRate(rateViewerNode->getParameter("max_rate")->getValue());
    return rateViewerCanvas;
}

void RateViewerEditor::buttonClicked(Button* button)
{
    if (button == heatmapToggle.get())
    {
        RateViewer* RateViewerNode = (RateViewer*) getProcessor();
        RateViewerCanvas* canvas = (RateViewerCanvas*) RateViewerNode->canvas;
        if (canvas != nullptr)
        {
            canvas->setUseHeatmap(heatmapToggle->getToggleState());
            canvas->repaint();
        }
    }
    else if (button == loadFileButton.get())
    {
        FileChooser chooser("Select a YAML layout file...",
                          File::getSpecialLocation(File::userHomeDirectory),
                          "*.yaml");
                          
        if (chooser.browseForFileToOpen())
        {
            File yamlFile = chooser.getResult();
            loadYamlFile(yamlFile.getFullPathName());
            
            // Add the file to the combobox if it's not already there
            String filename = yamlFile.getFileName();
            bool exists = false;
            for (int i = 0; i < electrodelayout->getNumItems(); i++)
            {
                if (electrodelayout->getItemText(i) == filename)
                {
                    exists = true;
                    break;
                }
            }
            
            if (!exists)
            {
                int itemId = electrodelayout->getNumItems() + 1;
                electrodelayout->addItem(filename, itemId);
                layoutFiles[itemId] = yamlFile.getFullPathName();
                electrodelayout->setSelectedId(itemId, sendNotification);
            }
        }
    }
}

void RateViewerEditor::comboBoxChanged(ComboBox* comboBox)
{
    int selectedId = comboBox->getSelectedId();

    String fullPath = layoutFiles[selectedId];
    File yamlFile(fullPath);
    if (yamlFile.existsAsFile())
    {
        loadYamlFile(fullPath);
    }
}

void RateViewerEditor::loadYamlFile(const String& filename)
{
    
    File yamlFile(filename); 
    YAML::Node config = YAML::LoadFile(filename.toStdString());

    auto pos_node = config["pos"];

    int nodeIndex = 0;
    if (auto* rv = dynamic_cast<RateViewer*>(getProcessor()))
    {
        if (auto* c = rv->canvas)
        {
            c->electrode_map.clear();
            c->electrodeLabels.clear();
            for (auto node : pos_node)
            {
                if (node[0].IsNull() || node[1].IsNull()) 
                {
                    continue;
                } 
                else {
                    float x = node[0].as<float>();
                    float y = node[1].as<float>();
                    c->electrode_map[nodeIndex] = {x, y};
                    c->electrodeLabels.add(new Label());
                }
                nodeIndex++;
            }     
        }
    }

    if (auto* rv = dynamic_cast<RateViewer*>(getProcessor()))
    {
        if (auto* c = rv->canvas)
        {
            c->setWindowSizeMs(1000);
        }
    }
}

void RateViewerEditor::filenameComponentChanged(FilenameComponent* fileComponentThatHasChanged)
{
    if (fileComponentThatHasChanged == fileChooser.get())
    {
        File selectedFile = fileChooser->getCurrentFile();
        if (selectedFile.existsAsFile())
        {
            loadYamlFile(selectedFile.getFullPathName());
        }
    }
}
