#pragma once
#include "Label.h"

namespace gui
{
	struct Button :
		public Comp
	{
		using OnClick = std::function<void(const Mouse&)>;
		using OnWheel = std::function<void(const Mouse&, const MouseWheel&)>;
		enum { kClickAniCB, numCallbacks };

		Button(Utils& u, const String& text = "", const String& _tooltip = "") :
			Comp(u, _tooltip),
			label(u, text, ""),
			onClick([](const Mouse&) {}),
			onWheel([](const Mouse&, const MouseWheel&) {}),
			clickAnimationPhase(0.f),
			toggleState(0)
		{
			addAndMakeVisible(label);
		}

		void paint(Graphics& g) override
		{
			g.fillAll(juce::Colours::white.withAlpha(clickAnimationPhase));
		}

		const String& getText() const noexcept
		{
			return label.text;
		}

		String& getText() noexcept
		{
			return label.text;
		}

		void resized() override
		{
			label.setBounds(getLocalBounds());
		}

		void mouseEnter(const Mouse& mouse) override
		{
			Comp::mouseEnter(mouse);
			repaint();
		}

		void mouseExit(const Mouse&) override
		{
			repaint();
		}

		void mouseUp(const Mouse& mouse) override
		{
			utils.giveDAWKeyboardFocus();

			if (mouse.mouseWasDraggedSinceMouseDown())
				return;

			const auto fps = cbFPS::k30;
			clickAnimationPhase = 1.f;
			addCallback(Callback([&, fps]()
			{
				clickAnimationPhase -= msToInc(200.f, fps);
				if (clickAnimationPhase <= 0.f)
				{
					clickAnimationPhase = 0.f;
					removeCallbacks(kClickAniCB);
				}
				repaint();
			}, kClickAniCB), fps);
			
			onClick(mouse);
		}

		void mouseWheelMove(const Mouse& mouse, const MouseWheel& wheel) override
		{
			onWheel(mouse, wheel);
		}

		Label label;
		OnClick onClick;
		OnWheel onWheel;
		float clickAnimationPhase;
		int toggleState;
	};
}