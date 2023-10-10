#pragma once
#include "gui/Label.h"
#include "gui/BgImage.h"

namespace gui
{
	using AudioProcessorEditor = juce::AudioProcessorEditor;

    struct Editor :
        public AudioProcessorEditor
    {
        Editor(Processor&);

        void paintOverChildren(Graphics&) override;
        void resized() override;
        void mouseEnter(const Mouse&) override;
        void mouseUp(const Mouse&) override;

        Processor& audioProcessor;
        Utils utils;
        Layout layout;
        evt::Member evtMember;
        BgImage bgImage;

        enum { kTitle, kDev, kNumLabels };
        std::array<Label, kNumLabels> labels;

        //JUCE_HEAVYWEIGHT_LEAK_DETECTOR(Editor)
    };
}