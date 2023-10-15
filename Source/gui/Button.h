#pragma once
#include "Label.h"

namespace gui
{
	struct Button :
		public Comp
	{
		static constexpr float ClickAniLengthMs = 400.f;

		using OnPaint = std::function<void(Graphics&, const Button&)>;
		using OnClick = std::function<void(const Mouse&)>;
		using OnWheel = std::function<void(const Mouse&, const MouseWheel&)>;
		enum { kClickAniCB, numCallbacks };

		/* u, text, tooltip */
		Button(Utils&, const String & = "", const String & = "");

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
		float clickAniPhase;
		int toggleState;
	};

	/* bgCol */
	Button::OnPaint makeButtonOnPaint(Colour) noexcept;

	/* btn, col */
	void makeTextButton(Button&, Colour) noexcept;
}