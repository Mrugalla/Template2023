#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

namespace dsp
{
	static constexpr double Pi = 3.1415926535897932384626433832795;
	static constexpr double Tau = Pi * 2.;
	
    using MidiBuffer = juce::MidiBuffer;
	using SIMD = juce::FloatVectorOperations;
	using AudioBuffer = juce::AudioBuffer<double>;
	
	enum class OversamplingOrder { x1, x2, x4, NumOrders };
	inline constexpr int getOversamplingFactor(OversamplingOrder order) noexcept
	{
		switch (order)
		{
			case OversamplingOrder::x1: return 1;
			case OversamplingOrder::x2: return 2;
			case OversamplingOrder::x4: return 4;
			default: return 1;
		}
	}

	static constexpr int NumChannels = PPDHasSidechain ? 4 : 2;
    static constexpr int BlockSize = 32;
	static constexpr int BlockSize2x = BlockSize * getOversamplingFactor(OversamplingOrder::x2);
	static constexpr int BlockSize4x = BlockSize * getOversamplingFactor(OversamplingOrder::x4);
}