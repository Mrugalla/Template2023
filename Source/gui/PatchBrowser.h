#pragma once
#include "TextEditor.h"
#include "../arch/State.h"

namespace gui
{
	namespace patch
	{
		static constexpr int NumPatches = 12;

		struct Patch :
			public Comp
		{
			Patch(Utils&);

			// name, author, file
			void activate(const String&, const String&, const File&);

			void deactivate();

			void resized() override;

			String name, author;
			File file;
			Button buttonLoad, buttonDelete;
		};

		struct ScrollBar :
			public Comp
		{
			ScrollBar(Utils&);

			void paint(Graphics&) override;

			void mouseWheelMove(const Mouse&, const MouseWheel&) override;

			std::function<void()> onScroll;
			int viewIdx, numFiles;
		};

		struct SelectionComp :
			public Comp
		{
			SelectionComp(Utils& u) :
				Comp(u),
				selected(nullptr)
			{
				setInterceptsMouseClicks(false, false);
			}

			void paint(Graphics& g) override
			{
				setCol(g, CID::Mod);
				g.fillEllipse(getLocalBounds().toFloat().reduced(utils.thicc * 4.f));
			}

			void select(Patch* p) noexcept
			{
				selected = p;
				setVisible(p != nullptr);
			}

			Patch* selected;
		};

		struct Patches :
			public Comp
		{
			Patches(Utils&);

			void resized() override;

			// name, author
			void updateFilter(const String&, const String&);

			void update();

			// file, i
			void updateAdd(const File&, int);

			const Patch& operator[](int) const noexcept;

			Patch& operator[](int) noexcept;

			const Patch* getSelected() const noexcept;

			Patch* getSelected() noexcept;

			void select(const String& author, const String& name) noexcept;

			const size_t size() const noexcept;

			void mouseWheelMove(const Mouse&, const MouseWheel&) override;

			// next
			void nextPatch(bool);
		private:
			std::array<Patch, NumPatches> patches;
			SelectionComp selection;
			ScrollBar scrollBar;
			Int64 directorySize;
			String filterName, filterAuthor;

			void resizePatches(float, float);
		public:
			std::function<void(const Patches&)> onUpdate;
		};

		struct ButtonSavePatch :
			public Button
		{
			// patches, name, author
			ButtonSavePatch(Patches&, TextEditor&, TextEditor&);
		};

		struct ButtonReveal :
			public Button
		{
			ButtonReveal(Patches&);
		};

		struct Browser :
			public Comp
		{
			Browser(Utils&);

			void setVisible(bool) override;

			void paint(Graphics&) override;

			// next
			void nextPatch(bool);

			const Patch* getSelectedPatch() const noexcept;

			Patch* getSelectedPatch() noexcept;

			void resized() override;

			std::function<void(const Patches&)>& getOnUpdate() noexcept
			{
				return patches.onUpdate;
			}
		protected:
			Label title;
			TextEditor editorAuthor, editorName;
			Patches patches;
			ButtonSavePatch saveButton;
			ButtonReveal revealButton;
			String nameText, authorText;
		};

		struct BrowserButton :
			public Button
		{
			enum cb { kPresetNameAni, kNumAnis };

			BrowserButton(Browser&);

			bool patchTweaked;
		};

		struct NextPatchButton :
			public Button
		{
			// browser, next
			NextPatchButton(Browser&, bool);
		};
	}
}

/*
bug: when selecting another patch, patch browser button needs to update & repaint

bug: select patch, then scroll out of its visbility and scroll back, wrong patch selected!

feature: save button near patch browser button for quick resave of selected patch

feature: when trying to save and file already exists, ask if overwrite ok

feature: authors are saved in author list (all plugins)
	a list of all authors can be shown for quick access
	quick workflow for removing all presets from 1 author
		or renaming all author names of presets of 1 author (for exmpl to consolidate presets)

feature: when new preset made serialize preset name
*/