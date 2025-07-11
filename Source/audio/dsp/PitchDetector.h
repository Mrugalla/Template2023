#pragma once
#include "FFT.h"
#include "Resonator.h"
#include "Gate.h"

namespace dsp
{
	class PitchDetector
	{
		using LP = smooth::Lowpass;

		static constexpr auto MinFreqHz = 60.f;
		static constexpr auto HighestSopranEverHz = 1100.f;
		static constexpr auto MaxFreqHz = HighestSopranEverHz; // https://www.uni-koeln.de/phil-fak/muwi/ag/aph2.html
									// ZeroCrossing::MaxStableFreqHz;

		struct ZeroCrossing
		{
			static constexpr auto MaxStableFreqHz = 4020.f;

			ZeroCrossing();

			void setMinLength(float) noexcept;

			float operator()(float) noexcept;

		private:
			float x1, count, length, minLength;
			bool positive;
		};
	public:
		struct Info
		{
			float freqHz;
			bool tonal;
		};

		PitchDetector();

		void prepare(double);

		// Parameters:

		void setFFTOrder(int) noexcept;

		void setLPDecayInMs(float) noexcept;

		// Process:

		Info operator()(float) noexcept;

		Info getInfo() const noexcept;
	private:
		LP lpTonal;
		Gate gate;
		FFT fft;
		Resonator2 resonator;
		ZeroCrossing zeroCrossing;
		LP lpSmooth;
		Info info;
		float sampleRate, sampleRateInv;
		float lpDecayMs, freqRangePerBinHz;
		int order, maxFreqBin;

		Info& processSample() noexcept;

		void updateFreqHz(float) noexcept;

		// bins, size, // gainDb, cutoffHz
		void boost(float*, int, float, float) noexcept;
	};
}

/*

states: silence, tonal, sibilance

remove rumble
	pre-hp

silence detection
	gate (threshold, ratio, lookahead)

boost fundamental
	pre-bell (freq, gain, q)

sibilance
	transient detection

latency:
	avoid lookahead in silence detection with transient detection?
	avoid fft in pitch detection by cascading band splits

*/