#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "audio/PluginProcessor.h"

#include "arch/XenManager.h"
#include "param/Param.h"

namespace audio
{
    using ChannelSet = juce::AudioChannelSet;
    using MidiBuffer = juce::MidiBuffer;
	using AudioBuffer = juce::AudioBuffer<float>;

	using XenManager = arch::XenManager;
    using State = arch::State;
    using Param = param::Param;
    using Params = param::Params;
    using PID = param::PID;
    
    struct Processor :
        public juce::AudioProcessor
    {
        Processor();
        ~Processor() override;
        void prepareToPlay(double, int) override;
        void releaseResources() override;
        bool isBusesLayoutSupported(const BusesLayout&) const override;
        void processBlock(AudioBuffer&, MidiBuffer&) override;
        void processBlockBypassed(AudioBuffer&, MidiBuffer&) override;
        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override;
        const juce::String getName() const override;
        bool acceptsMidi() const override;
        bool producesMidi() const override;
        bool isMidiEffect() const override;
        double getTailLengthSeconds() const override;
        int getNumPrograms() override;
        int getCurrentProgram() override;
        void setCurrentProgram(int) override;
        const juce::String getProgramName(int) override;
        void changeProgramName(int, const juce::String&) override;
        void getStateInformation(juce::MemoryBlock&) override;
        void setStateInformation(const void*, int) override;

#if PPDHasTuningEditor
        XenManager xenManager;
#endif
        Params params;
        State state;

        PluginProcessor pluginProcessor;
    };
}
