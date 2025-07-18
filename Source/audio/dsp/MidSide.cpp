#include "MidSide.h"

namespace dsp
{
	template<typename Float>
	void midSideEncode(Float* const* samples, int numSamples) noexcept
	{
		static constexpr auto A = static_cast<Float>(.5);
		auto smplsL = samples[0];
		auto smplsR = samples[1];

		for (auto s = 0; s < numSamples; ++s)
		{
			const auto mid = (smplsL[s] + smplsR[s]) * A;
			const auto side = (smplsL[s] - smplsR[s]) * A;
			smplsL[s] = mid;
			smplsR[s] = side;
		}
	}

	template<typename Float>
	void midSideDecode(Float* const* samples, int numSamples) noexcept
	{
		auto smplsM = samples[0];
		auto smplsS = samples[1];

		for (auto s = 0; s < numSamples; ++s)
		{
			const auto left = smplsM[s] + smplsS[s];
			const auto right = smplsM[s] - smplsS[s];
			smplsM[s] = left;
			smplsS[s] = right;
		}
	}

	void midSideEncode(ProcessorBufferView& view) noexcept
	{
		static constexpr auto A = .5f;
		auto smplsL = view.getSamplesMain(0);
		auto smplsR = view.getSamplesMain(1);

		for (auto s = 0; s < view.numSamples; ++s)
		{
			const auto mid = (smplsL[s] + smplsR[s]) * A;
			const auto side = (smplsL[s] - smplsR[s]) * A;
			smplsL[s] = mid;
			smplsR[s] = side;
		}
	}

	void midSideDecode(ProcessorBufferView& view) noexcept
	{
		auto smplsM = view.getSamplesMain(0);
		auto smplsS = view.getSamplesMain(1);
		for (auto s = 0; s < view.numSamples; ++s)
		{
			const auto left = smplsM[s] + smplsS[s];
			const auto right = smplsM[s] - smplsS[s];
			smplsM[s] = left;
			smplsS[s] = right;
		}
	}

	template void midSideEncode(float* const*, int);
	template void midSideEncode(double* const*, int);
	template void midSideDecode(float* const*, int);
	template void midSideDecode(double* const*, int);
}