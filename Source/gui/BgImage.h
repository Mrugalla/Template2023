#pragma once
#include "Comp.h"

namespace gui
{
	struct BgImage :
		public Comp
	{
        BgImage(Utils&);

        void paint(Graphics&);

        void resized() override;

		/* forced */
		void updateBgImage(bool);

	protected:
		Image img;
		//img refresh button
	};
}