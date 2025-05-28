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
    electrodelayout->setBounds(50,40,120,20);

    electrodelayout->addItem("64_intanRHD", 1);
    electrodelayout->addItem("32-electrode RHD", 2);
    electrodelayout->addItem("Custom layout",    3);
    electrodelayout->addListener(this);

    addAndMakeVisible(electrodelayout.get());

    addTextBoxParameterEditor("window_size", 15, 75);
    addTextBoxParameterEditor("bin_size", 120, 75);

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
    rateViewerCanvas->setBinSizeMs(rateViewerNode->getParameter("bin_size")->getValue());
    return rateViewerCanvas;
}

void RateViewerEditor::comboBoxChanged(ComboBox* comboBox)
{
    writeToDebugLog("ComboBox changed, selected ID: " + String(comboBox->getSelectedId()));
    
    if (comboBox->getSelectedId() == 1)
    {
        std::string filename = "/Users/aia/Downloads/64_intanRHD.yaml";
        writeToDebugLog("Attempting to load YAML file: " + String(filename));
        
        try {
            File yamlFile(filename);
            if (!yamlFile.exists()) {
                writeToDebugLog("ERROR: YAML file does not exist: " + String(filename));
                return;
            }
            
            writeToDebugLog("File exists, attempting to parse YAML...");
            YAML::Node config = YAML::LoadFile(filename);
            
            if (!config) {
                writeToDebugLog("ERROR: Failed to load YAML file or file is empty");
                return;
            }
            
            writeToDebugLog("YAML loaded successfully");
            
            if (!config["pos"]) {
                writeToDebugLog("ERROR: YAML file does not contain 'pos' key");
                return;
            }
            
            writeToDebugLog("Found 'pos' key in YAML");
            
            auto pos_node = config["pos"];
            writeToDebugLog("pos node type: " + String(pos_node.Type()) + ", size: " + String(pos_node.size()));
    
            int nodeIndex = 0;
            
            for (auto node : pos_node)
            {
                writeToDebugLog("Processing node " + String(nodeIndex));
                
                if (node[0].IsNull() || node[1].IsNull()) 
                {
                    continue;
                } 
                else {
                    float x = node[0].as<float>();
                    float y = node[1].as<float>();
                    
                    if (auto* rv = dynamic_cast<RateViewer*>(getProcessor()))
                    {
                        if (auto* c = rv->canvas)
                        {
                            c->electrode_map[nodeIndex] = {x, y};
                            c->electrodeLabels.add(new Label());
                            writeToDebugLog("Added coordinate pair: (" + String(x) + ", " + String(y) + ")");
                        }
                    }
                }
                     
                nodeIndex++;
            }
            
            writeToDebugLog("Successfully processed coordinates");
            
            if (auto* rv = dynamic_cast<RateViewer*>(getProcessor()))
            {
                if (auto* c = rv->canvas)
                {
                    c->setWindowSizeMs(1000);
                }
            }
            
        } catch (const YAML::Exception& e) {
            writeToDebugLog("ERROR: YAML Exception: " + String(e.what()));
        } catch (const std::exception& e) {
            writeToDebugLog("ERROR: Standard Exception: " + String(e.what()));
        } catch (...) {
            writeToDebugLog("ERROR: Unknown exception occurred while processing YAML file");
        }
    }
}