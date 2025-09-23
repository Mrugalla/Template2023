#include "Gain.h"
#include "../../arch/Math.h"

namespace dsp
{
	Gain::Gain(float defaultVal) :
		gainPRM(defaultVal),
		gainInfo(nullptr, 0.f, false)
	{}

	void Gain::prepare(float sampleRate, float smoothLengthMs) noexcept
	{
		gainPRM.prepare(sampleRate, smoothLengthMs);
	}

	void Gain::setGain(float g) noexcept
	{
		gainPRM.value = g;
	}

	void Gain::operator()(ProcessorBufferView& view) noexcept
	{
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.getNumSamples();
		gainInfo = gainPRM(numSamples);
		if (gainInfo.smoothing)
		{
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(view.getSamplesMain(ch), gainInfo.buf, numSamples);
			return;
		}
		if (gainInfo.val == 0.f)
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::clear(view.getSamplesMain(ch), numSamples);
		else if (gainInfo.val != 1.f)
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(view.getSamplesMain(ch), gainInfo.val, numSamples);
	}

	void Gain::operator()(BufferView2 buffer,
		int numChannels, int numSamples) noexcept
	{
		gainInfo = gainPRM(numSamples);
		if (gainInfo.smoothing)
		{
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(buffer[ch], gainInfo.buf, numSamples);
			return;
		}
		if (gainInfo.val == 0.f)
		{
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::clear(buffer[ch], numSamples);
		}
		else if (gainInfo.val != 1.f)
		{
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(buffer[ch], gainInfo.val, numSamples);
		}
	}

	void Gain::applyInverse(ProcessorBufferView& view) noexcept
	{
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.getNumSamples();
		if (gainInfo.smoothing)
			for (auto ch = 0; ch < numChannels; ++ch)
			{
				auto smpls = view.getSamplesMain(ch);
				for (auto s = 0; s < numSamples; ++s)
					smpls[s] /= gainInfo.buf[s];
			}
		else
		{
			gainInfo.val = 1.f / gainInfo.val;
			for (auto ch = 0; ch < numChannels; ++ch)
			{
				auto smpls = view.getSamplesMain(ch);
				SIMD::multiply(smpls, gainInfo.val, numSamples);
			}
		}
	}

	void Gain::applyInverse(float* smpls, int numSamples) noexcept
	{
		if (gainInfo.smoothing)
		{
			for (auto s = 0; s < numSamples; ++s)
				smpls[s] /= gainInfo.buf[s];
			return;
		}
		gainInfo.val = 1.f / gainInfo.val;
		SIMD::multiply(smpls, gainInfo.val, numSamples);
	}
}