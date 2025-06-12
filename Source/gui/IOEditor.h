#pragma once
#include "Knob2.h"
#include "ButtonPower.h"

namespace gui
{
	struct IOEditor :
		public Comp
	{
		IOEditor(Utils&);

		void resized() override;
	private:
#if PPDIO == PPDIOWetMix || PPDIO == PPDDryWet
		Knob2 dryOrWet, wetOrMix;
#endif
		Knob2 gainOut;
		ButtonPower power;
	};
}