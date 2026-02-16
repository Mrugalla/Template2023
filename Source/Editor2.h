#pragma once
#include "gui/RumbleEQEditor.h"
#include "gui/OscilloscopeEditor.h"

namespace gui
{
	struct Editor2 :
		public Comp
	{
		Editor2(Utils& u) :
			Comp(u),
			noiseSynthButton(u),
			lowCutFreqKnob(u), highCutFreqKnob(u),
			bell1FreqKnob(u), bell1QKnob(u), bell1GainKnob(u),
			bell2FreqKnob(u), bell2QKnob(u), bell2GainKnob(u),
			bell3FreqKnob(u), bell3QKnob(u), bell3GainKnob(u),
			eqEditor(u, u.audioProcessor.pluginProcessor.eq, 				
				{
					PID::LowCutFreq, PID::HighCutFreq,
					PID::Bell1Freq, PID::Bell1Q, PID::Bell1Gain,
					PID::Bell2Freq, PID::Bell2Q, PID::Bell2Gain,
					PID::Bell3Freq, PID::Bell3Q, PID::Bell3Gain
				})
		{
			layout.init
			(
				{ 1, 1, 1 },
				{ 1, 2, 2, 2, 4 }
			);

			addAndMakeVisible(noiseSynthButton);
			addAndMakeVisible(lowCutFreqKnob);
			addAndMakeVisible(highCutFreqKnob);
			addAndMakeVisible(bell1FreqKnob);
			addAndMakeVisible(bell2FreqKnob);
			addAndMakeVisible(bell3FreqKnob);
			addAndMakeVisible(bell1QKnob);
			addAndMakeVisible(bell2QKnob);
			addAndMakeVisible(bell3QKnob);
			addAndMakeVisible(bell1GainKnob);
			addAndMakeVisible(bell2GainKnob);
			addAndMakeVisible(bell3GainKnob);
			addAndMakeVisible(eqEditor);

			makeButton(PID::NoiseSynth, noiseSynthButton, Button::Type::kToggle, "Synth");
			lowCutFreqKnob.init(PID::LowCutFreq, "Low Cut Freq", KnobStyle::Knob);
			highCutFreqKnob.init(PID::HighCutFreq, "High Cut Freq", KnobStyle::Knob);
			bell1FreqKnob.init(PID::Bell1Freq, "Bell 1 Freq", KnobStyle::Knob);
			bell1QKnob.init(PID::Bell1Q, "Bell 1 Q", KnobStyle::Knob);
			bell1GainKnob.init(PID::Bell1Gain, "Bell 1 Gain", KnobStyle::Knob);
			bell2FreqKnob.init(PID::Bell2Freq, "Bell 2 Freq", KnobStyle::Knob);
			bell2QKnob.init(PID::Bell2Q, "Bell 2 Q", KnobStyle::Knob);
			bell2GainKnob.init(PID::Bell2Gain, "Bell 2 Gain", KnobStyle::Knob);
			bell3FreqKnob.init(PID::Bell3Freq, "Bell 3 Freq", KnobStyle::Knob);
			bell3QKnob.init(PID::Bell3Q, "Bell 3 Q", KnobStyle::Knob);
			bell3GainKnob.init(PID::Bell3Gain, "Bell 3 Gain", KnobStyle::Knob);
		}

		void paint(Graphics&) override
		{
		}

		void resized() override
		{
			Comp::resized();
			layout.place(noiseSynthButton, 0, 0, 1, 1);
			layout.place(lowCutFreqKnob, 1, 0, 1, 1);
			layout.place(highCutFreqKnob, 2, 0, 1, 1);
			layout.place(bell1FreqKnob, 0, 1, 1, 1);
			layout.place(bell2FreqKnob, 1, 1, 1, 1);
			layout.place(bell3FreqKnob, 2, 1, 1, 1);
			layout.place(bell1QKnob, 0, 2, 1, 1);
			layout.place(bell2QKnob, 1, 2, 1, 1);
			layout.place(bell3QKnob, 2, 2, 1, 1);
			layout.place(bell1GainKnob, 0, 3, 1, 1);
			layout.place(bell2GainKnob, 1, 3, 1, 1);
			layout.place(bell3GainKnob, 2, 3, 1, 1);
			layout.place(eqEditor, 0, 4, 3, 1);
		}
	private:
		Button noiseSynthButton;
		Knob2 lowCutFreqKnob, highCutFreqKnob,
			bell1FreqKnob, bell1QKnob, bell1GainKnob,
			bell2FreqKnob, bell2QKnob, bell2GainKnob,
			bell3FreqKnob, bell3QKnob, bell3GainKnob;
		RumbleEQEditor eqEditor;
	};
}