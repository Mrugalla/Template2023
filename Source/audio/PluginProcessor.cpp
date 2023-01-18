#include "PluginProcessor.h"

namespace audio
{
	PluginProcessor::PluginProcessor()
	{}

	void PluginProcessor::prepare(double, int)
	{}

	void PluginProcessor::operator()(float* const*, int, int, MidiBuffer&) noexcept
	{}

	void PluginProcessor::processBlockBypassed(float* const*, int, int, MidiBuffer&) noexcept
	{}

	void PluginProcessor::savePatch()
	{}

	void PluginProcessor::loadPatch()
	{}
}