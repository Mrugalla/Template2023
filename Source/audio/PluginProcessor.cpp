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

	void PluginProcessor::operator()(ProcessorBufferView& view,
		const Transport::Info&) noexcept
	{
		if (view.msg.isSysEx())
		{
			const auto data = view.msg.getSysExData();
			const auto size = view.msg.getSysExDataSize();
			const auto identifier = Sysex::decode(data, size);
			const auto bytes = Sysex::makeBytesOnset();
			const auto decoded = Sysex::decode(bytes);
			if (identifier == decoded)
			{
				const auto onsetIdx = static_cast<int>(view.msg.getTimeStamp());
				for (auto ch = 0; ch < view.getNumChannelsMain(); ++ch)
					view.getSamplesMain(ch)[onsetIdx] = 1.f;
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