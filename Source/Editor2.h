#pragma once
#include "gui/Comp.h"

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
				{ 1 },
				{ 1 }
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