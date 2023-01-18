#pragma once
#include <array>
#include <atomic>

namespace arch
{
	struct XenManager
	{
		XenManager();

		/* tmprVal, noteVal */
		void setTemperament(float, int) noexcept;

		/* xen, masterTune, baseNote */
		void operator()(float, float, float) noexcept;

		template<typename Float>
		Float noteToFreqHz(Float) const noexcept;

		/* note, lowestFreq, highestFreq */
		template<typename Float>
		Float noteToFreqHzWithWrap(Float, Float = static_cast<Float>(0), Float = static_cast<Float>(22000)) const noexcept;

		template<typename Float>
		Float freqHzToNote(Float) noexcept;

		float getXen() const noexcept;

	protected:
		float xen, masterTune, baseNote;
		std::array<std::atomic<float>, PPDMaxXen + 1> temperaments;
	};
}