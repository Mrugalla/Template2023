#include "PluginProcessor.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params& params,
#if PPDHasTuningEditor
		XenManager& xen,
#endif
		Transport&
	) :
		sampleRate(1.),
		onsetDetector()
	{
		params(PID::Smooth).callback = [&](CB cb)
		{
			onsetDetector.setSmooth(cb.denormD());
		};

		params(PID::Atk0).callback = [&](CB cb)
		{
			onsetDetector.setAttack(cb.denormD(), 0);
		};

		params(PID::Dcy0).callback = [&](CB cb)
		{
			onsetDetector.setDecay(cb.denormD(), 0);
		};

		params(PID::Atk1).callback = [&](CB cb)
		{
			onsetDetector.setAttack(cb.denormD(), 1);
		};

		params(PID::Dcy1).callback = [&](CB cb)
		{
			onsetDetector.setDecay(cb.denormD(), 1);
		};
	}

	void PluginProcessor::prepare(double _sampleRate)
	{
		sampleRate = _sampleRate;
		onsetDetector.prepare(sampleRate);
	}

	void PluginProcessor::operator()(ProcessorBufferView& view,
		const Transport::Info&) noexcept
	{
		onsetDetector(view);
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