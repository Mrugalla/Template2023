#pragma once
#include "../arch/TuneSys.h"
#include "../arch/Param.h"
#include "dsp/ProcessorBufferView.h"
#include "dsp/Transport.h"
//
#include "dsp/RumbleSynth.h"
#include "dsp/RumbleSampler.h"
#include "dsp/RumbleEQ.h"

namespace dsp
{
	using Params = param::Params;
	using Param = param::Param;
	using PID = param::PID;
	using State = arch::State;
	using CB = param::Param::CB;
	using TuneSys = arch::TuneSys;

	struct PluginProcessor
	{
		PluginProcessor(Params&, TuneSys&, State&, Transport&);

		// sampleRate
		void prepare(double) noexcept;

		// bufferView, transport
		void operator()(ProcessorBufferView&, const Transport::Info&) noexcept;
		
		// samples, midiBuffer, numChannels, numSamples
		void processBlockBypassed(float**, MidiBuffer&, int, int) noexcept;

		void savePatch(State&);
		
		void loadPatch(const State&);

		//
		RumbleSynth synth;
		RumbleSampler sampler;
		RumbleEQ eq;
	};
}