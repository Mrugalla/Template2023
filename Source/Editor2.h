#pragma once
#include "gui/Knob2.h"
#include "gui/OscilloscopeEditor.h"

namespace gui
{
	struct Editor2 :
		public Comp
	{
		Editor2(Utils& u) :
			Comp(u)
		{
			layout.init
			(
				{ 1, 1 },
				{ 1, 1, 2 }
			);
		}

		void paint(Graphics&) override
		{
		}

		void resized() override
		{
			Comp::resized();
		}
	private:
	};
}