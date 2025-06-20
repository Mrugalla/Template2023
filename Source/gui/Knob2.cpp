#include "Knob2.h"

namespace gui
{
	Knob2::Knob2(Utils& u) :
		Comp(u),
		knob(u),
		label(u),
		modDial(u),
		style(KnobStyle::Knob)
	{
		layout.init
		(
			{ 1 },
			{ 8, 3 }
		);

		addAndMakeVisible(label);
		addAndMakeVisible(knob);
		setInterceptsMouseClicks(false, true);
	}

	void Knob2::init(PID pID, const String& txt, KnobStyle _style)
	{
		style = _style;
		makeKnob(pID, knob, style);
		makeTextLabel(label, txt, font::text(), Just::centred, CID::Txt);
		const bool modulatable = utils.getParam(pID).isModulatable();
		if (!modulatable)
			return;
		addAndMakeVisible(modDial);
		modDial.attach(pID);
		if (style == KnobStyle::Slider || style == KnobStyle::TextKnob)
			modDial.verticalDrag = false;
	}

	void Knob2::resized()
	{
		Comp::resized();
		switch (style)
		{
		case KnobStyle::TextKnob:
			layout.place(knob, 0, 0, 1, 1);
			break;
		default: // knob
			layout.place(knob, 0, 0.f, 1, 1.5f);
		}
		layout.place(label, 0, 1, 1, 1);
		followKnob(modDial, knob, style);
	}

	void Knob2::setMaxHeight(float margin) noexcept
	{
		label.setMaxHeight(margin);
	}
}