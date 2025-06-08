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

	private:
		Knob knob;
		Label label;
		ModDial modDial;
		KnobStyle style;
	};
}