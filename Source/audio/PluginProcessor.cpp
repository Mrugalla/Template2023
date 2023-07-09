#include "PluginProcessor.h"

namespace audio
{
	PluginProcessor::PluginProcessor(Params& _params) :
		params(_params)
	{}

	void PluginProcessor::prepare(double)
	{}

	void PluginProcessor::operator()(double* const*, int, int, dsp::MidiBuffer&) noexcept
	{}

	void PluginProcessor::processBlockBypassed(double* const*, int, int, dsp::MidiBuffer&) noexcept
	{}

	void PluginProcessor::savePatch()
	{}

	void PluginProcessor::loadPatch()
	{}
}