#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

namespace math
{
	using String = juce::String;
	using Point = juce::Point<int>;
	using PointF = juce::Point<float>;
	
	inline Point getAspectRatio(const String& str)
	{
		const auto colon = str.indexOfChar(':');
		if (colon <= 0)
			return { 1, 1 };
		
		const auto w = str.substring(0, colon).getIntValue();
		const auto h = str.substring(colon + 1).getIntValue();
		return { w, h };
	}
}