#pragma once
#include "ParallelProcessor.h"
#include "Gain.h"
#include "WHead.h"

namespace dsp
{	
	struct LatencyCompensation
	{
		using DryBuffers = std::array<std::array<float, BlockSize>, 2>;

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

		void operator()(DryBuffers& dryBuffers, const float* const* inputSamples,
			int numChannels, int numSamples) noexcept
		{
			if (latency != 0)
			{
				wHead(numSamples);

				for (auto ch = 0; ch < numChannels; ++ch)
				{
					const auto smpls = inputSamples[ch];

					auto rng = ring.getWritePointer(ch);
					auto dry = dryBuffers[ch].data();

					for (auto s = 0; s < numSamples; ++s)
					{
						const auto w = wHead[s];
						const auto r = (w + 1) % latency;

						rng[w] = smpls[s];
						dry[s] = rng[r];
					}
				}
			}
			else
				for (auto ch = 0; ch < numChannels; ++ch)
					SIMD::copy(dryBuffers[ch].data(), inputSamples[ch], numSamples);
		}

		void operator()(float* const* samples, int numChannels, int numSamples) noexcept
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

	private:
		AudioBuffer ring;
		WHead wHead;
	public:
		int latency;
	};

	using Gain13 = Gain<13.f>;

	struct MixProcessorNonlinear
	{
		MixProcessorNonlinear() :
			gainIn(0.f)
		{}

		void prepare(float sampleRate)
		{
			gainIn.prepare(sampleRate);
		}

		void split(float* const* samples, float gainInDb,
			int numChannels, int numSamples) noexcept
		{
			gainIn(samples, gainInDb, numChannels, numSamples);
		}

		void join(float* const* samples,
			int numChannels, int numSamples,
			bool unityGain) noexcept
		{
			if(unityGain)
				gainIn.applyInverse(samples, numChannels, numSamples);
		}

		Gain13 gainIn;
	};

	struct MixProcessorDryWet
	{
		MixProcessorDryWet() :
			parallelProcessor(),
			gainDry(0.f),
			gainWetIn(0.f),
			gainWetOut(0.f)
		{}

		void prepare(float sampleRate)
		{
			gainDry.prepare(sampleRate);
			gainWetIn.prepare(sampleRate);
			gainWetOut.prepare(sampleRate);
		}

		// samples, gainDryDb, numChannels, numSamples
		void splitLinear(float* const* samples, float gainDryDb,
			int numChannels, int numSamples) noexcept
		{
			parallelProcessor.split(samples, numChannels, numSamples);
			const auto band = parallelProcessor.getBand(0);
			gainDry(band, gainDryDb, numChannels, numSamples);
		}

		// samples, gainDryDb, gainWetInDb, numChannels, numSamples
		void splitNonlinear(float* const* samples, float gainDryDb, float gainWetInDb,
			int numChannels, int numSamples) noexcept
		{
			splitLinear(samples, gainDryDb, numChannels, numSamples);
			gainWetIn(samples, gainWetInDb, numChannels, numSamples);
		}

		// samples, gainWetDb, numChannels, numSamples 
		void joinLinear(float* const* samples, float gainWetOutDb,
			int numChannels, int numSamples) noexcept
		{
			gainWetOut(samples, gainWetOutDb, numChannels, numSamples);
			parallelProcessor.join(samples, numChannels, numSamples);
		}

		// samples, gainWetDb, numChannels, numSamples, unityGain
		void joinNonlinear(float* const* samples, float gainWetOutDb,
			int numChannels, int numSamples, bool unityGain) noexcept
		{
			if(unityGain)
				gainWetIn.applyInverse(samples, numChannels, numSamples);
			joinLinear(samples, gainWetOutDb, numChannels, numSamples);
		}

	private:
		PP2Band parallelProcessor;
		Gain13 gainDry, gainWetIn, gainWetOut;
	};

	struct MixProcessorWetMix
	{
		MixProcessorWetMix() :
			parallelProcessor(),
			gainWetIn(0.f),
			gainWetOut(0.f),
			mixPRM(1.f)
		{}

		void prepare(float sampleRate)
		{
			gainWetIn.prepare(sampleRate);
			gainWetOut.prepare(sampleRate);
			mixPRM.prepare(sampleRate, 13.f);
		}

		// samples, numChannels, numSamples
		void splitLinear(float* const* samples,
			int numChannels, int numSamples) noexcept
		{
			parallelProcessor.split(samples, numChannels, numSamples);
		}

		// samples, gainWetInDb, numChannels, numSamples
		void splitNonlinear(float* const* samples, float gainWetInDb,
			int numChannels, int numSamples) noexcept
		{
			splitLinear(samples, numChannels, numSamples);
			gainWetIn(samples, gainWetInDb, numChannels, numSamples);
		}

		// samples, mix, gainWetOutDb, numChannels, numSamples, delta 
		void joinLinear(float* const* samples, float mix, float gainWetOutDb,
			int numChannels, int numSamples, bool delta) noexcept
		{
			gainWetOut(samples, gainWetOutDb, numChannels, numSamples);
			if (delta)
				joinDelta(samples, mix, numChannels, numSamples);
			else
				joinMix(samples, mix, numChannels, numSamples);
		}

		// samples, mix, gainWetOutDb, numChannels, numSamples, unityGain, delta 
		void joinNonlinear(float* const* samples, float mix, float gainWetOutDb,
			int numChannels, int numSamples, bool unityGain, bool delta) noexcept
		{
			if (unityGain)
				gainWetIn.applyInverse(samples, numChannels, numSamples);
			joinLinear(samples, mix, gainWetOutDb, numChannels, numSamples, delta);
		};
	
	private:
		PP2Band parallelProcessor;
		Gain13 gainWetIn, gainWetOut;
		PRM mixPRM;

		// samples, mix, numChannels, numSamples
		void joinMix(float* const* samples, float mix, int numChannels, int numSamples) noexcept
		{
			const auto mixInfo = mixPRM(mix, numSamples);
			if (mixInfo.smoothing)
				parallelProcessor.joinMix(samples, mixInfo.buf, numChannels, numSamples);
			else
				parallelProcessor.joinMix(samples, mixInfo.val, numChannels, numSamples);
		}

		// samples, mix, numChannels, numSamples
		void joinDelta(float* const* samples, float mix, int numChannels, int numSamples) noexcept
		{
			const auto mixInfo = mixPRM(mix, numSamples);
			if (mixInfo.smoothing)
				parallelProcessor.joinDelta(samples, mixInfo.buf, numChannels, numSamples);
			else
				parallelProcessor.joinDelta(samples, mixInfo.val, numChannels, numSamples);
		}
	};

	struct MixProcessor
	{
		MixProcessor() :
			mixProcessor(),
			gainOut(0.f)
		{}

		void prepare(float sampleRate)
		{
			mixProcessor.prepare(sampleRate);
			gainOut.prepare(sampleRate);
		}

#if PPDIO == PPDIOOut
	#if PPDIsNonlinear
		// samples, gainInDb, numChannels, numSamples
		void split(float* const* samples, float gainInDb,
			int numChannels, int numSamples) noexcept
		{
			mixProcessor.split(samples, gainInDb, numChannels, numSamples);
		}
		// samples, gainOutDb, numChannels, numSamples, unityGain
		void join(float* const* samples, float gainOutDb,
			int numChannels, int numSamples, bool unityGain) noexcept
		{
			mixProcessor.join(samples, numChannels, numSamples, unityGain);
			gainOut(samples, gainOutDb, numChannels, numSamples);
		}
	#else
		void join(float* const* samples, float gainOutDb,
			int numChannels, int numSamples) noexcept
		{
			gainOut(samples, gainOutDb, numChannels, numSamples);
		}
	#endif
#elif PPDIO == PPDIODryWet
	#if PPDIsNonlinear
		// samples, gainDryDb, gainWetInDb, numChannels, numSamples
		void split(float* const* samples, float gainDryDb, float gainWetInDb,
			int numChannels, int numSamples) noexcept
		{
			mixProcessor.splitNonlinear(samples, gainDryDb, gainWetInDb, numChannels, numSamples);
		}

		// samples, gainWetOutDb, gainOutDb, numChannels, numSamples, unityGain
		void join(float* const* samples, float gainWetOutDb, float gainOutDb,
			int numChannels, int numSamples, bool unityGain) noexcept
		{
			mixProcessor.joinNonlinear(samples, gainWetOutDb, numChannels, numSamples, unityGain);
			gainOut(samples, gainOutDb, numChannels, numSamples);
		}
	#else
		// samples, gainDryDb, numChannels, numSamples
		void split(float* const* samples, float gainDryDb,
			int numChannels, int numSamples) noexcept
		{
			mixProcessor.splitLinear(samples, gainDryDb, numChannels, numSamples);
		}

		// samples, gainWetOutDb, gainOutDb, numChannels, numSamples
		void join(float* const* samples, float gainWetOutDb, float gainOutDb,
			int numChannels, int numSamples) noexcept
		{
			mixProcessor.joinLinear(samples, gainWetOutDb, numChannels, numSamples);
			gainOut(samples, gainOutDb, numChannels, numSamples);
		}
	#endif
#else
	#if PPDIsNonlinear
		// samples, gainWetInDb, numChannels, numSamples
		void split(float* const* samples, float gainWetInDb,
			int numChannels, int numSamples) noexcept
		{
			mixProcessor.splitNonlinear(samples, gainWetInDb, numChannels, numSamples);
		}

		// samples, mix, gainWetDb, gainOutDb, numChannels, numSamples, unityGain, delta
		void join(float* const* samples, float mix, float gainWetDb, float gainOutDb,
			int numChannels, int numSamples, bool unityGain, bool delta) noexcept
		{
			mixProcessor.joinNonlinear(samples, mix, gainWetDb, numChannels, numSamples, unityGain, delta);
			gainOut(samples, gainOutDb, numChannels, numSamples);
		}
	#else
		// samples, numChannels, numSamples
		void split(float* const* samples,
			int numChannels, int numSamples) noexcept
		{
			mixProcessor.splitLinear(samples, numChannels, numSamples);
		}

		// samples, mix, gainWetDb, gainOutDb, numChannels, numSamples, delta
		void join(float* const* samples, float mix, float gainWetDb, float gainOutDb,
			int numChannels, int numSamples, bool delta) noexcept
		{
			mixProcessor.joinLinear(samples, mix, gainWetDb, numChannels, numSamples, delta);
			gainOut(samples, gainOutDb, numChannels, numSamples);
		}
	#endif
#endif

	private:
#if PPDIO == PPDIOOut
		MixProcessorNonlinear mixProcessor;
#elif PPDIO == PPDIODryWet
		MixProcessorDryWet mixProcessor;
#else
		MixProcessorWetMix mixProcessor;
#endif
		Gain13 gainOut;
	};
}