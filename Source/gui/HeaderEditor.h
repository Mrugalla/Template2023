#pragma once
#include "LabelPluginRecorder.h"
#include "PatchBrowser.h"
#include "MacroEditor.h"
#include "IOEditor.h"
#include "ManifestOfWisdom.h"
#include "ColoursEditor.h"
#include "ButtonRandomizer.h"

namespace gui
{
	struct HeaderEditor :
		public Comp
	{
		HeaderEditor(ColoursEditor&, ManifestOfWisdom&, patch::Browser&);

		void resized() override;
	private:
		LabelPluginRecorder title;
		patch::BrowserButton buttonBrowser;
		patch::NextPatchButton buttonPrev, buttonNext;
		patch::ButtonSaveQuick buttonSave;
		MacroEditor macro;
		IOEditor ioEditor;
		ButtonColours buttonColours;
		ButtonWisdom buttonManifest;
		ButtonRandomizer buttonRandomizer;
#if PPDHasStereoConfig
		Button stereoConfig;
#endif
	};
}