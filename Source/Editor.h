#pragma once
#include "gui/Tooltip.h"
#include "gui/BgImage.h"
#include "gui/Knob.h"

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
        Tooltip tooltip;

        enum { kTitle, kDev, kNumLabels };
        std::array<Label, kNumLabels> labels;

        Button filterTypeButton;

        KnobPainterBasic painterMacro, painterGainOut, painterPB;
        KnobPainterSpirograph painterSlew;

        KnobParam macroKnob, slewKnob, gainOutKnob, pbKnob;

        //JUCE_HEAVYWEIGHT_LEAK_DETECTOR(Editor)
    };
}