#pragma once
#include "../param/Param.h"
#include "Using.h"

/*
This is where dsp custom to each individual plugin will be written
*/

namespace audio
{
	struct PluginProcessor
	{
		using Params = param::Params;
		using PID = param::PID;
		
		PluginProcessor(Params&);

		/* sampleRate */
		void prepare(double);

		/* samples, numChannels, numSamples, midiBuffer */
		void operator()(double* const*, int, int, dsp::MidiBuffer&) noexcept;
		
		/* samples, numChannels, numSamples, midiBuffer */
		void processBlockBypassed(double* const*, int, int, dsp::MidiBuffer&) noexcept;

		void savePatch();
		
		void loadPatch();

		Params& params;
	};
}