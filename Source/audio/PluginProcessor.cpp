#include "PluginProcessor.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params& params
#if PPDHasTuningEditor
		, XenManager& xen
#endif
	) :
		sampleRate(1.),
		pitchDetector(),
		phase(0.f),
		inc(0.f)
	{
		params(PID::FFTOrder).callback = [&](CB cb)
		{
			pitchDetector.setFFTOrder(cb.getInt());
		};

		params(PID::Smooth).callback = [&](CB cb)
		{
			pitchDetector.setLPDecayInMs(cb.denorm());
		};
	}

	void PluginProcessor::prepare(double _sampleRate)
	{
		sampleRate = _sampleRate;
		pitchDetector.prepare(sampleRate);
	}

	void PluginProcessor::operator()(ProcessorBufferView& view,
		MidiBuffer&, const Transport::Info& transport) noexcept
	{
		auto main = view.getSamplesMain(0);
		for (auto s = 0; s < view.numSamples; ++s)
		{
			const auto info = pitchDetector(main[s]);
			auto gain = 0.f;
			if (transport.playing && info.tonal)
			{
				gain = 1.f;
				inc = info.freqHz / static_cast<float>(sampleRate);
			}
			phase += inc;
			if (phase >= 1.f)
				--phase;
			main[s] = std::cos(phase * Tau) * gain;
		}
		if(view.getNumChannelsMain() == 2)
			SIMD::copy(view.getSamplesMain(1), main, view.numSamples);
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