#include "AutoMPE.h"

namespace dsp
{
	AutoMPE::Voice::Voice() :
		note(-1),
		channel(0),
		noteOn(false)
	{}

	AutoMPE::AutoMPE() :
		buffer(),
		voices(),
		idx(0)
	{}

	void AutoMPE::operator()(MidiBuffer& midiMessages)
	{
		buffer.clear();

		for (const auto it : midiMessages)
		{
			auto msg = it.getMessage();

			if (msg.isNoteOn())
				processNoteOn(msg, it.samplePosition);
			else if (msg.isNoteOff())
				processNoteOff(msg);

			buffer.addEvent(msg, it.samplePosition);
		}

		midiMessages.swapWith(buffer);
	}

	void AutoMPE::processNoteOn(MidiMessage& msg, int ts)
	{
		for (auto ch = 0; ch < NumChannels; ++ch)
		{
			idx = (idx + 1) & MaxMIDIChannel;
			auto& voice = voices[idx];
			if (!voice.noteOn)
			{
				const auto note = msg.getNoteNumber();
				voice.note = note;
				voice.channel = idx + 1;
				voice.noteOn = true;
				msg.setChannel(voice.channel);
				return;
			}
		}
		idx = (idx + 1) & MaxMIDIChannel;
		auto& voice = voices[idx];
		buffer.addEvent(MidiMessage::noteOff(voice.channel, voice.note), ts);
		const auto note = msg.getNoteNumber();
		voice.note = note;
		voice.channel = idx + 1;
		voice.noteOn = true;
		msg.setChannel(voice.channel);
	}

	void AutoMPE::processNoteOff(MidiMessage& msg) noexcept
	{
		for (auto ch = 0; ch < NumChannels; ++ch)
		{
			auto i = idx - ch;
			if (i < 0)
				i += NumChannels;

			auto& voice = voices[i];

			if (voice.noteOn && voice.note == msg.getNoteNumber())
			{
				msg.setChannel(voice.channel);
				voice.note = 0;
				voice.noteOn = false;
				return;
			}
		}
	}
}