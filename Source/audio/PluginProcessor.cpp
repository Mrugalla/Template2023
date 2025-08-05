#include "PluginProcessor.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params& params
#if PPDHasTuningEditor
		, XenManager& xen
#endif
	) :
		sampleRate(1.),
		freqShifter()
	{
		params(PID::Reflect).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setReflect(cb.getInt());
		};

		params(PID::Shift).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setShift(cb.denorm());
		};
	}

	void PluginProcessor::prepare(double _sampleRate)
	{
		sampleRate = _sampleRate;
		freqShifter.prepare(sampleRate);
	}

	static float phasor = 0.f;
	static float incFast = 420.f / 44100.f;
	static float incSlow = 50.f / 44100.f;
	static float env = 0.f;
	static float decay = 24.f / 44100.f;

	void PluginProcessor::operator()(ProcessorBufferView& view,
		const Transport::Info&) noexcept
	{
		auto smpls = view.getSamplesMain(0);
		for (auto s = 0; s < view.numSamples; ++s)
		{
			smpls[s] = std::cos(phasor * Tau) * env;
			const auto inc = incSlow + env * env * (incFast - incSlow);
			phasor += inc;
			if (phasor >= 1.f)
				--phasor;
			env += decay * -env;
		}
		if (view.msg.isNoteOn())
		{
			env = 1.f;
			phasor = 0.f;
		}
		if(view.getNumChannelsMain() == 2)
			SIMD::copy(view.getSamplesMain(1), smpls, view.numSamples);
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