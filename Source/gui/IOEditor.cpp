#include "IOEditor.h"

namespace gui
{
	IOEditor::IOEditor(Utils& u) :
		Comp(u),
#if PPDIO == PPDIOWetMix || PPDIO == PPDDryWet
		dryOrWet(u),
		wetOrMix(u),
#endif
		gainOut(u),
		power(u)
	{
#if PPDIO == PPDIOWetMix || PPDIO == PPDDryWet
		layout.init
		(
			{ 2, 2, 2, 2 },
			{ 1 }
		);
		addAndMakeVisible(dryOrWet);
		addAndMakeVisible(wetOrMix);
#else
		layout.init
		(
			{ 4, 2 },
			{ 1 }
		);
#endif
		addAndMakeVisible(gainOut);
		addAndMakeVisible(power);
		const auto style = KnobStyle::Knob;
		gainOut.init(PID::GainOut, "Volume", style);
#if PPDIO == PPDIOWetMix
		dryOrWet.init(PID::GainWet, "Wet", style);
		wetOrMix.init(PID::Mix, "Mix", style);
#elif PPDIO == PPDDryWet
		dryOrWet.init(PID::GainDry, "Dry", style);
		wetOrMix.init(PID::Wet, "Wet", style);
#endif
	}

	void IOEditor::resized()
	{
		Comp::resized();
#if PPDIO == PPDIOWetMix || PPDIO == PPDDryWet
		layout.place(dryOrWet, -5, 0, 1, 1);
		layout.place(wetOrMix, -4, 0, 1, 1);
#endif
		layout.place(gainOut, -3, 0, 1, 1);
		layout.place(power, -2, 0, 1, 1);
	}
}