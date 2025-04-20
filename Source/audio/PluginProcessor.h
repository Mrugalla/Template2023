#pragma once
#include "dsp/Transport.h"
#include "../param/Param.h"

namespace dsp
{
	using Params = param::Params;
	using PID = param::PID;
	using State = arch::State;
	using XenManager = arch::XenManager;

	struct PluginProcessor
	{
		PluginProcessor();

		// sampleRate
		void prepare(double);

		// samples, midiBuffer, transport, numChannels, numSamples
		void operator()(double**, MidiBuffer&, const Transport::Info&, int, int) noexcept;
		
		// samples, midiBuffer, numChannels, numSamples
		void processBlockBypassed(double**, MidiBuffer&, int, int) noexcept;

		void savePatch(State&);
		
		void loadPatch(const State&);

		double sampleRate;
	};
}