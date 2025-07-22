#pragma once
#include "../audio/dsp/Oscilloscope.h"
#include "Button.h"

namespace gui
{
	struct OscilloscopeEditor :
		public Button
	{
		using Oscope = dsp::Oscilloscope;

		OscilloscopeEditor(Utils&, Oscope&);

		void resized() override;

		void paint(Graphics&) override;
	protected:
		const Oscope& oscope;
		BoundsF bounds;
		Path curve;
	public:
		bool bipolar;
	};
}