#include "PluginProcessor.h"
#include "dsp/midi/Sysex.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params&, TuneSys&, Transport&) :
		sampleRate(1.)
	{
	}

	void PluginProcessor::prepare(double _sampleRate)
	{
		sampleRate = _sampleRate;
	}

	void PluginProcessor::operator()(ProcessorBufferView&,
		const Transport::Info&) noexcept
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