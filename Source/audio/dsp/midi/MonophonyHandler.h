#pragma once
#include "../Using.h"

namespace dsp
{
	struct MonophonyHandler
	{
		using uint8 = juce::uint8;

		MonophonyHandler();

		// midi, poly
		void operator()(MidiBuffer&, int);

	private:
		MidiBuffer buffer;
		std::array<uint8, 128> heldNotes;
		int curNote, polyphony;

		// poly
		void updatePoly(int);

		void processBlockMono(MidiBuffer&);

		void processBlockPoly(const MidiBuffer&) noexcept;

		// msg, ts
		void processNoteOn(const MidiMessage&, int) noexcept;

		// msg, ts
		void processNoteOff(MidiMessage&, int) noexcept;

		void registerNoteOn(const MidiMessage&) noexcept;

		void registerNoteOff(const MidiMessage&) noexcept;
	};
}