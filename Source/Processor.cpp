#include "Processor.h"
#include "Editor.h"
#include "audio/dsp/Distortion.h"
#include "arch/Math.h"

namespace audio
{
    Processor::BusesProps Processor::makeBusesProps()
    {
        BusesProps bp;
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        bp.addBus(true, "Input", ChannelSet::stereo(), true);
#endif
        bp.addBus(false, "Output", ChannelSet::stereo(), true);
#if PPDHasSidechain
        if (!juce::JUCEApplicationBase::isStandaloneApp())
            bp.addBus(true, "Sidechain", ChannelSet::stereo(), true);
#endif
#endif
        return bp;
    }
	
    Processor::Processor() :
        juce::AudioProcessor(makeBusesProps()),
		Timer(),
#if PPDHasTuningEditor
		xenManager(),
        params(*this, xenManager),
#else
		params(*this),
#endif
        state(),
        
        pluginProcessor(params),
        audioBufferD()
#if PPDHasGainIn
        ,gainInParam()
#endif
#if PPDHasGainWet
        ,gainWetParam()
#endif
    {
        startTimerHz(4);
    }

    Processor::~Processor()
    {
        auto& user = *state.props.getUserSettings();
        user.setValue("firstTimeUwU", false);
        user.save();
    }

    bool Processor::supportsDoublePrecisionProcessing() const
    {
        return true;
    }
	
    bool Processor::canAddBus(bool isInput) const
    {
        if (wrapperType == wrapperType_Standalone)
            return false;

        return PPDHasSidechain ? isInput : false;
    }
	
    const juce::String Processor::getName() const
    {
        return JucePlugin_Name;
    }

    bool Processor::acceptsMidi() const
    {
#if JucePlugin_WantsMidiInput
        return true;
#else
        return false;
#endif
    }

    bool Processor::producesMidi() const
    {
#if JucePlugin_ProducesMidiOutput
        return true;
#else
        return false;
#endif
    }

    bool Processor::isMidiEffect() const
    {
#if JucePlugin_IsMidiEffect
        return true;
#else
        return false;
#endif
    }

    double Processor::getTailLengthSeconds() const
    {
        return 0.;
    }

    int Processor::getNumPrograms()
    {
        return 1;
    }

    int Processor::getCurrentProgram()
    {
        return 0;
    }

    void Processor::setCurrentProgram(int)
    {
    }

    const juce::String Processor::getProgramName(int)
    {
        return {};
    }

    void Processor::changeProgramName(int, const juce::String&)
    {
    }

    void Processor::prepareToPlay(double sampleRate, int maxBlockSize)
    {
        audioBufferD.setSize(2, maxBlockSize, false, true, false);
        gainInParam.prepare(sampleRate, 10.);
        gainWetParam.prepare(sampleRate, 10.);
		pluginProcessor.prepare(sampleRate);
    }

    void Processor::releaseResources()
    {
    }

    bool Processor::isBusesLayoutSupported(const BusesLayout& layouts) const
    {
#if JucePlugin_IsMidiEffect
        juce::ignoreUnused(layouts);
        return true;
#endif
        const auto mono = ChannelSet::mono();
        const auto stereo = ChannelSet::stereo();

        const auto mainIn = layouts.getMainInputChannelSet();
        const auto mainOut = layouts.getMainOutputChannelSet();

        if (mainIn != mainOut)
            return false;

        if (mainOut != stereo && mainOut != mono)
            return false;

#if PPDHasSidechain
        if (wrapperType != wrapperType_Standalone)
        {
            const auto scIn = layouts.getChannelSet(true, 1);
            if (!scIn.isDisabled())
                if (scIn != mono && scIn != stereo)
                    return false;
        }
#endif

        return true;
    }

    bool Processor::hasEditor() const
    {
        return false;
    }

    juce::AudioProcessorEditor* Processor::createEditor()
    {
        return new gui::Editor(*this);
    }

    void Processor::getStateInformation(juce::MemoryBlock& destData)
    {
        pluginProcessor.savePatch();
        params.savePatch(state);
        state.savePatch(*this, destData);
    }

    void Processor::setStateInformation(const void* data, int sizeInBytes)
    {
        state.loadPatch(*this, data, sizeInBytes);
        params.loadPatch(state);
        pluginProcessor.loadPatch();
    }

    void Processor::processBlockBypassed(AudioBufferD& buffer, MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;
		
        param::processMacroMod(params);
		
        const auto numSamples = buffer.getNumSamples();
        if (numSamples == 0)
            return;
		
        const auto numChannels = buffer.getNumChannels() == 2 ? 2 : 1;
        auto samples = buffer.getArrayOfWritePointers();

        for (auto s = 0; s < numSamples; s += dsp::BlockSize)
        {
            double* block[] = { &samples[0][s], &samples[1][s] };
            const auto dif = numSamples - s;
            const auto bNumSamples = dif < dsp::BlockSize ? dif : dsp::BlockSize;

            pluginProcessor.processBlockBypassed(block, numChannels, bNumSamples, midiMessages);
        }
    }

    void Processor::processBlockBypassed(AudioBufferF& buffer, MidiBuffer& midiMessages)
    {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        audioBufferD.setSize(numChannels, numSamples, true, false, true);

        auto samplesF = buffer.getArrayOfWritePointers();
        auto samplesD = audioBufferD.getArrayOfWritePointers();

        for (auto ch = 0; ch < numChannels; ++ch)
        {
            const auto smplsF = samplesF[ch];
            auto smplsD = samplesD[ch];

            for (auto s = 0; s < numSamples; ++s)
                smplsD[s] = static_cast<double>(smplsF[s]);
        }

        processBlockBypassed(audioBufferD, midiMessages);

        for (auto ch = 0; ch < numChannels; ++ch)
        {
            auto smplsF = samplesF[ch];
            const auto smplsD = samplesD[ch];

            for (auto s = 0; s < numSamples; ++s)
                smplsF[s] = static_cast<float>(smplsD[s]);
        }
    }

    void Processor::processBlock(AudioBufferD& buffer, MidiBuffer& midiMessages)
    {
        const bool pluginDisabled = params(PID::Power).getValue() < .5f;
        if (pluginDisabled)
            return processBlockBypassed(buffer, midiMessages);
		
        juce::ScopedNoDenormals noDenormals;
		
        param::processMacroMod(params);

        const auto numSamples = buffer.getNumSamples();
        {
            const auto totalNumInputChannels = getTotalNumInputChannels();
            const auto totalNumOutputChannels = getTotalNumOutputChannels();

            for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
                buffer.clear(i, 0, numSamples);
        }
        if (numSamples == 0)
            return;
		
		const auto numChannels = buffer.getNumChannels();
		auto samples = buffer.getArrayOfWritePointers();
        
#if PPDHasGainIn
        const auto gainInDb = params(PID::GainIn).getValModDenorm();
        const auto gainInAmp = math::decibelToAmp(gainInDb);
#if PPDHasUnityGain
        const auto unityGainEnabled = params(PID::UnityGain).getValMod() > .5;
#endif
#endif
#if PPDHasClipper
        const bool shallClip = params(PID::Clipper).getValMod() > .5f;
#endif
#if PPDHasGainWet
        const auto gainWetDb = params(PID::GainWet).getValModDenorm();
        const auto gainWetAmp = math::decibelToAmp(gainWetDb);
#endif

        for (auto s = 0; s < numSamples; s += dsp::BlockSize)
        {
            double* block[] = { &samples[0][s], &samples[1][s] };
            const auto dif = numSamples - s;
            const auto bNumSamples = dif < dsp::BlockSize ? dif : dsp::BlockSize;

			// process mix input
			
            pluginProcessor(block, numChannels, bNumSamples, midiMessages);
#if PPDHasClipper
            if (shallClip)
                for (auto ch = 0; ch < numChannels; ++ch)
                {
                    auto smpls = block[ch];
                    for (auto i = 0; i < bNumSamples; ++i)
                        smpls[i] = dsp::softclipSigmoid(smpls[i], 1., dsp::Pi);
                }
#endif
            // process mix output
#if PPDHasGainWet
            const auto gainWetInfo = gainWetParam(gainWetAmp, bNumSamples);
            if (!gainWetInfo.smoothing)
            {
                if (gainWetInfo.val != 1.)
                    for (auto ch = 0; ch < numChannels; ++ch)
                        SIMD::multiply(block[ch], gainWetInfo.val, bNumSamples);
            }
            else
                for (auto ch = 0; ch < numChannels; ++ch)
                    SIMD::multiply(block[ch], gainWetInfo.buf, bNumSamples);
#endif
        }
#if JUCE_DEBUG
        for (auto ch = 0; ch < numChannels; ++ch)
        {
            auto smpls = samples[ch];
            for (auto s = 0; s < numSamples; ++s)
                smpls[s] = dsp::hardclip(smpls[s], 1.);
        }
#endif
    }

    void Processor::processBlock(AudioBufferF& buffer, MidiBuffer& midiMessages)
    {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        audioBufferD.setSize(numChannels, numSamples, true, false, true);

        auto samplesF = buffer.getArrayOfWritePointers();
        auto samplesD = audioBufferD.getArrayOfWritePointers();

        for (auto ch = 0; ch < numChannels; ++ch)
        {
            const auto smplsF = samplesF[ch];
            auto smplsD = samplesD[ch];

            for (auto s = 0; s < numSamples; ++s)
                smplsD[s] = static_cast<double>(smplsF[s]);
        }

        processBlock(audioBufferD, midiMessages);

        for (auto ch = 0; ch < numChannels; ++ch)
        {
            auto smplsF = samplesF[ch];
            const auto smplsD = samplesD[ch];

            for (auto s = 0; s < numSamples; ++s)
                smplsF[s] = static_cast<float>(smplsD[s]);
        }
    }

    void Processor::timerCallback()
    {
		
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new audio::Processor();
}