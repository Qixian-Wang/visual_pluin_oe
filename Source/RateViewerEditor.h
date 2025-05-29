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

//This prevents include loops. We recommend changing the macro to a name suitable for your plugin
#ifndef VISUALIZERPLUGINEDITOR_H_DEFINED
#define VISUALIZERPLUGINEDITOR_H_DEFINED

#include <VisualizerEditorHeaders.h>
#include <fstream>
#include <map>

/** 
	The editor for the VisualizerPlugin

	Includes buttons for opening the canvas in a tab or window
*/

class RateViewerEditor : public VisualizerEditor,
						 public ComboBox::Listener,
						 public Button::Listener,
                         public FilenameComponentListener
{
   public:

      	/** Constructor */
		RateViewerEditor(GenericProcessor* parentNode);

		/** Destructor */
		~RateViewerEditor();

		/** Creates the canvas */
		Visualizer* createNewCanvas() override;

		void comboBoxChanged(ComboBox* comboBox) override;
		void buttonClicked(Button* button) override;
		void filenameComponentChanged(FilenameComponent* fileComponentThatHasChanged) override;
		
   	private:
        std::ofstream debugLogFile;
        void writeToDebugLog(const String& message);
        void initDebugLog();
        void loadYamlFile(const String& filename);

		std::unique_ptr<ComboBox> electrodelayout;
		std::unique_ptr<ToggleButton> heatmapToggle;
		std::unique_ptr<TextButton> loadFileButton;
		std::unique_ptr<FilenameComponent> fileChooser;
		std::map<int, String> layoutFiles;

		/** Generates an assertion if this class leaks */
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RateViewerEditor);
};

#endif // VISUALIZERPLUGINEDITOR_H_DEFINED