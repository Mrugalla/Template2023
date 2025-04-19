#pragma once
#include "Smooth.h"

namespace dsp
{
	template<typename Float>
	struct PRMInfo
	{
		// buf, val, smoothing
		PRMInfo(Float* = nullptr, Float = 0., bool = false);

		// idx
		Float operator[](int) const noexcept;

		// numSamples
		void copyToBuffer(int) noexcept;

		// startIdx, endIdx
		void copyToBuffer(int, int) noexcept;

		operator Float() const noexcept;

		Float* buf;
		Float val;
		bool smoothing;
	};

	using PRMInfoF = PRMInfo<float>;
	using PRMInfoD = PRMInfo<double>;
	
	template<typename Float>
	struct PRM
	{
		// startVal
		PRM(Float = static_cast<Float>(0));
		
		// sampleRate, smoothLenMs
		void prepare(Float sampleRate, Float smoothLenMs) noexcept;

		// value, numSamples
		PRMInfo<Float> operator()(Float, int) noexcept;

		// value, startIdx, endIdx
		PRMInfo<Float> operator()(Float, int, int) noexcept;

		// startIdx, endIdx
		PRMInfo<Float> operator()(int, int) noexcept;

		// numSamples
		PRMInfo<Float> operator()(int) noexcept;

		// idx
		Float operator[](int) const noexcept;

		std::array<Float, BlockSize> buf;
		smooth::Smooth<Float> smooth;
		Float value;
		bool smoothing;
	};

	using PRMF = PRM<float>;
	using PRMD = PRM<double>;

	template<typename Float>
	struct PRMBlock
	{
		// startVal
		PRMBlock(Float = static_cast<Float>(0));

		void reset() noexcept;

		// val
		void reset(Float) noexcept;

		// sampleRate, smoothLenMs
		void prepare(Float, Float) noexcept;

		// value
		PRMInfo<Float> operator()(Float) noexcept;

		operator Float() const noexcept;

		Float startVal;
		smooth::Lowpass<Float, false> lp;
		PRMInfo<Float> info;
	};

	using PRMBlockF = PRMBlock<float>;
	using PRMBlockD = PRMBlock<double>;

	template<typename Float>
	struct PRMBlockStereo
	{
		// startVal
		PRMBlockStereo(Float = static_cast<Float>(0));

		// sampleRate, smoothLenMs
		void prepare(Float, Float) noexcept;

		// value, ch
		PRMInfo<Float> operator()(Float, int) noexcept;

		// ch
		PRMInfo<Float> operator[](int) const noexcept;

	protected:
		std::array<PRMBlock<Float>, 2> prms;
	};

	using PRMBlockStereoF = PRMBlockStereo<float>;
	using PRMBlockStereoD = PRMBlockStereo<double>;
}