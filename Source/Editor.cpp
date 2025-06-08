#include "Editor.h"

namespace gui
{
    evt::Evt makeEvt(Editor& editor)
    {
        return [&e = editor](evt::Type type, const void*)
        {
            switch (type)
            {
            case evt::Type::ThemeUpdated:
                return e.repaint();
            case evt::Type::ClickedEmpty:
                e.parameterEditor.setActive(false);
                e.editor2.setVisible(true);
                //e.patchBrowser.setVisible(false);
                e.giveAwayKeyboardFocus();
                return;
            }
        };
    }

    void loadSize(Editor& editor)
    {
        const auto& user = *editor.audioProcessor.state.props.getUserSettings();
        const auto editorWidth = user.getDoubleValue("editorwidth", EditorWidth);
        const auto editorHeight = user.getDoubleValue("editorheight", EditorHeight);
        editor.setOpaque(true);
        editor.setResizable(true, true);
        editor.setSize
        (
            static_cast<int>(editorWidth),
            static_cast<int>(editorHeight)
        );
    }

	void saveSize(Editor& editor)
	{
        auto& user = *editor.audioProcessor.state.props.getUserSettings();
        const auto editorWidth = static_cast<double>(editor.getWidth());
        const auto editorHeight = static_cast<double>(editor.getHeight());
        user.setValue("editorwidth", editorWidth);
        user.setValue("editorheight", editorHeight);
	}

    bool canResize(Editor& editor)
    {
        if (editor.getWidth() < EditorMinWidth)
        {
            editor.setSize(EditorMinWidth, editor.getHeight());
            return false;
        }
        if (editor.getHeight() < EditorMinHeight)
        {
            editor.setSize(editor.getWidth(), EditorMinHeight);
            return false;
        }
        return true;
    }

    Editor::Editor(Processor& p) :
        AudioProcessorEditor(&p),
        audioProcessor(p),
        utils(*this, p),
        layout(),
        evtMember(utils.eventSystem, makeEvt(*this)),
        texture(utils, BinaryData::texture_png, BinaryData::texture_pngSize, .1f, 4),
        coloursEditor(utils),
        manifest(utils),
        header(coloursEditor, manifest),
        tooltip(utils),
        toast(utils),
        parameterEditor(utils),
        callback([&]()
        {
        }, 0, cbFPS::k_1_875, false),
        powerComp(utils),
        editor2(utils)
    {
        layout.init
        (
            { 1 },
            { 4, 32, 2 }
        );
        
        addAndMakeVisible(texture);
        addAndMakeVisible(header);
        addAndMakeVisible(tooltip);
		addAndMakeVisible(editor2);
		addChildComponent(coloursEditor);
		addChildComponent(manifest);
        addChildComponent(parameterEditor);
        addChildComponent(toast);
		addChildComponent(powerComp);

        utils.add(&callback);
        loadSize(*this);
    }

    void Editor::paint(Graphics& g)
    {
        g.fillAll(getColour(CID::Bg));
        const auto top = layout.top();
        setCol(g, CID::Darken);
        g.fillRect(top);
    }
    
    void Editor::paintOverChildren(Graphics&)
    {
        //layout.paint(g, getColour(CID::Hover));
    }

    void Editor::resized()
    {
		if (!canResize(*this))
			return;
        saveSize(*this);
        utils.resized();
        layout.resized(getLocalBounds());
        tooltip.setBounds(layout.bottom().toNearestInt());
		header.setBounds(layout.top().toNearestInt());
		layout.place(editor2, 0, 1, 1, 1);
        texture.setBounds(editor2.getBounds());
        powerComp.setBounds(editor2.getBounds());
		coloursEditor.setBounds(editor2.getBounds());
		manifest.setBounds(editor2.getBounds());

        const auto toastWidth = static_cast<int>(utils.thicc * 36.f);
        const auto toastHeight = toastWidth * 3 / 4;
        toast.setSize(toastWidth, toastHeight);
		parameterEditor.setSize(toastWidth * 3, toastHeight);
	}

    void Editor::mouseEnter(const Mouse&)
    {
        evtMember(evt::Type::TooltipUpdated);
    }

    void Editor::mouseUp(const Mouse&)
    {
        evtMember(evt::Type::ClickedEmpty);
    }
}