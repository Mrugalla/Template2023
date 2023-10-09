#include "Editor.h"

namespace gui
{
    evt::Evt makeEvt(Component& comp)
    {
        return [&c = comp](evt::Type type, const void* stuff)
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
        evtMember(utils.eventSystem, makeEvt(*this))
    {
        layout.init
        (
            { 1, 3, 5, 8 },
            { 8, 5, 3, 1 }
        );

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
    
    void Editor::paint(Graphics& g)
    {
        g.fillAll(juce::Colours::black);
        layout.paint(g, juce::Colours::limegreen);
    }

    void Editor::resized()
    {
        layout.resized(getLocalBounds());

        auto& user = *audioProcessor.state.props.getUserSettings();

		const auto editorWidth = static_cast<double>(getWidth());
		const auto editorHeight = static_cast<double>(getHeight());
		user.setValue("EditorWidth", editorWidth);
		user.setValue("EditorHeight", editorHeight);
	}

    void Editor::mouseUp(const Mouse&)
    {
        evtMember(evt::Type::ClickedEmpty);
    }
}