#pragma once
#include "FFT.h"
#include "Resonator.h"

namespace dsp
{
	class PitchDetector
	{
		struct ZeroCrossing
		{
			static constexpr auto MaxStableFreqHz = 4020.f;

			ZeroCrossing() :
				x1(0.f),
				count(0.f),
				length(0.f),
				minLength(0.f),
				positive(true)
			{}

			void setMinLength(float i) noexcept
			{
				minLength = i;
			}

			float operator()(float x0) noexcept
			{
				if (count >= minLength)
				{
					if (x0 > 0.f && !positive)
					{
						positive = true;
					}
					else if (x0 < 0.f && positive)
					{
						positive = false;
						const auto frac = x1 / (x1 - x0);
						length = count + frac;
						count -= length;
						
					}
				}
				++count;
				x1 = x0;
				return length;
			}

		private:
			float x1, count, length, minLength;
			bool positive;
		};

		static constexpr auto HighestSopranEverHz = 1100.f;
		static constexpr auto MaxFreqHz = HighestSopranEverHz; // https://www.uni-koeln.de/phil-fak/muwi/ag/aph2.html
																// ZeroCrossing::MaxStableFreqHz;
	public:
		PitchDetector() :
			fft(),
			resonator(),
			zeroCrossing(),
			sampleRate(1.f), freqHz(0.f), freqRangePerBinHz(0.f),
			order(9)
		{
			fft.callback = [&](const float* bins, int size)
			{
				auto binIdx = 0;
				auto maxMag = 0.f;
				for (auto i = 1; i < size; ++i)
				{
					const auto bin = bins[i];
					const auto mag = bin * bin;
					if (maxMag < mag)
					{
						maxMag = mag;
						binIdx = i;
					}
				}
				const auto fs = static_cast<float>(sampleRate);
				const auto nFreqHz = static_cast<float>(binIdx) * fs / static_cast<float>(size * 2);
				const auto dif = std::abs(nFreqHz - freqHz);
				if (dif < freqRangePerBinHz)
					return;
				updateFreqHz(nFreqHz);
			};
		}

		void prepare(double _sampleRate)
		{
			sampleRate = static_cast<float>(_sampleRate);
			const auto o = order;
			order = -1;
			setFFTOrder(o);
			const auto maxFreqSamples = math::freqHzToSamples(MaxFreqHz, sampleRate);
			zeroCrossing.setMinLength(maxFreqSamples);
		}

		void setFFTOrder(int _order) noexcept
		{
			if(order == _order)
				return;
			order = _order;
			fft.setOrder(order);
			freqRangePerBinHz = 2.f * fft.getFreqRangePerBin(sampleRate);
			const auto bw = freqRangePerBinHz / sampleRate;
			resonator.setBandwidth(bw);
			resonator.update();
		}

		float operator()(float x) noexcept
		{
			fft(x);
			const auto y = static_cast<float>(resonator(x));
			const auto waveLengthSamples = zeroCrossing(y);
			if (waveLengthSamples == 0.f)
				return 0.f;
			const auto nFreqHz = sampleRate / waveLengthSamples;
			if (freqHz != nFreqHz)
				updateFreqHz(nFreqHz);
			return freqHz;
		}

	private:
		FFT fft;
		Resonator2 resonator;
		ZeroCrossing zeroCrossing;
		float sampleRate, freqHz, freqRangePerBinHz;
		int order;

		void updateFreqHz(float f) noexcept
		{
			freqHz = f;
			const auto fc = math::freqHzToFc(freqHz, sampleRate);
			resonator.setCutoffFc(fc);
			resonator.update();
		}
	};
}

/*

noise bursts: maybe make edgecase handling for consonants
	highcut against sibilance
    lowcut against rumble
	smoothing on entire pitch detection (last resort)

silence detection

detects wrong harmonic: try tilt eq

avoid fft by cascading band splits

*/