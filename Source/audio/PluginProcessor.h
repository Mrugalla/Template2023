#pragma once
#include "dsp/Transport.h"
#include "../arch/Param.h"

#include "dsp/PitchDetector.h"

namespace dsp
{
	using Params = param::Params;
	using Param = param::Param;
	using PID = param::PID;
	using State = arch::State;
	using CB = param::Param::CB;
#if PPDHasTuningEditor
	using XenManager = arch::XenManager;
#endif
	struct PluginProcessor
	{
		PluginProcessor(Params&
#if PPDHasTuningEditor
			, XenManager&
#endif
		);

		// sampleRate
		void prepare(double);

		// samples, midiBuffer, transport
		void operator()(ProcessorBufferView& buffer, MidiBuffer&, const Transport::Info&) noexcept;
		
		// samples, midiBuffer, numChannels, numSamples
		void processBlockBypassed(float**, MidiBuffer&, int, int) noexcept;

		void savePatch(State&);
		
		void loadPatch(const State&);

		double sampleRate;
		//
		PitchDetector pitchDetector;

		float freqHz;
		float phase, inc, gain;
	};
}