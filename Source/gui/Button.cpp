#include "Button.h"

namespace gui
{
	Button::Button(Utils& u) :
		Comp(u),
		label(u),
		onPaint([](Graphics&, const Button&) {}),
		onClick([](const Mouse&) {}),
		onWheel([](const Mouse&, const MouseWheel&) {}),
		clickAniPhase(0.f),
		value(0.f)
	{
		addAndMakeVisible(label);
	}

	void Button::paint(Graphics& g)
	{
		onPaint(g, *this);
	}

	const String& Button::getText() const noexcept
	{
		return label.text;
	}

	String& Button::getText() noexcept
	{
		return label.text;
	}

	void Button::resized()
	{
		label.setBounds(getLocalBounds().toFloat().reduced(utils.thicc).toNearestInt());
		if(label.type == Label::Type::Text)
			label.setMaxHeight();
	}

	void Button::mouseEnter(const Mouse& mouse)
	{
		Comp::mouseEnter(mouse);
		repaint();
	}

	void Button::mouseExit(const Mouse& mouse)
	{
		Comp::mouseExit(mouse);
		repaint();
	}

	void Button::mouseDown(const Mouse& mouse)
	{
		Comp::mouseDown(mouse);
		repaint();
	}

	void Button::mouseUp(const Mouse& mouse)
	{
		utils.giveDAWKeyboardFocus();

		if (mouse.mouseWasDraggedSinceMouseDown())
			return;

		const auto fps = cbFPS::k30;
		clickAniPhase = 1.f;
		addCallback(Callback([&, fps]()
			{
				clickAniPhase -= msToInc(ClickAniLengthMs, fps);
				if (clickAniPhase <= 0.f)
				{
					clickAniPhase = 0.f;
					removeCallbacks(kClickAniCB);
				}
				repaint();
			}, kClickAniCB), fps);

		onClick(mouse);
		repaint();
	}

	void Button::mouseWheelMove(const Mouse& mouse, const MouseWheel& wheel)
	{
		onWheel(mouse, wheel);
	}

	//////

	Button::OnPaint makeButtonOnPaint(Colour col) noexcept
	{
		return [col](Graphics& g, const Button& b)
			{
				const auto& utils = b.utils;
				const auto thicc = utils.thicc;
				const auto bounds = b.getLocalBounds().toFloat().reduced(thicc);
				const auto over = b.isMouseOver();
				const auto down = b.isMouseButtonDown();
				const auto alphaBase = down ? .5f : over ? .25f : 0.f;
				const auto alphaRange = 1.f - alphaBase;
				const auto ani = b.clickAniPhase * b.clickAniPhase;
				const auto alpha = alphaBase + alphaRange * ani;
				const auto aniCol = col.withAlpha(alpha);
				g.setColour(aniCol);
				g.fillRoundedRectangle(bounds, thicc * .5f);
			};
	}

	//////

	void makeTextButton(Button& btn, const String& txt, const String& tooltip, Colour col)
	{
		makeTextLabel(btn.label, txt, font::nel(), Just::centred, col);
		btn.tooltip = tooltip;
		btn.onPaint = makeButtonOnPaint(col);
	}

	void makePaintButton(Button& btn, const Label::OnPaint& onPaint, const String& tooltip, Colour col)
	{
		makePaintLabel(btn.label, onPaint);
		btn.tooltip = tooltip;
		btn.onPaint = makeButtonOnPaint(col);
	}
}