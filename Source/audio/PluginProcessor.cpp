#include "PluginProcessor.h"

namespace dsp
{
	PluginProcessor::PluginProcessor(Params& params,
#if PPDHasTuningEditor
		XenManager& xen,
#endif
		Transport& transport
	) :
		sampleRate(1.),
		freqShifter()
	{
		params(PID::Reflect).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setReflect(cb.getInt());
		};

		params(PID::Temposync).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setTemposync(cb.getBool(), cb.numChannels);
		};

		params(PID::ShiftHz).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setShiftHz(cb.denorm(), cb.numChannels);
		};

		params(PID::ShiftBeats).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setShiftBeats(cb.denorm(), cb.numChannels);
		};

		params(PID::PhaseOffset).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setPhaseOffset(cb.norm, cb.numChannels);
		};

		params(PID::Feedback).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setFeedback(cb.denorm(), cb.numChannels);
		};

		params(PID::ShiftHzWidth).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setShiftHzWidth(cb.denorm(), cb.numChannels);
		};

		params(PID::ShiftBeatsWidth).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setShiftBeatsWidth(cb.denorm(), cb.numChannels);
		};

		params(PID::PhaseOffsetWidth).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setPhaseOffsetWidth(cb.denorm(), cb.numChannels);
		};

		params(PID::FeedbackWidth).callback = [&](const param::Param::CB& cb) noexcept
		{
			freqShifter.setFeedbackWidth(cb.denorm(), cb.numChannels);
		};

		transport.callback = [&](const Transport::Info& info) noexcept
		{
			freqShifter.setBpm(info.bpm, info.numChannels);
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