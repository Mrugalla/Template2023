#pragma once
#include "gui/Knob2.h"

namespace gui
{
	struct Editor2 :
		public Comp
	{
		Editor2(Utils& u) :
			Comp(u),
			order(u),
			smooth(u)
		{
			layout.init
			(
				{ 1, 1 },
				{ 1, 2 }
			);

			add(order);
			add(smooth);

			order.init(PID::FFTOrder, "FFT Size", KnobStyle::TextKnob);
			smooth.init(PID::Smooth, "Smooth", KnobStyle::Knob);

			add(Callback([&]() { repaint(); }, 0, cbFPS::k30, true));
		}

		void paint(Graphics& g) override
		{
			setCol(g, CID::Txt);
			String txt;
			const auto& pitchDetector = utils.audioProcessor.pluginProcessor.pitchDetector;
			const auto info = pitchDetector.getInfo();
			if (info.tonal)
				txt += String(std::round(info.freqHz)) + " hz";
			else
				txt += "-";
			g.setFont(60.f);
			g.drawFittedText(txt, layout.top().toNearestInt(), Just::centred, 1);
		}

		void resized() override
		{
			Comp::resized();
			layout.place(order, 0, 1, 1, 1);
			layout.place(smooth, 1, 1, 1, 1);
			order.setMaxHeight();
		}
	private:
		Knob2 order, smooth;
	};
}