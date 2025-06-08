#pragma once
#include "LabelPluginRecorder.h"
#include "MacroEditor.h"
#include "IOEditor.h"
#include "ManifestOfWisdom.h"
#include "ColoursEditor.h"

namespace gui
{
	struct HeaderEditor :
		public Comp
	{
		HeaderEditor(ColoursEditor& coloursEditor, ManifestOfWisdom& manifest) :
			Comp(manifest.utils),
			title(utils, utils.audioProcessor.pluginRecorder),
			macro(utils),
			ioEditor(utils),
			buttonColours(coloursEditor),
			buttonManifest(manifest)
		{
			layout.init
			(
				{ 16, 4, 4, 2 },
				{ 1, 1 }
			);

			addAndMakeVisible(title);
			addAndMakeVisible(macro);
			addAndMakeVisible(ioEditor);
			addAndMakeVisible(buttonColours);
			addAndMakeVisible(buttonManifest);

			makeTextLabel(title, "Plugin Titel", font::headline(), Just::centred, CID::Txt);
			title.autoMaxHeight = true;
		}

		void resized() override
		{
			Comp::resized();
			layout.place(title, 0, 0, 1, 2);
			layout.place(macro, 1, 0, 1, 2);
			layout.place(ioEditor, 2, 0, 1, 2);
			layout.place(buttonColours, 3, 0, 1, 1);
			layout.place(buttonManifest, 3, 1, 1, 1);
		}
	private:
		LabelPluginRecorder title;
		MacroEditor macro;
		IOEditor ioEditor;
		ButtonColours buttonColours;
		ButtonWisdom buttonManifest;
	};
}