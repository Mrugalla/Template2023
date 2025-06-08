#pragma once
#include "Button.h"

namespace gui
{
#if PPDHasSidechain
	struct ButtonSCAutogain :
		public Comp
	{
		ButtonSCAutogain(Utils&);

		~ButtonSCAutogain();

		void paint(Graphics& g) override;

		void resized() override;
	private:
		Button btn;
	};
#endif
}