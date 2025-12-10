#include "HeaderEditor.h"

namespace gui
{
	HeaderEditor::HeaderEditor(ColoursEditor& coloursEditor,
		ManifestOfWisdom& manifest, patch::Browser& patchBrowser,
		Prompt& prompt) :
		Comp(manifest.utils),
		title(utils, utils.audioProcessor.pluginRecorder),
		buttonBrowser(patchBrowser),
		buttonPrev(patchBrowser, false),
		buttonNext(patchBrowser, true),
		buttonSave(patchBrowser),
		macro(utils),
		ioEditor(utils),
		buttonColours(coloursEditor),
		buttonManifest(manifest),
		buttonRandomizer(utils, "randall")
#if PPDHasMTSESP
		, mtsesp(utils)
#endif
#if PPDHasStereoConfig
		, stereoConfig(utils)
#endif
	{
		layout.init
		(
			{ 12, 1, 4, 1, 1, 4, 6, 2, 2 },
			{ 1, 1 }
		);

		addAndMakeVisible(title);
		addAndMakeVisible(macro);
		addAndMakeVisible(ioEditor);
		addAndMakeVisible(buttonPrev);
		addAndMakeVisible(buttonNext);
		addAndMakeVisible(buttonSave);
		addAndMakeVisible(buttonBrowser);
		addAndMakeVisible(buttonColours);
		addAndMakeVisible(buttonManifest);
		addAndMakeVisible(buttonRandomizer);
#if PPDHasMTSESP
		initMTSButton(prompt);
#endif
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
		layout.place(buttonSave, 3, 0, 1, 2);
		layout.place(buttonNext, 4, 0, 1, 2);
		layout.place(macro, 5, 0, 1, 2);
		layout.place(ioEditor, 6, 0, 1, 2);
		layout.place(buttonColours, 7, 0, 1, 1);
		layout.place(buttonManifest, 7, 1, 1, 1);
#if PPDHasStereoConfig
		layout.place(buttonRandomizer, 8, 0, 1, 1);
		layout.place(stereoConfig, 8, 1, 1, 1);
#else
#if PPDHasMTSESP
		layout.place(mtsesp, 8, 0, 1, 1);
		layout.place(buttonRandomizer, 8, 1, 1, 1);
#else
		layout.place(buttonRandomizer, 8, 0, 1, 2);
#endif
		
#endif
	}

#if PPDHasMTSESP
	void HeaderEditor::initMTSButton(Prompt& prompt)
	{
		addAndMakeVisible(mtsesp);
		mtsesp.type = Button::Type::kTrigger;
		makeTextButton(mtsesp, "MTS-ESP", "Open additional MTS-ESP settings.", CID::Interact, Colour(0x00000000));
		mtsesp.onClick = [&](const Mouse&)
		{
			if (prompt.isVisible())
				return prompt.deactivate();
			const auto scaleName = utils.audioProcessor.tuneSys.getScaleName();
			const auto masterExists = utils.audioProcessor.tuneSys.hasMaster();
			PromptData pd;
			pd.message = "MTS-ESP enables you to compose microtonally.\nHere are additional features and information.\nCurrent scale name: " + String(scaleName) + ".\nMaster: " + String(masterExists ? "active" : "inactive") + ".";
			{
				PromptButtonData pbd;
				pbd.text = "Update";
				pbd.tooltip = "This link leads you to the MTS-ESP installers.";
				pbd.onClick = [&]()
				{
					auto& user = utils.getProps();
					user.setValue("mtsinterest", true);
					URL url("https://github.com/ODDSound/MTS-ESP/tree/main/libMTS");
					url.launchInDefaultBrowser();
					prompt.deactivate();
				};
				pd.buttons.push_back(pbd);
			}
			{
				PromptButtonData pbd;
				pbd.text = "Close";
				pbd.tooltip = "This incredible button closes the menu!";
				pbd.onClick = [&]()
				{
					prompt.deactivate();
				};
				pd.buttons.push_back(pbd);
			}
			
			prompt.activate(pd);
		};
	}
#endif
}