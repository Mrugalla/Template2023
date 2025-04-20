#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "../arch/Math.h"
#include <array>

namespace dsp
{
	using MidiBuffer = juce::MidiBuffer;
	using MidiMessage = juce::MidiMessage;
	using SIMD = juce::FloatVectorOperations;
	using AudioBuffer = juce::AudioBuffer<double>;
	using AudioBufferF = juce::AudioBuffer<float>;
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

	static constexpr double Pi = 3.1415926535897932384626433832795;
	static constexpr double Tau = Pi * 2.;
	static constexpr double PiHalf = Pi * .5;
	static constexpr double E = 2.7182818284590452353602874713527;
	static constexpr double TauInv = 1. / Tau;

	static constexpr float PiF = 3.1415926535897932384626433832795f;
	static constexpr float TauF = PiF * 2.f;
	static constexpr double SixtyInv = 1. / 60.;

	static constexpr int NumMIDIChannels = 16;
	static constexpr int NumMPEChannels = NumMIDIChannels - 1;
	static constexpr double PitchbendRange = 16383.;
	static constexpr double PitchbendCenter = PitchbendRange * .5;

	static constexpr int NumChannels = PPDHasSidechain ? 4 : 2;
    static constexpr int BlockSize = 16;

	inline double loadFromMemory(AudioBufferF& buffer, const char* data, int size)
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