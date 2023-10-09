#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

namespace dsp
{
	static constexpr double Pi = 3.1415926535897932384626433832795;
	static constexpr double Tau = Pi * 2.;
	static constexpr double E = 2.7182818284590452353602874713527;
	static constexpr double TauInv = 1. / Tau;

	static constexpr float PiF = 3.1415926535897932384626433832795f;
	static constexpr float TauF = PiF * 2.f;

	using MidiBuffer = juce::MidiBuffer;
	using MidiMessage = juce::MidiMessage;
	using SIMD = juce::FloatVectorOperations;
	using AudioBuffer = juce::AudioBuffer<double>;
	using Uint8 = uint8_t;
	
	static constexpr int NumMIDIChannels = 16;
	static constexpr int NumMPEChannels = NumMIDIChannels - 1;
	static constexpr double PitchbendRange = 16383.;
	static constexpr double PitchbendRangeHalf = PitchbendRange * .5;
	
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

	template<typename Float>
	inline void copy(Float* dest, Float* src, int numSamples) noexcept
	{
		SIMD::copy(dest, src, numSamples);
	}

	template<typename Float>
	inline void add(Float* dest, Float* src, int numSamples) noexcept
	{
		SIMD::add(dest, src, numSamples);
	}

	template<typename Float>
	inline void copy(Float** dest, Float** src,
		int numChannels, int numSamples) noexcept
	{
		for(auto ch = 0; ch < numChannels; ++ch)
			copy(dest[ch], src[ch], numSamples);
	}

	template<typename Float>
	inline void add(Float** dest, Float** src,
		int numChannels, int numSamples) noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
			add(dest[ch], src[ch], numSamples);
	}
}