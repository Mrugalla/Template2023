#include "Editor.h"

namespace gui
{
    evt::Evt makeEvt(Component& comp)
    {
        return [&c = comp](evt::Type type, const void*)
        {
            if (type == evt::Type::ColourSchemeChanged)
            {
                repaintWithChildren(&c);
            }
            else if (type == evt::Type::ClickedEmpty)
            {
                c.giveAwayKeyboardFocus();
            }
        };
    }

    Editor::Editor(Processor& p) :
        AudioProcessorEditor(&p),
        audioProcessor(p),
        utils(*this, p),
        layout(),
        evtMember(utils.eventSystem, makeEvt(*this)),
        bgImage(utils),
        labels
        {
            Label(utils, JucePlugin_Name, "This is the name of my plugin."),
            Label(utils, JucePlugin_Manufacturer, "Hi :) I'm the developer of this plugin.")
        }
    {
        layout.init
        (
            { 1, 3, 5, 8 },
            { 1, 3, 5, 8 }
        );

        addAndMakeVisible(bgImage);
        for (auto& label : labels)
            addAndMakeVisible(label);

        const auto& user = *audioProcessor.state.props.getUserSettings();
        const auto editorWidth = user.getDoubleValue("EditorWidth", EditorWidth);
        const auto editorHeight = user.getDoubleValue("EditorHeight", EditorHeight);
        setOpaque(true);
        setResizable(true, true);
        setSize
        (
            static_cast<int>(editorWidth),
            static_cast<int>(editorHeight)
        );
    }
    
    void Editor::paintOverChildren(Graphics& g)
    {
        layout.paint(g, juce::Colours::limegreen);
    }

    void Editor::resized()
    {
        if (getWidth() < EditorMinWidth)
            return setSize(EditorMinWidth, getHeight());
        if (getHeight() < EditorMinHeight)
            return setSize(getWidth(), EditorMinHeight);

        utils.resized();
        layout.resized(getLocalBounds());

        bgImage.setBounds(getLocalBounds());

        layout.place(labels[kTitle], 1, 1, 1, 1);
        layout.place(labels[kDev], 2, 1, 1, 1);
        setMaxCommonHeight(labels.data(), kNumLabels);

        auto& user = *audioProcessor.state.props.getUserSettings();
		const auto editorWidth = static_cast<double>(getWidth());
		const auto editorHeight = static_cast<double>(getHeight());
		user.setValue("EditorWidth", editorWidth);
		user.setValue("EditorHeight", editorHeight);
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