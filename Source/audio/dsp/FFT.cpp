#include "FFT.h"

namespace dsp
{
	FFT::FFT() :
		ffts
	{
		juce::dsp::FFT(5), juce::dsp::FFT(6), juce::dsp::FFT(7),
		juce::dsp::FFT(8), juce::dsp::FFT(9), juce::dsp::FFT(10),
		juce::dsp::FFT(11), juce::dsp::FFT(12), juce::dsp::FFT(13),
		juce::dsp::FFT(14)
	},
		fifo(),
		sizeInv(0.f),
		order(0), size(0), idx(0)
	{
		setOrder(9);
	}

	void FFT::setOrder(int o) noexcept
	{
		if (order == o)
			return;
		if (o < MinOrder || o > MaxOrder)
			return;
		order = o;
		size = 1 << order;
		idx = 0;
		sizeInv = 1.f / static_cast<float>(size);
	}

	void FFT::operator()(float x) noexcept
	{
		const auto w = std::sin(Pi * static_cast<float>(idx) * sizeInv);
		const auto window = w * w;
		fifo[idx] = x * window;
		++idx;
		if (idx >= size)
			applyFFT();
	}

	float FFT::getFreqRangePerBin(float sampleRate) const noexcept
	{
		return sampleRate * sizeInv;
	}

	float FFT::fcToBin(float fc) const noexcept
	{
		return fc * size;
	}

	float FFT::freqHzToBin(float freqHz, float sampleRate) const noexcept
	{
		return fcToBin(math::freqHzToFc(freqHz, sampleRate));
	}

	void FFT::applyFFT() noexcept
	{
		idx = 0;
		ffts[order - MinOrder].performRealOnlyForwardTransform(fifo.data(), true);
		callback(fifo.data(), size);
	}
}