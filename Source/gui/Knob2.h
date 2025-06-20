#pragma once
#include "Knob.h"

namespace gui
{
	struct Knob2 :
		public Comp
	{
		// utils
		Knob2(Utils&);

		// pID, text, style
		void init(PID, const String&, KnobStyle);
		
		void resized() override;

		// margin
		void setMaxHeight(float = 0.f) noexcept;
	private:
		Knob knob;
		Label label;
		ModDial modDial;
		KnobStyle style;
	};
}