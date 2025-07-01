#pragma once
#include "juce_dsp/juce_dsp.h"
#include "Using.h"

namespace dsp
{
	struct FFT
	{
		static constexpr int MinOrder = 5;
		static constexpr int MaxOrder = 14;
		static constexpr int RangeOrder = MaxOrder - MinOrder + 1;
		static constexpr int MaxSize = 1 << MaxOrder;
		static constexpr int MaxSize2 = MaxSize * 2;

		FFT();

		void setOrder(int) noexcept;

		void operator()(float) noexcept;

		// bins, size
		std::function<void(float*, int)> callback;

		float getFreqRangePerBin(float) const noexcept;

		float fcToBin(float) const noexcept;

		float freqHzToBin(float, float) const noexcept;
	private:
		std::array<juce::dsp::FFT, RangeOrder> ffts;
		std::array<float, MaxSize2> fifo;
		float sizeInv;
		int order, size, idx;

		void applyFFT() noexcept;
	};
}