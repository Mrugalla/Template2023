#include "MacroEditor.h"

namespace gui
{
	MacroEditor::MacroEditor(Utils& u) :
		Comp(u),
		macro(u),
		mode(u),
		swap(u)
	{
		layout.init
		(
			{ 4, 3 },
			{ 1, 1 }
		);

		addAndMakeVisible(macro);
		addAndMakeVisible(mode);
		addAndMakeVisible(swap);

		macro.init(PID::Macro, "Macro", KnobStyle::Knob);

		swap.setTooltip("Swap all parameters' main values with their modulation destinations.");
		swap.onPaint = makeButtonOnPaintSwap();
		swap.onClick = [&u = utils](const Mouse&)
		{
			for (auto param : u.audioProcessor.params.data())
			{
				const auto val = param->getValue();
				const auto modDepth = param->getModDepth();
				const auto valModMax = juce::jlimit(0.f, 1.f, val + modDepth);
				const auto modBias = param->getModBias();

				param->beginGesture();
				param->setValueNotifyingHost(valModMax);
				param->endGesture();
				param->setModDepth(juce::jlimit(-1.f, 1.f, val - valModMax));
				param->setModBias(1.f - modBias);
			}
		};

		makePaintButton(mode, [](Graphics& g, const Button& b)
		{
			const auto thicc = b.utils.thicc;
			const auto thiccHalf = thicc * .5f;
			const auto thicc3 = thicc * 3.f;
			const auto thicc5 = thicc * 5.f;

			const auto bounds = b.getLocalBounds().toFloat().reduced(thicc);

			const auto hoverPhase = b.callbacks[Button::kHoverAniCB].phase;
			const auto togglePhase = b.callbacks[Button::kToggleStateCB].phase;

			const auto bgColour = getColour(CID::Bg).interpolatedWith(getColour(CID::Interact), togglePhase * .3f);
			g.setColour(bgColour);
			const auto lineThicc = thicc + hoverPhase * thiccHalf;
			const auto boundsBg = bounds.reduced((1.f - hoverPhase) * bounds.getWidth() * .5f);
			g.fillRoundedRectangle(boundsBg, lineThicc);

			setCol(g, CID::Interact);

			const auto aWidth = togglePhase * bounds.getWidth() * .6f;
			const auto aX0 = bounds.getX();
			const auto aX1 = aX0 + aWidth;
			if (togglePhase > 0.f)
			{
				const auto aY = bounds.getY() + bounds.getHeight() * .5f;
				const LineF arrow(aX0, aY, aX1, aY);
				const auto arrowStuff = thicc5 + (1.f - togglePhase) * thicc3;
				g.drawArrow(arrow, lineThicc, arrowStuff, arrowStuff);
			}

			const auto cX = aX1;
			const auto cY = bounds.getY();
			const auto cW = bounds.getWidth() - aWidth;
			const auto cH = bounds.getHeight();
			const auto circle = maxQuadIn(BoundsF(cX, cY, cW, cH)).reduced(thicc3);
			g.drawEllipse(circle, lineThicc);

		},
		"If enabled, macro modulation depth is relative to the main value.");
		mode.type = Button::Type::kToggle;
		mode.onClick = [&](const Mouse&)
		{
			u.audioProcessor.params.switchModDepthAbsolute();
			mode.value = u.audioProcessor.params.isModDepthAbsolute() ? 0.f : 1.f;
			mode.label.setText(mode.value > .5f ? "Rel" : "Abs");
			mode.label.repaint();
		};
		mode.value = utils.audioProcessor.params.isModDepthAbsolute() ? 0.f : 1.f;

		const auto fps = cbFPS::k15;
		const auto inc = msToInc(1000.f, fps);
		add(Callback([&, inc]()
		{
			auto& phase = callbacks[0].phase;
			phase += inc;
			if (phase < 1.f)
				return;
			callbacks[0].stop(0.f);
			mode.value = utils.audioProcessor.params.isModDepthAbsolute() ? 0.f : 1.f;
			mode.label.setText(mode.value > .5f ? "Rel" : "Abs");
			mode.label.repaint();
		}, 0, fps, true));
	}

	void MacroEditor::resized()
	{
		layout.resized(getLocalBounds());
		layout.place(macro, 0, 0, 1, 2);
		layout.place(mode, 1, 0, 1, 1);
		layout.place(swap, 1, 1, 1, 1);
	}
}