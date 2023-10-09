#include "PluginProcessor.h"
#include "dsp/Distortion.h"

namespace audio
{
	PluginProcessor::PluginProcessor(Params& _params) :
		params(_params)
	{
	}

	void PluginProcessor::prepare(double)
	{}

	void PluginProcessor::operator()(double** samples, dsp::MidiBuffer&, int numChannels, int numSamples) noexcept
	{
	}

	void PluginProcessor::processBlockBypassed(double**, dsp::MidiBuffer&, int, int) noexcept
	{}

	void PluginProcessor::savePatch()
	{}

	void PluginProcessor::loadPatch()
	{}
}