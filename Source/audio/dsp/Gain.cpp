#include "Gain.h"
#include "../../arch/Math.h"

namespace dsp
{
	template<double SmoothLengthMs>
	Gain<SmoothLengthMs>::Gain(double defaultValueDb) :
		gainPRM(math::decibelToAmp(defaultValueDb)),
		gainInfo(nullptr, 0., false)
	{}

	template<double SmoothLengthMs>
	void Gain<SmoothLengthMs>::prepare(double sampleRate) noexcept
	{
		gainPRM.prepare(sampleRate, SmoothLengthMs);
	}

	template<double SmoothLengthMs>
	void Gain<SmoothLengthMs>::operator()(double* const* samples, double gainDb,
		int numChannels, int numSamples) noexcept
	{
		const auto gainAmp = math::decibelToAmp(gainDb);
		gainInfo = gainPRM(gainAmp, numSamples);

		if (gainInfo.smoothing)
			for (auto ch = 0; ch < numChannels; ++ch)
			{
				auto smpls = samples[ch];
				SIMD::multiply(smpls, gainInfo.buf, numSamples);
			}
		else
			for (auto ch = 0; ch < numChannels; ++ch)
			{
				auto smpls = samples[ch];
				SIMD::multiply(smpls, gainInfo.val, numSamples);
			}
	}

	template<double SmoothLengthMs>
	void Gain<SmoothLengthMs>::applyInverse(double* const* samples, int numChannels, int numSamples) noexcept
	{
		if (gainInfo.smoothing)
			for (auto ch = 0; ch < numChannels; ++ch)
			{
				auto smpls = samples[ch];
				for (auto s = 0; s < numSamples; ++s)
					smpls[s] /= gainInfo.buf[s];
			}
		else
		{
			gainInfo.val = 1. / gainInfo.val;

			for (auto ch = 0; ch < numChannels; ++ch)
			{
				auto smpls = samples[ch];
				SIMD::multiply(smpls, gainInfo.val, numSamples);
			}
		}
	}

	template<double SmoothLengthMs>
	void Gain<SmoothLengthMs>::applyInverse(double* smpls, int numSamples) noexcept
	{
		if (gainInfo.smoothing)
			for (auto s = 0; s < numSamples; ++s)
				smpls[s] /= gainInfo.buf[s];
		else
		{
			gainInfo.val = 1. / gainInfo.val;
			SIMD::multiply(smpls, gainInfo.val, numSamples);
		}
	}

	template struct Gain<1.>;
	template struct Gain<2.>;
	template struct Gain<3.>;
	template struct Gain<5.>;
	template struct Gain<8.>;
	template struct Gain<13.>;
	template struct Gain<21.>;
	template struct Gain<34.>;
	template struct Gain<55.>;
	template struct Gain<89.>;
	template struct Gain<144.>;
}