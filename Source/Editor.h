#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "gui/Utils.h"
#include "Processor.h"

namespace gui
{
	static constexpr double EditorWidth = 690.;
    static constexpr double EditorHeight = 420.;
    static constexpr int FPS = 60;
    
    static constexpr double FPSD = static_cast<double>(FPS);
    static constexpr double FPSInv = 1. / FPSD;

    // JUCE GRAPHICS
	using AudioProcessorEditor = juce::AudioProcessorEditor;
    using Timer = juce::Timer;
    using Graphics = juce::Graphics;
    using String = juce::String;
    using Just = juce::Justification;

    // CUSTOM
    using AudioProcessor = audio::Processor;

    struct Editor :
        public AudioProcessorEditor,
        public Timer
    {
        Editor(AudioProcessor&);

        void paint(Graphics&) override;
        void resized() override;
        
        void timerCallback() override;

        AudioProcessor& audioProcessor;
    };
}