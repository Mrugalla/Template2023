#include "PluginProcessor.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params&
#if PPDHasTuningEditor
		, XenManager& xen
#endif
	) :
		sampleRate(1.),
		freqShifter()
	{
	}

	void PluginProcessor::prepare(double _sampleRate)
	{
		sampleRate = _sampleRate;
		freqShifter.prepare(sampleRate);
	}

	void PluginProcessor::operator()(ProcessorBufferView& view,
		MidiBuffer&, const Transport::Info&) noexcept
	{
		freqShifter(view);
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