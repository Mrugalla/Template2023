#pragma once
#include "Button.h"
#include "../audio/dsp/SCGain.h"

namespace gui
{
#if PPDHasSidechain
	struct ButtonSCAutogain :
		public Comp
	{
		using SCGain = dsp::SCGain;

		ButtonSCAutogain(Utils&, SCGain&);

		~ButtonSCAutogain();

		void paint(Graphics&) override;

		void resized() override;
	private:
		Button btn;
		SCGain& scGainer;
	};
#endif
}