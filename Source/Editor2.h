#pragma once
#include "gui/Knob2.h"

namespace gui
{
	struct Editor2 :
		public Comp
	{
		Editor2(Utils& u) :
			Comp(u),
			order(u)
		{
			layout.init
			(
				{ 1 },
				{ 1, 1 }
			);

			add(order);
			order.init(PID::FFTOrder, "FFT Size", KnobStyle::TextKnob);

			add(Callback([&]() { repaint(); }, 0, cbFPS::k30, true));
		}

		void paint(Graphics& g) override
		{
			setCol(g, CID::Txt);
			const String txt(round(utils.audioProcessor.pluginProcessor.freqHz));
			g.setFont(60.f);
			g.drawFittedText(txt, layout(0, 0, 1, 1).toNearestInt(), Just::centred, 1);
		}

		void resized() override
		{
			Comp::resized();
			layout.place(order, 0, 1, 1, 1);
			order.setMaxHeight();
		}
	private:
		Knob2 order;
	};
}