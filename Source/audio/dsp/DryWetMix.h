#pragma once
#include "PRM.h"
#include "WHead.h"

namespace dsp
{	
	struct LatencyCompensation
	{
		LatencyCompensation() :
			ring(),
			wHead(),
			latency(0)
		{}

		void prepare(int _latency)
		{
			latency = _latency;
			if (latency != 0)
			{
				ring.setSize(2, latency, false, true, false);
				wHead.prepare(latency);
			}
			else
			{
				ring.setSize(0, 0);
				wHead.prepare(0);
			}
		}

		void operator()(double* const* dry, const double* const* inputSamples,
			int numChannels, int numSamples) noexcept
		{
			if (latency != 0)
			{
				wHead(numSamples);

				for (auto ch = 0; ch < numChannels; ++ch)
				{
					const auto smpls = inputSamples[ch];

					auto rng = ring.getWritePointer(ch);
					auto dr = dry[ch];

					for (auto s = 0; s < numSamples; ++s)
					{
						const auto w = wHead[s];
						const auto r = (w + 1) % latency;

						rng[w] = smpls[s];
						dr[s] = rng[r];
					}
				}
			}
			else
				for (auto ch = 0; ch < numChannels; ++ch)
					SIMD::copy(dry[ch], inputSamples[ch], numSamples);
		}

		void operator()(double* const* samples, int numChannels, int numSamples) noexcept
		{
			if (latency != 0)
			{
				wHead(numSamples);

				for (auto ch = 0; ch < numChannels; ++ch)
				{
					const auto smpls = samples[ch];
					auto rng = ring.getWritePointer(ch);

					for (auto s = 0; s < numSamples; ++s)
					{
						const auto w = wHead[s];
						const auto r = (w + 1) % latency;

						rng[w] = smpls[s];
						smpls[s] = rng[r];
					}
				}
			}
		}

	protected:
		AudioBuffer ring;
		WHead wHead;
	public:
		int latency;
	};
	
	struct DryWetMix
	{
		DryWetMix() :
#if PPDHasGainIn
			gainInParam(1.),
			gainInBuf(nullptr)
#endif
#if PPDHasGainWet
			,gainWetParam(1.)
#endif
			,gainOutParam(1.)
#if PPDHasMix
			,mixParam(1.)
#endif
			,latencyCompensation()
		{}

		void prepare(double sampleRate, int latency)
		{
			latencyCompensation.prepare(latency);
#if PPDHasGainIn
			gainInParam.prepare(sampleRate, 10.);
#endif
#if PPDHasGainWet
			gainWetParam.prepare(sampleRate, 10.);
#endif
			gainOutParam.prepare(sampleRate, 10.);
#if PPDHasMix
			mixParam.prepare(sampleRate, 10.);
#endif
		}
		
		void processInput(double* const* samples
#if PPDHasGainIn
			,double gainInAmp
#endif
			,int numChannels, int numSamples
#if PPDHasPolarity
			,bool polarity
#endif
		) noexcept
		{
#if PPDHasGainIn
			auto gainInInfo = gainInParam(gainInAmp, numSamples);
			if (!gainInInfo.smoothing)
				SIMD::fill(gainInInfo.buf, gainInInfo.val, numSamples);
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(samples[ch], gainInInfo.buf, numSamples);
#if PPDHasUnityGain
			gainInBuf = gainInInfo.buf;
#endif
#endif
#if PPDHasPolarity
			if (polarity)
				for (auto ch = 0; ch < numChannels; ++ch)
					SIMD::multiply(samples[ch], -1., numSamples);
#endif
			latencyCompensation(samples, numChannels, numSamples);
		}
		
		void processOutput(double* const* samples
			,double gainWet
#if PPDHasMix
			,double mix
#endif
			,int numChannels, int numSamples
#if PPDHasGainIn && PPDHasUnityGain
			,bool unityGain
#endif
		)
		{
#if PPDHasGainIn && PPDHasUnityGain
			if (unityGain)
			{
				for (auto s = 0; s < numSamples; ++s)
					gainInBuf[s] = 1. / gainInBuf[s];
				for (auto ch = 0; ch < numChannels; ++ch)
					SIMD::multiply(samples[ch], gainInBuf, numSamples);
			}
#endif
		}

#if PPDHasGainIn
		PRMD gainInParam;
		double* gainInBuf;
#endif
#if PPDHasGainWet
		PRMD gainWetParam;
#endif
		PRMD gainOutParam;
#if PPDHasMix
		PRMD mixParam;
#endif
		LatencyCompensation latencyCompensation;
	};
}