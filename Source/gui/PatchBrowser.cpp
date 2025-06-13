#include "PatchBrowser.h"

namespace gui
{
	File getPatchesDirectory(const Utils& u)
	{
		const auto& user = *u.audioProcessor.state.props.getUserSettings();
		const auto& settingsFile = user.getFile();
		const auto settingsDirectory = settingsFile.getParentDirectory();
		return settingsDirectory.getChildFile("Patches");
	}

	Int64 getDirectorySize(const File& directory)
	{
		const auto wildcard = "*.txt";
		const auto type = File::TypesOfFileToFind::findFiles;
		const RangedDirectoryIterator iterator
		(
			directory,
			true,
			wildcard,
			type
		);

		Int64 size = 0;
		for (const auto& it : iterator)
			size += it.getFileSize();
		return size;
	}

	String generateWildcard(const String& filter)
	{
		if (filter.isEmpty())
			return "*.txt";
		String wildcard("*");
		wildcard += filter + "*.txt";
		const char charactersToReplace[] =
		{
			' ', '.', '-', '_', '(', ')', '\'', '\"', '!', '?', ',', ';', ':', '=', '+',
			'/', '\\', '|', '~', '`', '@', '#', '$', '%', '^', '&'
		};
		for (const char c : charactersToReplace)
			wildcard = wildcard.replaceCharacter(c, '*');
		return wildcard;
	}

	bool isInAuthor(const String& author, const String& filter)
	{
		if (filter.isEmpty())
			return true;
		const auto authorLC = author.toLowerCase();
		const auto filterLC = filter.toLowerCase();
		const auto words = StringArray::fromTokens(filterLC, " ", "");
		for (const auto& word : words)
			if (authorLC.contains(word))
				return true;
		return false;
	}

	namespace patch
	{
		// Patch

		Patch::Patch(Utils& u) :
			Comp(u),
			name(""),
			author(""),
			file(),
			buttonLoad(u),
			buttonDelete(u)
		{
			addAndMakeVisible(buttonLoad);
			addAndMakeVisible(buttonDelete);

			setInterceptsMouseClicks(false, true);
		}

		void Patch::activate(const String& _name, const String& _author, const File& _file)
		{
			name = _name;
			author = _author;
			file = _file;
			makeTextButton(buttonLoad, name + " by " + author, "Click here to load " + name + ".", CID::Interact);
			buttonLoad.label.font = font::text();
			makePaintButton(buttonDelete, [](Graphics& g, const Button& b)
				{
					const auto thicc = b.utils.thicc;
					const auto thicc3 = thicc * 3.f;

					const auto hoverPhase = b.callbacks[Button::kHoverAniCB].phase;

					const auto margin = thicc + thicc3 - hoverPhase * thicc3;
					const auto bounds = maxQuadIn(b.getLocalBounds().toFloat()).reduced(margin);

					const auto shrink = .2f;

					const auto w = bounds.getWidth();
					const auto wShrinked = hoverPhase * w * shrink;
					const auto h = bounds.getHeight();
					const auto y = bounds.getY();
					const auto x = bounds.getX() + wShrinked;
					const auto r = bounds.getRight() - wShrinked;

					setCol(g, CID::Interact);
					const auto lineThicc = thicc + hoverPhase * thicc;
					g.drawLine(x, y, r, y + h, lineThicc);
					g.drawLine(x, y + h, r, y, lineThicc);
				}, "Click here to remove " + name + ".");
			setVisible(true);
		}

		void Patch::deactivate()
		{
			setVisible(false);
		}

		void Patch::resized()
		{
			Comp::resized();
			const auto bounds = getLocalBounds().toFloat();
			const auto quad = maxQuadIn(bounds);
			const auto loadBounds = bounds.withWidth(bounds.getWidth() - quad.getWidth());
			buttonLoad.setBounds(loadBounds.toNearestInt());
			buttonLoad.label.setMaxHeight(utils.thicc);
			const auto deleteBounds = quad.withX(loadBounds.getRight());
			buttonDelete.setBounds(deleteBounds.toNearestInt());
		}

		// ScrollBar

		ScrollBar::ScrollBar(Utils& u) :
			Comp(u),
			onScroll(),
			viewIdx(0),
			numFiles(1)
		{
		}

		void ScrollBar::paint(Graphics& g)
		{
			g.fillAll(getColour(CID::Darken));
			const auto bounds = getLocalBounds().toFloat();
			const auto x = 0.f;
			const auto w = bounds.getWidth() * 1.2f;
			const auto h = bounds.getHeight() / static_cast<float>(numFiles);
			const auto y = h * static_cast<float>(viewIdx);
			g.setColour(getColour(CID::Hover));
			g.drawRect(x, y, w, h, utils.thicc);
		}

		void ScrollBar::mouseWheelMove(const Mouse&, const MouseWheel& wheel)
		{
			auto inc = wheel.deltaY > 0.f ? -1 : 1;
			if (wheel.isReversed)
				inc = -inc;
			viewIdx += inc;
			if (viewIdx < 0)
				viewIdx = 0;
			onScroll();
			repaint();
		}

		// Patches

		Patches::Patches(Utils& u) :
			Comp(u),
			patches
			{
				Patch(u), Patch(u), Patch(u), Patch(u),
				Patch(u), Patch(u), Patch(u), Patch(u),
				Patch(u), Patch(u), Patch(u), Patch(u)
			},
			selection(u),
			scrollBar(u),
			directorySize(getDirectorySize(getPatchesDirectory(u))),
			filterName(""),
			filterAuthor(""),
			onUpdate(nullptr)
		{
			for (auto& patch : patches)
				addChildComponent(patch);
			addAndMakeVisible(scrollBar);
			addChildComponent(selection);

			scrollBar.onScroll = [&]()
			{
				update();
				selection.select(nullptr);
				for (auto& patch : patches)
					if(patch.isVisible())
						if (patch.name == selection.name && patch.author == selection.author)
						{
							selection.select(&patch);
							break;
						}
				resized();
				repaint();
			};

			for (auto& patch : patches)
			{
				patch.buttonLoad.onWheel = [&](const Mouse& mouse, const MouseWheel& wheel)
				{
					scrollBar.mouseWheelMove(mouse, wheel);
				};
				patch.buttonDelete.onWheel = [&](const Mouse& mouse, const MouseWheel& wheel)
				{
					scrollBar.mouseWheelMove(mouse, wheel);
				};
			}

			update();

			Comp::add(Callback([&]()
			{
				if (!isShowing())
					return;
				const auto patchesDirectory = getPatchesDirectory(u);
				const auto nSize = getDirectorySize(patchesDirectory);
				if (directorySize == nSize)
					return;
				directorySize = nSize;
				update();
				resized();
				repaint();
			}, 0, cbFPS::k_1_875, true));
		}

		void Patches::resized()
		{
			Comp::resized();
			const auto bounds = getLocalBounds().toFloat();
			const auto w = bounds.getWidth();
			const auto h = bounds.getHeight();
			const auto patchHeight = h / static_cast<float>(NumPatches);
			const auto quad = maxQuadIn(bounds.withHeight(patchHeight));
			const auto patchesBounds = bounds.withWidth(w - quad.getWidth());
			const auto patchWidth = patchesBounds.getWidth();
			resizePatches(patchWidth, patchHeight);
			const auto scrollBarBounds = quad.withX(patchesBounds.getRight()).withHeight(h);
			scrollBar.setBounds(scrollBarBounds.toNearestInt());
			if (selection.selected)
			{
				const auto scrollX = scrollBarBounds.getX();
				const auto selBounds = selection.selected->getBounds().toFloat();
				const auto xWidth = static_cast<float>(selection.selected->buttonDelete.getWidth());
				auto selQuad = maxQuadIn(selBounds);
				const auto quadW = quad.getWidth();
				selQuad.setX(scrollX - xWidth - quadW);
				selection.setBounds(selQuad.toNearestInt());
			}
		}

		void Patches::resizePatches(float w, float h)
		{
			auto patchY = 0.f;
			auto j = 0;
			for (auto i = 0; i < NumPatches; ++i)
			{
				while (!patches[j].isVisible())
				{
					++j;
					if (j >= patches.size())
						return;
				}
				auto& patch = patches[j];
				patch.setBounds(BoundsF(0.f, patchY, w, h).toNearestInt());
				patch.resized();
				patchY += h;

				++j;
				if (j >= patches.size())
					return;
			}
		}

		void Patches::updateFilter(const String& _filterName, const String& _filterAuthor)
		{
			filterName = _filterName.toLowerCase();
			filterAuthor = _filterAuthor.toLowerCase();
			update();
			resized();
			repaint();
		}

		void Patches::update()
		{
			const auto patchesDirectory = getPatchesDirectory(utils);
			const auto wildcard = generateWildcard(filterName);
			const auto type = File::TypesOfFileToFind::findFiles;
			const RangedDirectoryIterator iterator
			(
				patchesDirectory,
				true,
				wildcard,
				type
			);

			const auto numFiles = patchesDirectory.getNumberOfChildFiles(type, wildcard);
			if (numFiles == 0)
			{
				scrollBar.numFiles = 1;
				scrollBar.repaint();
				for (auto idx = 0; idx < NumPatches; ++idx)
					patches[idx].deactivate();
				return;
			}
			if (scrollBar.numFiles != numFiles)
			{
				scrollBar.numFiles = numFiles;
				scrollBar.repaint();
			}
			if (scrollBar.viewIdx >= scrollBar.numFiles)
				scrollBar.viewIdx = scrollBar.numFiles - 1;

			auto idx = -scrollBar.viewIdx;
			for (const auto& it : iterator)
			{
				if (idx >= 0)
				{
					if (idx < NumPatches)
					{
						const auto file = it.getFile();
						updateAdd(file, idx);
					}
				}
				++idx;
			}
			for (; idx < NumPatches; ++idx)
				patches[idx].deactivate();
		}

		void Patches::updateAdd(const File& file, int i)
		{
			const auto state = ValueTree::fromXml(file.loadFileAsString());
			auto name = file.getFileName();
			name = name.substring(0, name.lastIndexOf("."));
			const auto author = state.getProperty("author", "");
			if (!isInAuthor(author, filterAuthor))
			{
				patches[i].deactivate();
				return;
			}
			patches[i].activate(name, author, file);
			patches[i].buttonLoad.onClick = [&, i](const Mouse&)
			{
				auto& p = patches[i];
				selection.select(&p);
				const auto& file = p.file;
				const auto vt = ValueTree::fromXml(file.loadFileAsString());
				if (!vt.isValid())
					return;
				auto& state = utils.audioProcessor.state;
				state.state = vt;
				utils.audioProcessor.params.loadPatch(state);
				utils.audioProcessor.pluginProcessor.loadPatch(state);
				resized();
				repaint();
				onUpdate(*this);
			};
			patches[i].buttonDelete.onClick = [&, i](const Mouse&)
			{
				if (patches[i].author == "factory")
					return;
				const auto file = patches[i].file;
				file.deleteFile();
				const auto directory = getPatchesDirectory(utils);
				directorySize = getDirectorySize(directory);
				patches[i].deactivate();
				update();
				resized();
				repaint();
				onUpdate(*this);
			};
		}

		const Patch& Patches::operator[](int i) const noexcept
		{
			return patches[i];
		}

		Patch& Patches::operator[](int i) noexcept
		{
			return patches[i];
		}

		const Patch* Patches::getSelected() const noexcept
		{
			return selection.selected;
		}

		Patch* Patches::getSelected() noexcept
		{
			return selection.selected;
		}

		void Patches::select(const String& author, const String& name) noexcept
		{
			for (auto& patch : patches)
				if (patch.author == author && patch.name == name)
				{
					selection.select(&patch);
					if(onUpdate)
						onUpdate(*this);
					return;
				}
			selection.select(nullptr);
		}

		const size_t Patches::size() const noexcept
		{
			return patches.size();
		}

		void Patches::mouseWheelMove(const Mouse& mouse, const MouseWheel& wheel)
		{
			scrollBar.mouseWheelMove(mouse, wheel);
		}

		void Patches::nextPatch(bool next)
		{
			auto idx = 0;
			if (selection.selected != nullptr)
			{
				const auto size = static_cast<int>(patches.size());
				idx = static_cast<int>(selection.selected - patches.data());
				idx += next ? 1 : -1;
				if (idx < 0)
					idx = size - 1;
				else if (idx >= size)
					idx = 0;
			}
			selection.select(&patches[idx]);
			const auto& file = patches[idx].file;
			const auto vt = ValueTree::fromXml(file.loadFileAsString());
			if (!vt.isValid())
				return;
			auto& state = utils.audioProcessor.state;
			state.state = vt;
			utils.audioProcessor.params.loadPatch(state);
			utils.audioProcessor.pluginProcessor.loadPatch(state);
		}

		// ButtonSavePatch

		void clearEditors(Patches& patches, TextEditor& eName, TextEditor& eAuthor)
		{
			eName.clear();
			eAuthor.clear();
			patches.updateFilter(eName.txt, eAuthor.txt);
		}

		void saveStuff(Patches& patches, TextEditor& eName, TextEditor& eAuthor)
		{
			auto name = eName.txt;
			auto author = eAuthor.txt;
			if (author.isEmpty() || author == "factory")
				author = "Audio Traveller";
			if (name.isEmpty())
			{
				if (const auto p = patches.getSelected())
				{
					if (p->author == "factory")
						return;
					if (name.isEmpty())
						name = p->name;
					if (author.isEmpty())
						author = p->author;
				}
			}
			const auto& utils = patches.utils;
			const auto patchesDirectory = getPatchesDirectory(utils);
			auto& state = patches.utils.audioProcessor.state;
			utils.audioProcessor.params.savePatch(state);
			utils.audioProcessor.pluginProcessor.savePatch(state);
			auto& vt = state.state;
			vt.setProperty("author", author, nullptr);
			const auto file = patchesDirectory.getChildFile(name + ".txt");
			if (file.existsAsFile())
				file.deleteFile();
			const auto result = file.create();
			if (result.failed())
				return;
			file.replaceWithText(vt.toXmlString());
			clearEditors(patches, eName, eAuthor);
			patches.select(author, name);
		}

		ButtonSavePatch::ButtonSavePatch(Patches& patches,
			TextEditor& editorName, TextEditor& editorAuthor) :
			Button(patches.utils)
		{
			onClick = [&](const Mouse&)
			{
				saveStuff(patches, editorName, editorAuthor);
			};

			makePaintButton(*this, makeButtonOnPaintSave(), "Click here to save this patch.");
		}

		// ButtonReveal

		ButtonReveal::ButtonReveal(Patches& patches) :
			Button(patches.utils)
		{
			onClick = [&, &p = patches](const Mouse&)
			{
				auto patchSelected = p.getSelected();
				if (patchSelected)
				{
					const auto& file = patchSelected->file;
					file.revealToUser();
					return;
				}
				const auto patchesDirectory = getPatchesDirectory(utils);
				patchesDirectory.revealToUser();
			};

			makePaintButton(*this, [](Graphics& g, const Button& b)
			{
				const auto hoverPhase = b.callbacks[Button::kHoverAniCB].phase;
				const auto clickPhase = b.callbacks[Button::kClickAniCB].phase;

				const auto thicc = b.utils.thicc;
				const auto lineThicc = thicc + hoverPhase + clickPhase * thicc;

				const auto bounds = maxQuadIn(b.getLocalBounds()).reduced(lineThicc);

				const auto x = bounds.getX();
				const auto y = bounds.getY();
				const auto w = bounds.getWidth();
				const auto h = bounds.getHeight();
				const auto r = x + w;
				const auto btm = y + h;

				Path path;
				const auto x1 = x + w * .1f;
				const auto x2 = x + w * .2f;
				const auto x4 = x + w * .4f;
				const auto x5 = x + w * .5f;
				const auto x8 = x + w * .8f;
				const auto x9 = x + w * .9f;
				const auto y1 = y + h * .1f;
				const auto y2 = y + h * .2f;
				const auto y3 = y + h * .3f;
				const auto y5 = y + h * .5f;
				const auto y3To5 = y3 + hoverPhase * (y5 - y3);
				path.startNewSubPath(x1, y);
				path.lineTo(x4, y);
				path.lineTo(x5, y1);
				path.lineTo(x9, y1);
				path.lineTo(x8, y3To5);
				path.lineTo(x2, y3To5);
				path.lineTo(x, btm);
				path.lineTo(x, y2);
				path.closeSubPath();

				path.startNewSubPath(x, btm);
				path.lineTo(x8, btm);
				path.lineTo(r, y3To5);
				path.lineTo(x8, y3To5);

				Stroke stroke(lineThicc, Stroke::JointStyle::beveled, Stroke::EndCapStyle::butt);
				setCol(g, CID::Interact);
				g.strokePath(path, stroke);
			}, "Click here to reveal the patches directory or the selected patch.");
		}

		// Browser

		Browser::Browser(Utils& u) :
			Comp(u),
			title(u),
			editorAuthor(u, "enter author"),
			editorName(u, "enter name"),
			patches(u),
			saveButton(patches, editorName, editorAuthor),
			revealButton(patches),
			nameText(""),
			authorText("")
		{
			layout.init
			(
				{ 2, 2, 1, 1 },
				{ 1, 1, 13 }
			);

			addAndMakeVisible(title);
			addAndMakeVisible(editorAuthor);
			addAndMakeVisible(editorName);
			addAndMakeVisible(saveButton);
			addAndMakeVisible(revealButton);
			addAndMakeVisible(patches);

			editorAuthor.tooltip = "Click here to enter the name of the author of the current patch.";
			editorName.tooltip = "Click here to enter the name of the current patch.";
			editorAuthor.labelEmpty.setText("Author");
			editorName.labelEmpty.setText("Name");

			makeTextLabel
			(
				title,
				"Patch Browser",
				font::text(),
				Just::centred, CID::Txt,
				"You have entered the patch browser. no shit."
			);
			title.autoMaxHeight = true;

			editorName.onKeyPress = [&](const KeyPress&)
			{
				const auto text = editorName.txt;
				if (nameText == text)
					return;
				nameText = text;
				patches.updateFilter(nameText, authorText);
			};

			editorName.onEnter = [&]()
			{
				saveStuff(patches, editorName, editorAuthor);
			};

			editorAuthor.onKeyPress = [&](const KeyPress&)
			{
				const auto text = editorAuthor.txt;
				if (authorText == text)
					return;
				authorText = text;
				patches.updateFilter(nameText, authorText);
			};

			addEvt([&](evt::Type t, const void*)
			{
				if (t == evt::Type::ClickedEmpty)
					setVisible(false);
			});

			const auto& user = u.getProps();
			const auto name = user.getValue("patchname", "");
			const auto author = user.getValue("patchauthor", "");
			if (name.isNotEmpty() && author.isNotEmpty())
			{
				patches.select(author, name);
			}

			setOpaque(true);
		}

		Browser::~Browser()
		{
			const auto selected = getSelectedPatch();
			const auto tweaked = utils.params.isTweaked();
			auto& user = utils.getProps();
			if (selected == nullptr || tweaked)
			{
				user.setValue("patchname", "");
				user.setValue("patchauthor", "");
				return;
			}
			user.setValue("patchname", selected->name);
			user.setValue("patchauthor", selected->author);
		}

		void Browser::setVisible(bool e)
		{
			Comp::setVisible(e);
			clearEditors(patches, editorName, editorAuthor);
		}

		void Browser::paint(Graphics& g)
		{
			g.fillAll(getColour(CID::Bg));
			const auto c0 = getColour(CID::Darken);
			const auto c1 = Colour(0x00000000);
			const auto p0 = PointF(0.f, 0.f);
			const auto p1 = Point(0, getHeight()).toFloat();
			Gradient gradient(c0, p0, c1, p1, false);
			g.setGradientFill(gradient);
			g.fillRect(getLocalBounds().toFloat());
		}

		void Browser::nextPatch(bool next)
		{
			patches.nextPatch(next);
		}

		const Patch* Browser::getSelectedPatch() const noexcept
		{
			return patches.getSelected();
		}

		Patch* Browser::getSelectedPatch() noexcept
		{
			return patches.getSelected();
		}

		void Browser::resized()
		{
			Comp::resized();
			layout.place(title, 0, 0, 4, 1);
			layout.place(editorName, 0, 1, 1, 1);
			layout.place(editorAuthor, 1, 1, 1, 1);
			layout.place(saveButton, 2, 1, 1, 1);
			layout.place(revealButton, 3, 1, 1, 1);
			layout.place(patches, 0, 2, 4, 1);
		}

		void Browser::overwriteSelectedPatch()
		{
			const auto selectedPtr = getSelectedPatch();
			if (!selectedPtr)
				return;
			editorName.txt = selectedPtr->name;
			editorAuthor.txt = selectedPtr->author;
			saveStuff(patches, editorName, editorAuthor);
		}

		PatchesUpdatedFunc& Browser::getOnUpdate() noexcept
		{
			return patches.onUpdate;
		}

		// BrowserButton

		BrowserButton::BrowserButton(Browser& browser) :
			Button(browser.utils),
			patchTweaked(false),
			reportUpdate(true)
		{
			makeTextButton
			(
				*this, "-",
				"Click here to save, browse or manage patches.",
				CID::Interact, Colour(0x000000)
			);
			label.autoMaxHeight = true;
			onClick = [&b = browser](const Mouse&)
			{
				const auto e = !b.isVisible();
				b.notify(evt::Type::ClickedEmpty);
				b.setVisible(e);
			};

			auto& onUpdate = browser.getOnUpdate();
			onUpdate = [&](const Patches&)
			{
				reportUpdate = true;
			};

			add(Callback([&]()
			{
				if (reportUpdate)
				{
					const auto selectedPtr = browser.getSelectedPatch();
					const auto& sName = selectedPtr ? selectedPtr->name : "-";
					label.setText(sName);
					patchTweaked = false;
					utils.params.setTweaked(false);
					reportUpdate = false;
					repaint();
					return;
				}
				if (!isShowing())
					return;
				const auto nT = utils.params.isTweaked();
				if (patchTweaked == nT || !nT)
					return;
				patchTweaked = true;
				label.setText(label.text + "*");
				repaint();
			}, Button::kNumCallbacks, cbFPS::k7_5, true));
		}

		// ButtonSaveQuick

		ButtonSaveQuick::ButtonSaveQuick(Browser& browser) :
			Button(browser.utils)
		{
			makePaintButton(*this, makeButtonOnPaintSave(), "Click here to overwrite the selected patch.");
			label.autoMaxHeight = true;
			onClick = [&](const Mouse&)
			{
				notify(evt::Type::ClickedEmpty);
				browser.overwriteSelectedPatch();
			};
		}

		// NextPatchButton

		NextPatchButton::NextPatchButton(Browser& browser, bool next) :
			Button(browser.utils)
		{
			const String text(next ? ">" : "<");
			makeTextButton(*this, text, "Click here to load the " + text + " patch.", CID::Interact, Colour(0x00000000));
			label.autoMaxHeight = true;
			onClick = [&b = browser, next](const Mouse&)
			{
				b.notify(evt::Type::ClickedEmpty);
				b.nextPatch(next);
			};
		}
	}
}