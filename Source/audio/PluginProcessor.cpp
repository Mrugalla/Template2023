#include "PluginProcessor.h"

namespace dsp
{
	PluginProcessor::PluginProcessor() :
		sampleRate(1.)
	{
	}

	void PluginProcessor::prepare(double _sampleRate)
	{
		sampleRate = _sampleRate;
	}

	void PluginProcessor::operator()(double**,
		MidiBuffer&, const Transport::Info&,
		int, int) noexcept
	{
	}

	void PluginProcessor::processBlockBypassed(double**, MidiBuffer&, int, int) noexcept
	{}

	void PluginProcessor::savePatch(State&)
	{
	}

	void PluginProcessor::loadPatch(const State&)
	{
	}
}