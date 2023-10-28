#include "Button.h"

namespace gui
{
	Button::Button(Utils& u) :
		Comp(u),
		label(u),
		onPaint([](Graphics&, const Button&) {}),
		onClick([](const Mouse&) {}),
		onWheel([](const Mouse&, const MouseWheel&) {}),
		hoverAniPhase(0.f), clickAniPhase(0.f),
		value(0.f),
		type(Type::kBool)
	{
		addAndMakeVisible(label);

		const auto fps = cbFPS::k30;
		add(Callback([&, fps]()
		{
			hoverAniPhase -= msToInc(AniLengthMs, fps);
			if (hoverAniPhase <= 0.f)
			{
				hoverAniPhase = 0.f;
				callbacks[kHoverAniCB].active = false;
			}
			repaint();
		}, kHoverAniCB, fps, false));

		add(Callback([&, fps]()
		{
			clickAniPhase -= msToInc(AniLengthMs, fps);
			if (clickAniPhase <= 0.f)
			{
				clickAniPhase = 0.f;
				callbacks[kClickAniCB].active = false;
			}
			repaint();
		}, kClickAniCB, fps, false));
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
		hoverAniPhase = 1.f;
		callbacks[kHoverAniCB].active = false;
		repaint();
	}

	void Button::mouseExit(const Mouse& mouse)
	{
		Comp::mouseExit(mouse);
		callbacks[kHoverAniCB].active = true;
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

		clickAniPhase = 1.f;
		callbacks[kClickAniCB].active = true;

		onClick(mouse);
		repaint();
	}

	void Button::mouseWheelMove(const Mouse& mouse, const MouseWheel& wheel)
	{
		onWheel(mouse, wheel);
	}

	////// LOOK AND FEEL:

	Button::OnPaint makeButtonOnPaint(Button::Type type) noexcept
	{
		const auto valAlphaFunc = type == Button::Type::kBool ?
		[](float val)
		{
			return val > .5f ? .25f : 0.f;
		} :
		[](float)
		{
			return 0.f;
		};

		return [valAlphaFunc](Graphics& g, const Button& b)
		{
			const auto& utils = b.utils;
			const auto thicc = utils.thicc;
			const auto bounds = b.getLocalBounds().toFloat().reduced(thicc);

			const auto hoverAniPhase = b.hoverAniPhase * b.hoverAniPhase;
			const auto clickAniPhase = b.clickAniPhase * b.clickAniPhase;
			const auto valueAlpha = valAlphaFunc(b.value);
			const auto alpha = hoverAniPhase * .25f + clickAniPhase * .5f + valueAlpha;
			const auto aniCol = getColour(CID::Interact).withAlpha(alpha);
			g.setColour(aniCol);
			g.fillRoundedRectangle(bounds, thicc * .5f);
		};
	}

	void makeTextButton(Button& btn, const String& txt, const String& tooltip, CID cID)
	{
		makeTextLabel(btn.label, txt, font::nel(), Just::centred, cID);
		btn.tooltip = tooltip;
		btn.onPaint = makeButtonOnPaint(btn.type);
	}

	void makePaintButton(Button& btn, const Label::OnPaint& onPaint, const String& tooltip)
	{
		makePaintLabel(btn.label, onPaint);
		btn.tooltip = tooltip;
		btn.onPaint = makeButtonOnPaint(btn.type);
	}

	////// PARAMETER ATTACHMENT:

	void makeParameter(Button& button, PID pID)
	{
		makeTextButton(button, param::toString(pID), param::toTooltip(pID), CID::Interact);

		auto& utils = button.utils;
		auto& param = utils.getParam(pID);
		const auto type = param.getType();
		button.type = type == Param::Type::Bool ? Button::Type::kBool : Button::Type::kInt;

		button.onClick = [&btn = button, pID](const Mouse&)
		{
			auto& utils = btn.utils;
			auto& param = utils.getParam(pID);
			const auto& range = param.range;
			const auto interval = static_cast<int>(range.interval);
			auto valDenorm = static_cast<int>(param.getValueDenorm()) + interval;
			if (valDenorm > static_cast<int>(range.end))
				valDenorm = static_cast<int>(range.start);
			const auto valNorm = range.convertTo0to1(static_cast<float>(valDenorm));
			param.setValueWithGesture(valNorm);
		};

		button.add(Callback([&btn = button, pID]()
		{
			const auto& utils = btn.utils;
			const auto& param = utils.getParam(pID);
			const auto val = param.getValue();

			if (btn.value == val)
				return;

			btn.value = val;
			btn.repaint();

		}, Button::kUpdateParameterCB, cbFPS::k15, true));
	}
}