#pragma once
#include "gui/Prompt.h"
#include "gui/CompTexture.h"
#include "gui/HeaderEditor.h"
#include "gui/Tooltip.h"
#include "gui/ParameterEditor.h"
#include "Editor2.h"

namespace gui
{
	using AudioProcessorEditor = juce::AudioProcessorEditor;

    struct Editor :
        public AudioProcessorEditor
    {
        Editor(Processor&);
		void paint(Graphics&) override;
        void paintOverChildren(Graphics&) override;
        void resized() override;
        void mouseEnter(const Mouse&) override;
        void mouseUp(const Mouse&) override;

        Processor& audioProcessor;
        Utils utils;
        Layout layout;
        evt::Member evtMember;
        CompTexture texture;

        patch::Browser patchBrowser;
        ColoursEditor coloursEditor;
        ManifestOfWisdom manifest;
		HeaderEditor header;
        Tooltip tooltip;
        Toast toast;
        ParameterEditor parameterEditor;
        Callback callback;
        CompPower powerComp;
		Editor2 editor2;
        Prompt prompt;

        //JUCE_HEAVYWEIGHT_LEAK_DETECTOR(Editor)
    };
}