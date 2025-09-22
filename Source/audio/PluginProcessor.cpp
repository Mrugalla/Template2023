#include "PluginProcessor.h"
#if PPDHasOnsetDetector
#include "dsp/midi/Sysex.h"
#endif

namespace dsp
{
	PluginProcessor::PluginProcessor(Params&,
#if PPDHasTuningEditor
		XenManager& xen,
#endif
		Transport&
	) :
		sampleRate(1.)
	{
	}

	void PluginProcessor::prepare(double _sampleRate)
	{
		sampleRate = _sampleRate;
	}

	static bool onsetDetected = false;
	void PluginProcessor::operator()(ProcessorBufferView& view,
		const Transport::Info&) noexcept
	{
		if(onsetDetected && view.numSamples)
		{
			onsetDetected = false;
			for (auto ch = 0; ch < view.getNumChannelsMain(); ++ch)
			{
				auto smpls = view.getSamplesMain(ch);
				smpls[0] = 1.f;
			}
		}
		if (view.msg.isSysEx())
		{
			const auto size = view.msg.getSysExDataSize();
			if (size != 0)
			{
				const auto data = view.msg.getSysExData();
				Sysex sysexMessage(data, size);

				Sysex sysexOnset;
				sysexOnset.makeBytesOnset();
				if (sysexMessage == sysexOnset)
					onsetDetected = true;
			}
		}
	}

	void PluginProcessor::processBlockBypassed(float**, MidiBuffer&, int, int) noexcept
	{}

	void PluginProcessor::savePatch(State&)
	{
	}

	void PluginProcessor::loadPatch(const State&)
	{
	}
}