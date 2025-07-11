#include "PluginProcessor.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params&
#if PPDHasTuningEditor
		, XenManager& xen
#endif
	) :
		sampleRate(1.)
	{
	}

	void PluginProcessor::prepare(double _sampleRate)
	{
		sampleRate = _sampleRate;
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