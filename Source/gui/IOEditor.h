#pragma once
#include "Knob2.h"
#include "ButtonPower.h"

namespace gui
{
	struct IOEditor :
		public Comp
	{
		IOEditor(Utils& u) :
			Comp(u),
			gainOut(u),
			power(u)
		{
			layout.init
			(
				{ 4, 2 },
				{ 1 }
			);

			addAndMakeVisible(gainOut);
			addAndMakeVisible(power);

			gainOut.init(PID::GainOut, "Volume", KnobStyle::TextKnob);
		}

		void resized() override
		{
			Comp::resized();
			layout.place(gainOut, 0, 0, 1, 1);
			layout.place(power, 1, 0, 1, 1);
		}

	private:
		Knob2 gainOut;
		ButtonPower power;
	};
}