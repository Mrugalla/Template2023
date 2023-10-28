#pragma once
#include "Label.h"

namespace gui
{
	struct Button :
		public Comp
	{
		static constexpr float AniLengthMs = 200.f;

		using OnPaint = std::function<void(Graphics&, const Button&)>;
		using OnClick = std::function<void(const Mouse&)>;
		using OnWheel = std::function<void(const Mouse&, const MouseWheel&)>;
		enum class Type { kBool, kInt, kNumTypes };
		enum { kHoverAniCB, kClickAniCB, kNumCallbacks };

		/* u */
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
		float hoverAniPhase, clickAniPhase;
		float value;
		Type type;
	};

	/* type */
	Button::OnPaint makeButtonOnPaint(Button::Type) noexcept;

	//////

	/* btn, text, tooltip, cID */
	void makeTextButton(Button&, const String&, const String&, CID);

	/* btn, onPaint, tooltip */
	void makePaintButton(Button&, const Label::OnPaint&, const String&);

	//////

	inline void makeParameter(Button& button, PID pID)
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
		
		button.addCallback(Callback([&btn = button, pID]()
		{
			const auto& utils = btn.utils;
			const auto& param = utils.getParam(pID);
			const auto val = param.getValue();

			if (btn.value == val)
				return;

			btn.value = val;
			btn.repaint();

		}, 3, cbFPS::k15, true));

		button.registerCallbacks();
	}
}