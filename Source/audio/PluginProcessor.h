#pragma once
#include "dsp/Transport.h"
#include "../param/Param.h"
#include "dsp/SlewLimiter.h"

namespace audio
{
	struct PluginProcessor
	{
		using Params = param::Params;
		using PID = param::PID;
		using MidiBuffer = dsp::MidiBuffer;
		using Transport = dsp::Transport;
		using State = arch::State;
		using XenManager = arch::XenManager;
		
		PluginProcessor(Params&, XenManager&);

		// sampleRate
		void prepare(double);

		// samples, midiBuffer, transport, numChannels, numSamples
		void operator()(double**, MidiBuffer&, const Transport::Info&, int, int) noexcept;
		
		// samples, midiBuffer, numChannels, numSamples
		void processBlockBypassed(double**, MidiBuffer&, int, int) noexcept;

		void savePatch(State&);
		
		void loadPatch(const State&);

		Params& params;
		XenManager& xen;
		double sampleRate;
		dsp::SlewLimiterStereo slew;
	};
}