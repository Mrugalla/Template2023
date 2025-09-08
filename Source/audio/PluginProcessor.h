#pragma once
#include "dsp/ProcessorBufferView.h"
#include "dsp/Transport.h"
#include "../arch/Param.h"
//

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
		PluginProcessor(Params&,
#if PPDHasTuningEditor
			XenManager&,
#endif
			Transport&
		);

		// sampleRate
		void prepare(double);

		// bufferView, transport
		void operator()(ProcessorBufferView&, const Transport::Info&) noexcept;
		
		// samples, midiBuffer, numChannels, numSamples
		void processBlockBypassed(float**, MidiBuffer&, int, int) noexcept;

		void savePatch(State&);
		
		void loadPatch(const State&);

		double sampleRate;
		//
	};
}