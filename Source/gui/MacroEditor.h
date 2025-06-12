#pragma once
#include "Knob2.h"

namespace gui
{
	struct MacroEditor :
		public Comp
	{
		MacroEditor(Utils&);

		void resized() override;
	private:
		Knob2 macro;
		Button mode, swap;
	};
}