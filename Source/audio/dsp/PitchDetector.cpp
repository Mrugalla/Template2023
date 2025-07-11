#include "PitchDetector.h"

namespace dsp
{
	// ZeroCrossing

	PitchDetector::ZeroCrossing::ZeroCrossing() :
		x1(0.f),
		count(0.f),
		length(0.f),
		minLength(0.f),
		positive(true)
	{
	}

	void PitchDetector::ZeroCrossing::setMinLength(float i) noexcept
	{
		minLength = i;
	}

	float PitchDetector::ZeroCrossing::operator()(float x0) noexcept
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

	// PitchDetector
	
	PitchDetector::PitchDetector() :
		lpTonal(0.),
		gate(),
		fft(),
		resonator(),
		zeroCrossing(),
		lpSmooth(0.),
		info{ 0.f, false },
		sampleRate(1.f), sampleRateInv(1.f),
		lpDecayMs(0.f), freqRangePerBinHz(0.f),
		order(9), maxFreqBin(0)
	{
		fft.callback = [&](float* bins, int size)
		{
			boost(bins, size, 6.f, 60.f);
			auto binIdx = 0;
			auto maxMag = 0.f;
			for (auto i = 1; i < maxFreqBin; ++i)
			{
				const auto bin = bins[i];
				const auto mag = bin * bin;
				if (maxMag < mag)
				{
					maxMag = mag;
					binIdx = i;
				}
			}
			const auto nFreqHz = static_cast<float>(binIdx) * sampleRate / static_cast<float>(size * 2);
			const auto dif = std::abs(nFreqHz - info.freqHz);
			if (dif < freqRangePerBinHz)
				return;
			updateFreqHz(nFreqHz);
		};
	}

	void PitchDetector::prepare(double _sampleRate)
	{
		lpTonal.makeFromDecayInHz(MaxFreqHz, _sampleRate);
		gate.prepare(_sampleRate);
		sampleRate = static_cast<float>(_sampleRate);
		sampleRateInv = 1.f / sampleRate;
		info = { 0.f, false };
		const auto o = order;
		order = -1;
		setFFTOrder(o);
		const auto maxFreqSamples = math::freqHzToSamples(MaxFreqHz, sampleRate);
		zeroCrossing.setMinLength(maxFreqSamples);
		lpSmooth.makeFromDecayInMs(4., _sampleRate);
	}

	// Parameters

	void PitchDetector::setFFTOrder(int _order) noexcept
	{
		order = _order;
		fft.setOrder(order);
		freqRangePerBinHz = 2.f * fft.getFreqRangePerBin(sampleRate);
		const auto bw = freqRangePerBinHz * sampleRateInv;
		resonator.setBandwidth(bw);
		resonator.update();
		maxFreqBin = static_cast<int>(fft.fcToBin(MaxFreqHz * sampleRateInv));
	}

	void PitchDetector::setLPDecayInMs(float _lpDecayMs) noexcept
	{
		lpDecayMs = _lpDecayMs;
		lpSmooth.makeFromDecayInMs(lpDecayMs, sampleRate);
	}

	// Process

	PitchDetector::Info PitchDetector::operator()(float x) noexcept
	{
		const auto xTonal = lpTonal(x);
		const auto xGated = gate(static_cast<float>(xTonal));
		// silence detection and handling here or something

		fft(xGated);
		const auto y = static_cast<float>(resonator(xGated));
		const auto waveLengthSamples = zeroCrossing(y);
		if (waveLengthSamples != 0.f)
		{
			const auto nFreqHz = sampleRate / waveLengthSamples;
			if (info.freqHz == nFreqHz)
				return processSample();
			updateFreqHz(nFreqHz);
		}
		return processSample();
	}

	PitchDetector::Info PitchDetector::getInfo() const noexcept
	{
		return info;
	}

	PitchDetector::Info& PitchDetector::processSample() noexcept
	{
		info.freqHz = static_cast<float>(lpSmooth(info.freqHz));
		return info;
	}

	void PitchDetector::updateFreqHz(float f) noexcept
	{
		if (f < MinFreqHz || f > MaxFreqHz)
		{
			info.tonal = false;
			return;
		}
		info.freqHz = f;
		info.tonal = true;
		const auto fc = math::freqHzToFc(info.freqHz, sampleRate);
		resonator.setCutoffFc(fc);
		resonator.update();
	}

	void PitchDetector::boost(float* bins, int size, float gainDb, float cutoffHz) noexcept
	{
		if (gainDb == 0.f)
			return;
		const auto gain = math::dbToAmp(gainDb);
		const auto sizeF = static_cast<float>(size);
		const auto freqBin = cutoffHz / sampleRate * sizeF;
		const auto freqBinInt = static_cast<int>(freqBin);
		const auto freqBinInv = 1.f / freqBin;
		for (auto i = 1; i < freqBinInt; ++i)
		{
			const auto iF = static_cast<float>(i);
			const auto x = iF * freqBinInv;
			bins[i] += x * gain;
		}
		const auto range = sizeF - freqBin;
		const auto rangeInv = 1.f / range;
		for (auto i = freqBinInt; i < size; ++i)
		{
			const auto iF = static_cast<float>(i);
			const auto x = (iF - freqBin) * rangeInv;
			bins[i] += x * gain;
		}
	}
}