#pragma once
#include "../../Using.h"

namespace dsp
{
	struct MPESplit
	{
		static constexpr int Sysex = 0;

		MPESplit();

		void operator()(MidiBuffer&);

		MidiBuffer& operator[](int ch) noexcept;

		const MidiBuffer& operator[](int ch) const noexcept;

	protected:
		MidiBuffersMPE buffers;
	};
}