#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

/*
This is where dsp custom to each individual plugin will be written
*/

namespace audio
{
	using MidiBuffer = juce::MidiBuffer;

	struct PluginProcessor
	{
		PluginProcessor();

		void prepare(double, int);

		void operator()(float* const*, int, int, MidiBuffer&) noexcept;
		
		void processBlockBypassed(float* const*, int, int, MidiBuffer&) noexcept;

		void savePatch();
		
		void loadPatch();
	};
}