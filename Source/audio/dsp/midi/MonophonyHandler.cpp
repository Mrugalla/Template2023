#include "MonophonyHandler.h"

namespace dsp
{
	MonophonyHandler::MonophonyHandler() :
		heldNotes(),
		curNote(-1),
		polyphony(0)
	{
	}

	void MonophonyHandler::operator()(MidiBuffer& midi, int poly)
	{
		buffer.clear();
		updatePoly(poly);
		const bool monophonic = polyphony == 1;
		if (monophonic)
		{
			processBlockMono(midi);
			midi.swapWith(buffer);
			return;
		}
		processBlockPoly(midi);
	}

	void MonophonyHandler::updatePoly(int poly)
	{
		if (polyphony == poly)
			return;
		polyphony = poly;
		const bool gotMono = polyphony == 1;
		if (!gotMono)
			return;
		for (auto i = 0; i < 128; ++i)
		{
			if (heldNotes[i] != 0)
			{
				buffer.addEvent(MidiMessage::noteOff(1, i), 0);
				heldNotes[i] = 0;
			}
		}
		curNote = -1;
	}

	void MonophonyHandler::processBlockMono(MidiBuffer& midi)
	{
		for (const auto it : midi)
		{
			auto msg = it.getMessage();
			const auto ts = it.samplePosition;
			if (msg.isNoteOn())
				processNoteOn(msg, ts);
			else if (msg.isNoteOff())
				processNoteOff(msg, ts);
			else
				buffer.addEvent(msg, ts);
		}
	}

	void MonophonyHandler::processBlockPoly(const MidiBuffer& midi) noexcept
	{
		for (const auto it : midi)
		{
			const auto msg = it.getMessage();
			if (msg.isNoteOn())
				registerNoteOn(msg);
			else if (msg.isNoteOff())
				registerNoteOff(msg);
		}
	}

	void MonophonyHandler::processNoteOn(const MidiMessage& msg, int ts) noexcept
	{
		const auto velo = msg.getVelocity();
		if (curNote != -1 || velo == 0)
			buffer.addEvent(MidiMessage::noteOff(1, curNote), ts);
		registerNoteOn(msg);
		buffer.addEvent(MidiMessage::noteOn(1, curNote, velo), ts);
	}

	void MonophonyHandler::processNoteOff(MidiMessage& msg, int ts) noexcept
	{
		curNote = msg.getNoteNumber();
		buffer.addEvent(MidiMessage::noteOff(1, curNote), ts);
		registerNoteOff(msg);
		for (auto i = 0; i < 128; ++i)
			if (heldNotes[i] != 0)
			{
				curNote = i;
				buffer.addEvent(MidiMessage::noteOn(1, curNote, heldNotes[i]), ts);
				return;
			}
	}

	void MonophonyHandler::registerNoteOn(const MidiMessage& msg) noexcept
	{
		curNote = msg.getNoteNumber();
		heldNotes[curNote] = msg.getVelocity();
	}

	void MonophonyHandler::registerNoteOff(const MidiMessage& msg) noexcept
	{
		curNote = -1;
		heldNotes[msg.getNoteNumber()] = 0;
	}
}