#include "PluginProcessor.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params& params
#if PPDHasTuningEditor
		, XenManager& xen
#endif
	) :
		sampleRate(1.),
		freqShifter()
	{
		params(PID::Reflect).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setReflect(cb.getInt());
		};

		params(PID::Shift).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setShift(cb.denorm(), cb.numChannels);
		};

		params(PID::PhaseOffset).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setPhaseOffset(cb.norm);
		};

		params(PID::Feedback).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setFeedback(cb.denorm());
		};

		params(PID::ShiftWidth).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setShiftWidth(cb.denorm(), cb.numChannels);
		};
	}

	void PluginProcessor::prepare(double _sampleRate)
	{
		sampleRate = _sampleRate;
		freqShifter.prepare(sampleRate);
	}

	void PluginProcessor::operator()(ProcessorBufferView& view,
		const Transport::Info&) noexcept
	{
		freqShifter(view);
		if (view.msg.isNoteOn())
			freqShifter.reset();
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