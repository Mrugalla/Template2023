#pragma once
#include "juce_core/juce_core.h"

namespace makeRange
{
	using Range = juce::NormalisableRange<float>;
	using String = juce::String;

	// start, end, steps = 1
	Range stepped(float, float, float = 1.f) noexcept;

	Range stepped(int, int, int = 1) noexcept;

	Range toggle() noexcept;

	// start, end
	Range lin(float, float) noexcept;

	Range lin(int, int) noexcept;

	// start, end, bias[-1, 1]
	Range biased(float, float, float) noexcept;

	// start, end, centre
	Range withCentre(float, float, float) noexcept;

	Range withCentre(int, int, int) noexcept;

	// start, end, bias[-1, 1]
	// optimally round�
	Range biasedSatisfy(float, float, float) noexcept;

	Range biasedSatisfy(int, int, float) noexcept;

	// min, max
	Range foleysLogRange(float, float) noexcept;

	Range foleysLogRange(int, int) noexcept;

	// min, max, numSteps ]1, N]
	Range quad(float, float, int) noexcept;

	Range quad(int, int, int) noexcept;

	// minDenominator, maxDenominator, withZero, flipped
	// for example { 16, .5, true }
	// starts at 0, over 1/16 and ends at 2/1
	Range beats(float, float, bool = false);
}