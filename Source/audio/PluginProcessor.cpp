#include "PluginProcessor.h"
#include "dsp/midi/Sysex.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params& params, TuneSys& tuneSys, State& state, Transport&) :
		synth(),
		sampler(state),
		eq()
	{
		params(PID::NoiseSynth).callback = [&](Param::CB cb)
		{
			synth.setEnabled(cb.getBool());
		};

		params(PID::LowCutFreq).callback = [&](Param::CB cb)
		{
			const auto pitch = cb.denorm();
			eq.setLowCutFreq(tuneSys.noteToFreq(pitch));
		};

		params(PID::HighCutFreq).callback = [&](Param::CB cb)
		{
			const auto pitch = cb.denorm();
			eq.setHighCutFreq(tuneSys.noteToFreq(pitch));
		};

		params(PID::Bell1Freq).callback = [&](Param::CB cb)
		{
			const auto pitch = cb.denorm();
			eq.setBellFreq(tuneSys.noteToFreq(pitch), 0);
		};

		params(PID::Bell1Q).callback = [&](Param::CB cb)
		{
			const auto q = cb.denorm();
			eq.setBellQ(q, 0);
		};

		params(PID::Bell1Gain).callback = [&](Param::CB cb)
		{
			const auto gainDb = cb.denorm();
			eq.setBellGain(gainDb, 0);
		};

		params(PID::Bell2Freq).callback = [&](Param::CB cb)
		{
			const auto pitch = cb.denorm();
			eq.setBellFreq(tuneSys.noteToFreq(pitch), 1);
		};

		params(PID::Bell2Q).callback = [&](Param::CB cb)
		{
			const auto q = cb.denorm();
			eq.setBellQ(q, 1);
		};

		params(PID::Bell2Gain).callback = [&](Param::CB cb)
		{
			const auto gainDb = cb.denorm();
			eq.setBellGain(gainDb, 1);
		};

		params(PID::Bell3Freq).callback = [&](Param::CB cb)
		{
			const auto pitch = cb.denorm();
			eq.setBellFreq(tuneSys.noteToFreq(pitch), 2);
		};

		params(PID::Bell3Q).callback = [&](Param::CB cb)
		{
			const auto q = cb.denorm();
			eq.setBellQ(q, 2);
		};

		params(PID::Bell3Gain).callback = [&](Param::CB cb)
		{
			const auto gainDb = cb.denorm();
			eq.setBellGain(gainDb, 2);
		};
	}

	void PluginProcessor::prepare(double sampleRate) noexcept
	{
		eq.prepare(sampleRate);
	}

	void PluginProcessor::operator()(ProcessorBufferView& view,
		const Transport::Info&) noexcept
	{
		synth(view);
		eq(view);
	}

	void PluginProcessor::processBlockBypassed(float**, MidiBuffer&, int, int) noexcept
	{}

	void PluginProcessor::savePatch(State&)
	{
	}

	void PluginProcessor::loadPatch(const State&)
	{
	}
}