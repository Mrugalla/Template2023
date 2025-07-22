#pragma once
#include "../Using.h"

namespace dsp
{
	struct MidiTranspose
	{
		struct Note
		{
			Note() :
				pitch(0),
				channel(1)
			{}

			int pitch, channel;
		};

		using Notes = std::array<Note, NumMIDIChannels>;

		MidiTranspose();

		// midi, retuneSemi
		void operator()(MidiBuffer&, int);

	protected:
		Notes notes;
		MidiBuffer buffer;
		int retuneSemi, idx;

		// retuneSemi
		bool parameterChanged(int);

		void processTranspose(MidiBuffer&);
	};
}