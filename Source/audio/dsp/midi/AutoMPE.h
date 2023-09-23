#pragma once
#include "../../Using.h"

namespace dsp
{
	struct AutoMPE
	{
		struct Voice
		{
			Voice();

			int note, channel;
			bool noteOn;
		};

		using Voices = std::array<Voice, NumMIDIChannels>;

		AutoMPE();

		void operator()(MidiBuffer&);

	private:
		MidiBuffer buffer;
		Voices voices;
		int idx;

		void processNoteOn(MidiMessage&, int);

		void processNoteOff(MidiMessage&) noexcept;
	};
}