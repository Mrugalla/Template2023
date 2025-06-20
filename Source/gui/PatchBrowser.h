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
			SelectionComp(Utils&);

			void paint(Graphics&) override;

			void select(Patch*) noexcept;

			Patch* selected;
			String name, author;
		};

		struct Patches :
			public Comp
		{
			using UpdatedFunc = std::function<void()>;

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
			UpdatedFunc onUpdate;
		};

		using PatchesUpdatedFunc = Patches::UpdatedFunc;

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

			~Browser();

			void setVisible(bool) override;

			void paint(Graphics&) override;

			// next
			void nextPatch(bool);

			const Patch* getSelectedPatch() const noexcept;

			Patch* getSelectedPatch() noexcept;

			void resized() override;

			void overwriteSelectedPatch();

			PatchesUpdatedFunc& getOnUpdate() noexcept;

			// fromSelected
			void save(bool);
		protected:
			Label title;
			TextEditor editorAuthor, editorName;
			Patches patches;
			ButtonSavePatch saveButton;
			ButtonReveal revealButton;
			String nameText, authorText;
		};

		// Quick Access Buttons:

		struct BrowserButton :
			public Button
		{
			enum cb { kPresetNameAni, kNumAnis };

			BrowserButton(Browser&);

			bool patchTweaked, reportUpdate;
		};

		struct ButtonSaveQuick :
			public Button
		{
			// browser
			ButtonSaveQuick(Browser&);
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

style: rewrite everything to use the word preset instead of patch

feature: authors are saved in author list (all plugins)
	a list of all authors can be shown for quick access
	quick workflow for removing all presets from 1 author
		or renaming all author names of presets of 1 author (for exmpl to consolidate presets)
*/