#pragma once
#include "Label.h"

namespace gui
{
	struct Button :
		public Comp
	{
		static constexpr float DragThreshold = 8.f;

		using OnPaint = std::function<void(Graphics&, const Button&)>;
		using OnClick = std::function<void(const Mouse&)>;
		using OnWheel = std::function<void(const Mouse&, const MouseWheel&)>;
		enum class Type { kTrigger, kToggle, kChoice, kNumTypes };
		enum { kHoverAniCB, kClickAniCB, kToggleStateCB, kUpdateParameterCB, kNumCallbacks };

		// u
		Button(Utils&);

		void paint(Graphics&) override;

		const String& getText() const noexcept;

		String& getText() noexcept;

		void resized() override;

		void mouseEnter(const Mouse&) override;

		void mouseExit(const Mouse&) override;

		void mouseDown(const Mouse&) override;

		void mouseUp(const Mouse&) override;

		void mouseWheelMove(const Mouse&, const MouseWheel&) override;

		Label label;
		OnPaint onPaint;
		OnClick onClick;
		OnWheel onWheel;
		float value;
		Type type;
	};

	////// ONPAINTS:
	
	// drawToggle, bgColour
	Button::OnPaint makeButtonOnPaint(bool, Colour);

	Button::OnPaint makeButtonOnPaintPower();

	Button::OnPaint makeButtonOnPaintPolarity();

	Button::OnPaint makeButtonOnPaintSwap();

	Button::OnPaint makeButtonOnPaintClip();

	Button::OnPaint makeButtonOnPaintSave();

	// numDiagnoalLines
	Button::OnPaint makeButtonOnPaintVisor(int);

	////// LOOK AND FEEL:

	// btn, text, tooltip, cID, bgCol
	void makeTextButton(Button&, const String&, const String&, CID, Colour = getColour(CID::Bg));

	// btn, onPaint, tooltip
	void makePaintButton(Button&, const Button::OnPaint&, const String&);

	////// PARAMETER ATTACHMENT:

	// PID, button
	void makeButton(PID, Button&);

	// PID, button, type, name
	void makeButton(PID, Button&, Button::Type, const String& = "");

	// PID, button, type, onPaint
	void makeButton(PID, Button&, Button::Type, Button::OnPaint);

	void makeButtons(PID, std::vector<std::unique_ptr<Button>>&);
}