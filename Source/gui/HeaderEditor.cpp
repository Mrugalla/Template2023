#include "HeaderEditor.h"

namespace gui
{
	HeaderEditor::HeaderEditor(ColoursEditor& coloursEditor,
		ManifestOfWisdom& manifest, patch::Browser& patchBrowser) :
		Comp(manifest.utils),
		title(utils, utils.audioProcessor.pluginRecorder),
		buttonPrev(patchBrowser, false),
		buttonNext(patchBrowser, true),
		buttonBrowser(patchBrowser),
		macro(utils),
		ioEditor(utils),
		buttonColours(coloursEditor),
		buttonManifest(manifest),
		buttonRandomizer(utils, "randall")
#if PPDHasStereoConfig
		, stereoConfig(utils)
#endif
	{
		layout.init
		(
			{ 16, 1, 4, 1, 4, 6, 2, 2 },
			{ 1, 1 }
		);

		addAndMakeVisible(title);
		addAndMakeVisible(macro);
		addAndMakeVisible(ioEditor);
		addAndMakeVisible(buttonPrev);
		addAndMakeVisible(buttonNext);
		addAndMakeVisible(buttonBrowser);
		addAndMakeVisible(buttonColours);
		addAndMakeVisible(buttonManifest);
		addAndMakeVisible(buttonRandomizer);
#if PPDHasStereoConfig
		addAndMakeVisible(stereoConfig);
#endif

		makeTextLabel(title, "Plugin Titel", font::headline(), Just::centred, CID::Txt);
		title.autoMaxHeight = true;

		buttonRandomizer.add(utils.getAllParams());
#if PPDHasStereoConfig
		makeButton(PID::StereoConfig, stereoConfig, Button::Type::kChoice, "l/r;m/s");
#endif
	}

	void HeaderEditor::resized()
	{
		Comp::resized();
		layout.place(title, 0, 0, 1, 2);
		layout.place(buttonPrev, 1, 0, 1, 2);
		layout.place(buttonBrowser, 2, 0, 1, 2);
		layout.place(buttonNext, 3, 0, 1, 2);
		layout.place(macro, 4, 0, 1, 2);
		layout.place(ioEditor, 5, 0, 1, 2);
		layout.place(buttonColours, 6, 0, 1, 1);
		layout.place(buttonManifest, 6, 1, 1, 1);
#if PPDHasStereoConfig
		layout.place(buttonRandomizer, 7, 0, 1, 1);
		layout.place(stereoConfig, 7, 1, 1, 1);
#else
		layout.place(buttonRandomizer, 7, 0, 1, 2);
#endif
	}
}