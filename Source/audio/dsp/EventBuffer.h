#pragma once
#include "Using.h"

namespace dsp
{
	struct Event
	{
		enum class Type
		{
			MIDI,
			Onset,
			NumTypes
		};
		static constexpr int NumTypes = static_cast<int>(Type::NumTypes);

		Event(const MidiMessage& m) :
			stuff(nullptr),
			msg(std::move(m)),
			type(Type::MIDI)
		{
		}

		Event(Type t, const void* s) :
			stuff(s),
			msg(),
			type(t)
		{}

		const void* stuff;
		MidiMessage msg;
		Type type;
	};
}