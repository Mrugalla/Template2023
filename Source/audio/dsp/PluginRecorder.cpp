#include "PluginRecorder.h"

namespace dsp
{
	PluginRecorder::PluginRecorder() :
		recording(),
		outBuffer(),
		writeHead(0)
	{}

	void PluginRecorder::prepare(float sampleRate)
	{
		const auto numSamples = static_cast<int>(sampleRate * 10.f);
		recording.setSize(2, numSamples, false, true, false);
		outBuffer.setSize(2, numSamples, false, true, false);
		writeHead = 0;
	}

	void PluginRecorder::operator()(const ProcessorBufferView& view) noexcept
	{
		auto recSamples = recording.getArrayOfWritePointers();
		const auto recNumSamples = recording.getNumSamples();
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.numSamples;
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			const auto smpls = view.getSamplesMain(ch);
			auto recSmpls = recSamples[ch];
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto wH = (writeHead + s) % recNumSamples;
				recSmpls[wH] = smpls[s];
			}
		}
		writeHead = (writeHead + numSamples) % recNumSamples;
	}

	const AudioBuffer& PluginRecorder::getRecording()
	{
		const auto numChannels = 2;
		const auto numSamples = recording.getNumSamples();
		auto rHead = (writeHead + 1) % numSamples;
		auto recBuf = recording.getArrayOfReadPointers();
		auto outBuf = outBuffer.getArrayOfWritePointers();
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			const auto rec = recBuf[ch];
			auto buf = outBuf[ch];
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto rH = (rHead + s) % numSamples;
				buf[s] = rec[rH];
			}
		}
		normalize(outBuf, numChannels, numSamples);
		return outBuffer;
	}

	void PluginRecorder::normalize(float* const* samples, int numChannels, int numSamples) noexcept
	{
		auto mag = 0.f;
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			const auto smpls = samples[ch];
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto absSmpl = std::abs(smpls[s]);
				mag = std::max(mag, absSmpl);
			}
		}
		if (mag == 0.f || mag == 1.f)
			return;
		const auto g = 1.f / mag;
		for (auto ch = 0; ch < numChannels; ++ch)
			SIMD::multiply(samples[ch], g, numSamples);
	}
}

