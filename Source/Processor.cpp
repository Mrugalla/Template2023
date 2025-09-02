#include "Processor.h"
#include "Editor.h"

#if PPDHasStereoConfig
#include "audio/dsp/MidSide.h"
#endif

#include "audio/dsp/Distortion.h"

#define KeepState true

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
        pluginRecorder(),
        transport(),
        pluginProcessor
        (
            params,
#if PPDHasTuningEditor
            xenManager,
#endif
			transport
        ),
        mixProcessor()
    {
        const auto& user = *state.props.getUserSettings();
        const auto& settingsFile = user.getFile();
        const auto settingsDirectory = settingsFile.getParentDirectory();
		const auto patchesDirectory = settingsDirectory.getChildFile("Patches");
        if (!patchesDirectory.exists())
        {
            patchesDirectory.createDirectory();
            /*
            using String = juce::String;
            const auto makePatch = [&p = patchesDirectory]
                (const String& name, const void* data, int size)
            {
                const auto initFile = p.getChildFile(name + ".txt");
                if (initFile.existsAsFile())
                    initFile.deleteFile();
                initFile.create();
                initFile.replaceWithData(data, size);
            };
            */
		}

        state.set("author", "factowy");
        params.savePatch(state);
        pluginProcessor.savePatch(state);
        const auto init = state.state.createCopy();
        const auto initFile = patchesDirectory.getChildFile(" init.txt");
        if (initFile.existsAsFile())
            initFile.deleteFile();
        initFile.create();
        const auto xmlString = init.toXmlString();
		initFile.replaceWithText(xmlString);
    }

    Processor::~Processor()
    {
        auto& user = *state.props.getUserSettings();
        user.setValue("firstTimeUwU", false);
        user.save();
    }

    bool Processor::supportsDoublePrecisionProcessing() const
    {
        return false;
    }
	
    bool Processor::canAddBus(bool isInput) const
    {
        if (wrapperType == wrapperType_Standalone)
            return false;

        return PPDHasSidechain ? isInput : false;
    }
	
    const String Processor::getName() const
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
        return false;
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

    void Processor::prepareToPlay(double sampleRate, int)
    {
        auto latency = 0;
        const auto sampleRateF = static_cast<float>(sampleRate);
        params.prepare();
        pluginRecorder.prepare(sampleRateF);
        mixProcessor.prepare(sampleRateF);
        transport.prepare(1. / sampleRate);
        pluginProcessor.prepare(sampleRate);
        setLatencySamples(latency);
        startTimerHz(4);
    }

    void Processor::releaseResources()
    {
    }

    bool Processor::isBusesLayoutSupported(const BusesLayout& layouts) const
    {
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
#if KeepState
        pluginProcessor.savePatch(state);
        params.savePatch(state);
        state.savePatch(*this, destData);
#endif
    }

    void Processor::setStateInformation(const void* data, int sizeInBytes)
    {
#if KeepState
        state.loadPatch(*this, data, sizeInBytes);
        params.loadPatch(state);
        pluginProcessor.loadPatch(state);
#endif
    }

    void Processor::processBlock(AudioBufferF& buffer, MidiBuffer& midiMessages)
    {
        const bool pluginDisabled = params(PID::Power).getValue() < .5f;
        if (pluginDisabled)
            return processBlockBypassed(buffer, midiMessages);

        juce::ScopedNoDenormals noDenormals;

        auto mainBus = getBus(true, 0);
        auto mainBuffer = mainBus->getBusBuffer(buffer);
        dsp::ProcessorBufferView bufferView;
		bufferView.assignMain
        (
            mainBuffer.getArrayOfWritePointers(),
            mainBuffer.getNumChannels(),
            mainBuffer.getNumSamples()
        );
#if PPDHasSidechain
        if (wrapperType != wrapperType_Standalone)
        {
            auto scBus = getBus(true, 1);
            if (scBus)
                if (scBus->isEnabled())
                {
                    const auto scEnabled = params(PID::Sidechain).getValue() > .5f;
                    const auto& scGainParam = params(PID::SCGain);
                    const auto scGainDb = scGainParam.getValModDenorm();
                    const auto scGain = math::dbToAmp(scGainDb);

                    auto scBuffer = scBus->getBusBuffer(buffer);
                    auto scSamples = scBuffer.getArrayOfWritePointers();
                    const auto numChannelsSC = scBuffer.getNumChannels();
                    bufferView.assignSC(scSamples, scGain, numChannelsSC, scEnabled);
                }
        }
        bufferView.useMainForSCIfRequired();
#endif

        const auto macroVal = params(PID::Macro).getValue();
        params.modulate(macroVal, bufferView.getNumChannelsMain());

        {
            const auto totalNumInputChannels = getTotalNumInputChannels();
            const auto totalNumOutputChannels = getTotalNumOutputChannels();
            for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
                buffer.clear(i, 0, bufferView.getNumSamples());
        }
        if (bufferView.getNumSamples() == 0)
            return;
        transport(playHead, bufferView.getNumChannelsMain());
#if PPDHasStereoConfig
        bool midSide = false;
        if (bufferView.getNumChannelsMain() == 2)
        {
            midSide = params(PID::StereoConfig).getValue() > .5f;
            if (midSide)
                dsp::midSideEncode(bufferView);
        }
#endif
#if PPDIsNonlinear
        const auto gainInDb = params(PID::GainIn).getValModDenorm();
        const auto unityGain = params(PID::UnityGain).getValMod() > .5f;
#endif
#if PPDIO == PPDIODryWet
        const auto gainDryDb = params(PID::GainDry).getValModDenorm();
        const auto gainWetDb = params(PID::GainWet).getValModDenorm();
#elif PPDIO == PPDIOWetMix
        const auto gainWetDb = params(PID::GainWet).getValModDenorm();
        const auto mix = params(PID::Mix).getValMod();
#if PPDHasDelta
        const auto delta = params(PID::Delta).getValMod() > .5f;
#else
        static constexpr bool delta = false;
#endif
#endif
        const auto gainOutDb = params(PID::GainOut).getValModDenorm();

#if PPDHasTuningEditor
        auto xen = params(PID::Xen).getValModDenorm();
        const auto xenSnap = params(PID::XenSnap).getValMod() > .5f;
        if (xenSnap)
            xen = std::round(xen);
        const auto masterTune = std::round(params(PID::MasterTune).getValModDenorm());
        const auto anchor = std::round(params(PID::AnchorPitch).getValModDenorm());
        const auto pitchbendRange = std::round(params(PID::PitchbendRange).getValModDenorm());
        xenManager({ xen, masterTune, anchor, pitchbendRange }, numChannels);
#endif
        {
            const auto dummy = juce::MidiMessage::createSysExMessage(nullptr, 0);
            auto s = 0;
            dsp::ProcessorBufferView bufferViewBlock;
            for (const auto it : midiMessages)
            {
                const auto msg = it.getMessage();
                const auto ts = it.samplePosition;
                do
                {
                    const auto numSamplesToEvt = ts - s;
                    if (numSamplesToEvt < dsp::BlockSize)
                    {
                        bufferViewBlock.fillBlock(bufferView, msg, s, numSamplesToEvt);
						s += numSamplesToEvt;
                    }
                    else
                    {
                        bufferViewBlock.fillBlock(bufferView, dummy, s, dsp::BlockSize);
						s += dsp::BlockSize;
                    }
                    processIGuess(bufferViewBlock, mix, gainWetDb, gainOutDb, delta);
                } while (s < ts);
            }
            while (s < bufferView.getNumSamples())
            {
                const auto numSamplesToEnd = bufferView.getNumSamples() - s;
                const auto numSamples = std::min(dsp::BlockSize, numSamplesToEnd);
                bufferViewBlock.fillBlock(bufferView, dummy, s, numSamples);
                processIGuess(bufferViewBlock, mix, gainWetDb, gainOutDb, delta);
                s += numSamples;
            }
        }
#if PPDHasStereoConfig
        if (midSide)
            dsp::midSideDecode(bufferView);
#endif
        pluginRecorder(bufferView);
#if JUCE_DEBUG && false
        for (auto ch = 0; ch < numChannels; ++ch)
        {
            auto smpls = samplesMain[ch];
            for (auto s = 0; s < numSamplesMain; ++s)
                smpls[s] = dsp::hardclip(smpls[s], 2.f);
        }
#endif
    }

    void Processor::processIGuess(dsp::ProcessorBufferView& bufferViewBlock,
        float mix, float gainWetDb, float gainOutDb, bool delta) noexcept
    {
#if PPDIO == PPDIOOut
#if PPDIsNonlinear
        mixProcessor.split
        (
            samples, gainInDb, numChannels, numSamples
        );
#endif
#elif PPDIO == PPDIODryWet
#if PPDIsNonlinear
        mixProcessor.split
        (
            samples, gainDryDb, gainInDb, numChannels, numSamples
        );
#else
        mixProcessor.split
        (
            samples, gainDryDb, numChannels, numSamples
        );
#endif
#else
#if PPDIsNonlinear
        mixProcessor.split(bufferViewBlock, gainInDb);
#else
        mixProcessor.split
        (
            bufferViewBlock
        );
#endif
#endif
        pluginProcessor(bufferViewBlock, transport.info);
        transport(bufferViewBlock.numSamples);
#if JucePlugin_ProducesMidiOutput
        midiMessages.clear();
        for (auto it : midiSubBuffer)
            midiMessages.addEvent(it.getMessage(), it.samplePosition + s);
#endif
#if PPDIO == PPDIOOut
#if PPDIsNonlinear
        mixProcessor.join
        (
            bufferViewBlock, gainOutDb, unityGain
        );
#else
        mixProcessor.join
        (
            bufferViewBlock, gainOutDb
        );
#endif
#elif PPDIO == PPDIODryWet
#if PPDIsNonlinear
        mixProcessor.join
        (
            samples, gainWetDb, gainOutDb, numChannels, numSamples, unityGain
        );
#else
        mixProcessor.join
        (
            samples, gainWetDb, gainOutDb, numChannels, numSamples
        );
#endif
#else
#if PPDIsNonlinear
        mixProcessor.join
        (
            bufferViewBlock, mix, gainWetDb, gainOutDb, unityGain, delta
        );
#else
        mixProcessor.join
        (
            bufferViewBlock, mix, gainWetDb, gainOutDb, delta
        );
#endif
#endif
    }

    void Processor::processBlockBypassed(AudioBufferF& buffer, MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;

        const auto macroVal = params(PID::Macro).getValue();
        const auto numChannels = std::min(2, buffer.getNumChannels());
        params.modulate(macroVal, numChannels);

        const auto numSamplesMain = buffer.getNumSamples();
        if (numSamplesMain == 0)
            return;
        auto samplesMain = buffer.getArrayOfWritePointers();

        for (auto s = 0; s < numSamplesMain; s += dsp::BlockSize)
        {
            float* samples[] = { &samplesMain[0][s], &samplesMain[1][s] };
            const auto dif = numSamplesMain - s;
            const auto numSamples = dif < dsp::BlockSize ? dif : dsp::BlockSize;

            pluginProcessor.processBlockBypassed(samples, midiMessages, numChannels, numSamples);
        }
    }

    void Processor::timerCallback()
    {
        static constexpr bool needForcePrepare = false;
        if(needForcePrepare)
            forcePrepare();
    }

    void Processor::forcePrepare()
    {
        suspendProcessing(true);
        prepareToPlay(getSampleRate(), getBlockSize());
        suspendProcessing(false);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new audio::Processor();
}

#undef KeepState