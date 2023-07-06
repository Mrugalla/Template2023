#include "Editor.h"

namespace gui
{
    Editor::Editor(AudioProcessor& p) :
        AudioProcessorEditor(&p),
        Timer(),
        audioProcessor(p)
    {
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
        startTimerHz(FPS);
    }

    void Editor::timerCallback()
    {
    }
    
    void Editor::paint(Graphics& g)
    {
        g.fillAll(juce::Colours::black);
    }

    void Editor::resized()
    {
        auto& user = *audioProcessor.state.props.getUserSettings();

		const auto editorWidth = static_cast<double>(getWidth());
		const auto editorHeight = static_cast<double>(getHeight());
		user.setValue("EditorWidth", editorWidth);
		user.setValue("EditorHeight", editorHeight);
	}
}