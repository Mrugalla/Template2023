#pragma once
#include "dsp/Transport.h"
#include "../arch/Param.h"

#include "../libs/signalsmith/include/signalsmith-basics/freq-shifter.h"

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

		// butter view, midiBuffer, transport
		void operator()(ProcessorBufferView&, MidiBuffer&, const Transport::Info&) noexcept;
		
		// samples, midiBuffer, numChannels, numSamples
		void processBlockBypassed(float**, MidiBuffer&, int, int) noexcept;

		void savePatch(State&);
		
		void loadPatch(const State&);

		double sampleRate;
		//
		signalsmith::basics::FreqShifterDouble freqShifter;
	};
}