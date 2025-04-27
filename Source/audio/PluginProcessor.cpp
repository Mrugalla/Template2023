#include "PluginProcessor.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params& params
#if PPDHasTuningEditor
		, XenManager& xen
#endif
	) :
		sampleRate(1.)
	{
	}

	void PluginProcessor::prepare(float _sampleRate)
	{
		sampleRate = _sampleRate;
	}

	void PluginProcessor::operator()(float** samples,
		MidiBuffer&, const Transport::Info&,
		int numChannels, int numSamples) noexcept
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