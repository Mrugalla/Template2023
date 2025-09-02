#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "../../arch/Math.h"
#include <array>
#include <complex>

namespace dsp
{
	using ComplexF = std::complex<float>;
	using ComplexD = std::complex<double>;
	using MidiBuffer = juce::MidiBuffer;
	using MidiMessage = juce::MidiMessage;
	using SIMD = juce::FloatVectorOperations;
	using AudioBuffer = juce::AudioBuffer<float>;
	using Uint8 = uint8_t;
	using Int64 = juce::int64;
	using PlayHead = juce::AudioPlayHead;
	using PosInfo = PlayHead::CurrentPositionInfo;
	using MemoryInputStream = juce::MemoryInputStream;
	using WavAudioFormat = juce::WavAudioFormat;
	using AudioFormatReader = juce::AudioFormatReader;
	using FileOutputStream = juce::FileOutputStream;
	using PNGImageFormat = juce::PNGImageFormat;
	using File = juce::File;
	using Image = juce::Image;
	using Graphics = juce::Graphics;
	using String = juce::String;
	using BoundsF = juce::Rectangle<float>;
	using Random = juce::Random;
	using PointF = juce::Point<float>;

	static constexpr float Pi = 3.1415926535897932384626433832795f;
	static constexpr float Tau = Pi * 2.f;
	static constexpr float PiHalf = Pi * .5f;
	static constexpr float E = 2.7182818284590452353602874713527f;
	static constexpr float TauInv = 1.f / Tau;
	static constexpr float SixtyInv = 1.f / 60.f;
	static constexpr double PiD = 3.1415926535897932384626433832795;
	static constexpr double TauD = PiD * 2.;

	static constexpr int NumMIDIChannels = 16;
	static constexpr int NumMPEChannels = NumMIDIChannels - 1;
	static constexpr float PitchbendRange = 16383.f;
	static constexpr float PitchbendCenter = PitchbendRange * .5f;

	static constexpr int NumChannels = 2;
    static constexpr int BlockSize = 32;

	static constexpr auto BlockSizeF = static_cast<float>(BlockSize);
	static constexpr auto BlockSizeD = static_cast<double>(BlockSize);

	// returns the sampleRate of the wav file
	inline double loadFromMemory(AudioBuffer& buffer, const char* data, int size)
	{
		double sampleRate = 0.;
		auto stream = new MemoryInputStream(data, size, false);
		WavAudioFormat wav;
		const auto reader = wav.createReaderFor(stream, false);
		if (reader != nullptr)
		{
			sampleRate = reader->sampleRate;
			const auto numChannels = static_cast<int>(reader->numChannels);
			const auto numSamples = static_cast<int>(reader->lengthInSamples);
			buffer.setSize(numChannels, numSamples, false, false, false);
			reader->read(&buffer, 0, numSamples, 0, true, true);
		}
		delete reader;
		return sampleRate;
	}
}