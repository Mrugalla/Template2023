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
		freqShifter.configure(sampleRate, BlockSize, 2);
	}

	void PluginProcessor::operator()(ProcessorBufferView&,
		MidiBuffer&, const Transport::Info&) noexcept
	{
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