#pragma once
#include "../Using.h"

namespace dsp
{
	template<size_t NumBands>
	struct ParallelProcessor
	{
		static constexpr int MaxBand = NumBands - 1;
		static constexpr int NumChannels = 2 * MaxBand;

		ParallelProcessor();

		// samples, numChannels, numSamples
		void split(float* const*, int, int) noexcept;

		// samples, numChannels, numSamples
		void join(float* const*, int, int) noexcept;

		// samples, numChannels, numSamples
		void joinReplace(float* const*, int, int) noexcept;

		// gain, bandIdx, numChannels, numSamples
		void applyGain(float, int, int, int) noexcept;

		// gain, bandIdx, numChannels, numSamples
		void applyGain(float*, int, int, int) noexcept;

		BufferView2 getBand(int) noexcept;

		BufferView2 operator[](int) noexcept;

		// only for parallel processors with exactly 2 bands
		//samples, mix, numChannels, numSamples
		void joinMix(float* const*, float*, int, int) noexcept;

		// only for parallel processors with exactly 2 bands
		//samples, mix, numChannels, numSamples
		void joinMix(float* const*, float, int, int) noexcept;

		// only for parallel processors with exactly 2 bands
		//samples, gain, numChannels, numSamples
		void joinDelta(float* const*, float*, int, int) noexcept;

		//only for parallel processors with exactly 2 bands
		//samples, gain, numChannels, numSamples
		void joinDelta(float* const*, float, int, int) noexcept;

		// bandIdx
		bool isSleepy(int) const noexcept;

		// sleepyState, bandIdx
		void setSleepy(bool, int) noexcept;

	private:
		std::array<std::array<float, BlockSize>, NumChannels> bands;
		std::array<bool, NumBands> sleepy;
	};

	using PP2Band = ParallelProcessor<2>;
	using PP3Band = ParallelProcessor<3>;
	using PP4Band = ParallelProcessor<4>;
	using PP5Band = ParallelProcessor<5>;
	using PPMPEBand = ParallelProcessor<NumMPEChannels>;
	using PPMIDIBand = ParallelProcessor<NumMIDIChannels>;
}