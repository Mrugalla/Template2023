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
	
    static constexpr int BlockSize = 32;
}