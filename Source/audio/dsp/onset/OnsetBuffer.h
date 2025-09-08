#pragma once
#if PPDHasOnsetDetector
#include "../ProcessorBufferView.h"

namespace dsp
{
	struct OnsetBuffer
	{
		OnsetBuffer() :
			buffer()
		{ }

		void copyFrom(OnsetBuffer& other, int numSamples) noexcept
		{
			SIMD::copy(buffer.data(), other.buffer.data(), numSamples);
		}

		float* getSamples() noexcept
		{
			return buffer.data();
		}

		float getMaxMag(int numSamples) const noexcept
		{
			auto max = 0.f;
			for (auto s = 0; s < numSamples; ++s)
				if (max < buffer[s])
					max = buffer[s];
			return max;
		}

		void clear(int numSamples) noexcept
		{
			SIMD::fill(buffer.data(), 0.f, numSamples);
		}

		float& operator[](int i) noexcept
		{
			return buffer[i];
		}

		const float& operator[](int i) const noexcept
		{
			return buffer[i];
		}

		void rectify(int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
				buffer[s] = std::abs(buffer[s]);
		}

		void copyFromMid(ProcessorBufferView& view) noexcept
		{
			SIMD::copy(buffer.data(), view.getSamplesMain(0), view.numSamples);
			if (view.getNumChannelsMain() != 2)
				return;
			SIMD::add(buffer.data(), view.getSamplesMain(1), view.numSamples);
			SIMD::multiply(buffer.data(), .5f, view.numSamples);
		}

		void copyTo(ProcessorBufferView& view) noexcept
		{
			for (auto ch = 0; ch < view.getNumChannelsMain(); ++ch)
				SIMD::copy(view.getSamplesMain(ch), buffer.data(), view.numSamples);
		}
	protected:
		std::array<float, BlockSize> buffer;
	};
}
#endif