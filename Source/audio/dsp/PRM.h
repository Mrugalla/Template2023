#pragma once
#include "Smooth.h"

namespace dsp
{
	template<typename Float>
	struct PRMInfo
	{
		/* buf, val, smoothing */
		PRMInfo(Float*, Float, bool);

		/* idx */
		Float operator[](int) const noexcept;

		void copyToBuffer(int) noexcept;

		Float* buf;
		Float val;
		bool smoothing;
	};

	using PRMInfoF = PRMInfo<float>;
	using PRMInfoD = PRMInfo<double>;
	
	template<typename Float>
	struct PRM
	{
		/* startVal */
		PRM(Float = static_cast<Float>(0));
		
		/* sampleRate, smoothLenMs */
		void prepare(Float sampleRate, Float smoothLenMs) noexcept;

		/* value, numSamples */
		PRMInfo<Float> operator()(Float, int) noexcept;

		/* numSamples */
		PRMInfo<Float> operator()(int) noexcept;

		/* idx */
		Float operator[](int) const noexcept;

	protected:
		smooth::Smooth<Float> smooth;
		std::array<Float, BlockSize> buf;
		Float value;
	};

	using PRMF = PRM<float>;
	using PRMD = PRM<double>;
}