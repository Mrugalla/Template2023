#pragma once
#include "ProcessorBufferView.h"
#include "PRM.h"

namespace dsp
{	
	struct MixProcessor
	{
		static constexpr auto SmoothTimeMs = 4.f;

		MixProcessor() :
			bandDry(),
			gainWetIn(1.f),
			gainDryOut(0.f),
			gainWetOut(1.f),
			mix(1.f),
			gainOut(1.f),
			unityGain(true),
			delta(false)
		{
		}

		void prepare(float sampleRate)
		{
			gainWetIn.prepare(sampleRate, SmoothTimeMs);
			gainDryOut.prepare(sampleRate, SmoothTimeMs);
			gainWetOut.prepare(sampleRate, SmoothTimeMs);
			mix.prepare(sampleRate, SmoothTimeMs);
			gainOut.prepare(sampleRate, SmoothTimeMs);
		}

		// parameters:

		void setGainDryOut(float g) noexcept
		{
			gainDryOut.value = g;
		}

		void setGainWetIn(float g) noexcept
		{
			gainWetIn.value = g;
		}

		void setGainWetOut(float g) noexcept
		{
			gainWetOut.value = g;
		}

		void setMix(float m) noexcept
		{
			mix.value = m;
		}

		void setGainOut(float g) noexcept
		{
			gainOut.value = g;
		}

		void setUnityGain(bool u) noexcept
		{
			unityGain = u;
		}

		void setDelta(bool d) noexcept
		{
			delta = d;
		}

		// process:

		void split(ProcessorBufferView& view) noexcept
		{
#if PPDIO == PPDIODryWet || PPDIO == PPDIOWetMix
			copyDry(view);
#endif
#if PPDIsNonlinear
			processGain(view, gainWetIn);
#endif
		}

		void join(ProcessorBufferView& view) noexcept
		{
#if PPDIsNonlinear
			if (unityGain)
				inverseGainWetIn(view);
#endif
#if PPDIO == PPDIODryWet
			processGain(view, gainWetOut);
			processGainDryOut(view);
			
			joinDry(view);
#elif PPDIO == PPDIOWetMix
			processGain(view, gainWetOut);
			processMixOrDelta(view);
#endif
			processGain(view, gainOut);
		}
	private:
		std::array<std::array<float, BlockSize>, 2> bandDry;
		PRM gainWetIn, gainDryOut, gainWetOut, mix, gainOut;
		bool unityGain, delta;

		void processGain(ProcessorBufferView& view, PRM& prm) noexcept
		{
			const auto numChannels = view.getNumChannelsMain();
			const auto numSamples = view.numSamples;
			const auto prmInfo = prm(view.numSamples);
			if (prmInfo.smoothing)
			{
				for (auto ch = 0; ch < numChannels; ++ch)
					SIMD::multiply(view.getSamplesMain(ch), prmInfo.buf, numSamples);
				return;
			}
			if (prmInfo.val == 0.f)
				view.clearMain();
			else if (prmInfo.val == 1.f)
				return;
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(view.getSamplesMain(ch), prmInfo.val, numSamples);
		}

		void inverseGainWetIn(ProcessorBufferView& view) noexcept
		{
			const auto numChannels = view.getNumChannelsMain();
			const auto numSamples = view.numSamples;
			if (gainWetIn.smoothing)
			{
				for (auto ch = 0; ch < numChannels; ++ch)
				{
					auto smpls = view.getSamplesMain(ch);
					for (auto s = 0; s < numSamples; ++s)
						smpls[s] /= gainWetIn[s];
				}
				return;
			}
			// gainWetIn shall never be 0, as that is not invertable!
			oopsie(gainWetIn.value == 0.f);
			if (gainWetIn.value == 1.f)
				return;
			const auto g = 1.f / gainWetIn.value;
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(view.getSamplesMain(ch), g, numSamples);
		}

		void copyDry(ProcessorBufferView& view) noexcept
		{
			const auto numSamples = view.numSamples;
			const auto numChannels = view.getNumChannelsMain();
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::copy(bandDry[ch].data(), view.getSamplesMain(ch), numSamples);
#if PPDIO == PPDIODryWet
			const auto gainDryOutInfo = gainDryOut(numSamples);
			if (gainDryOutInfo.smoothing)
			{
				for (auto ch = 0; ch < numChannels; ++ch)
					SIMD::multiply(bandDry[ch].data(), gainDryOutInfo.buf, numSamples);
				return;
			}
			if (gainDryOutInfo.val == 1.f)
				return;
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(bandDry[ch].data(), gainDryOutInfo.val, numSamples);
#endif
		}

		void joinDry(ProcessorBufferView& view) noexcept
		{
			const auto numChannels = view.getNumChannelsMain();
			const auto numSamples = view.numSamples;
			for(auto ch = 0; ch < numChannels; ++ch)
				SIMD::add(view.getSamplesMain(ch), bandDry[ch].data(), numSamples);
		}

		void processGainDryOut(const ProcessorBufferView& view) noexcept
		{
			const auto numChannels = view.getNumChannelsMain();
			const auto numSamples = view.numSamples;
			const auto gainDryOutInfo = gainDryOut(numSamples);
			if (gainDryOutInfo.smoothing)
			{
				for(auto ch = 0; ch < numChannels; ++ch)
					SIMD::multiply(bandDry[ch].data(), gainDryOutInfo.buf, numSamples);
				return;
			}
			if (gainDryOutInfo.val == 1.f)
				return;
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(bandDry[ch].data(), gainDryOutInfo.val, numSamples);
		}

		float processMix(float m, float x0, float x1) noexcept
		{
#if PPDEqualLoudnessMix
			const auto m0 = std::sqrt(1.f - m);
			const auto m1 = std::sqrt(m);
			return m0 * x0 + m1 * x1;
#else
			return x0 + m * (x1 - x0);
#endif
		}

		void processMix(ProcessorBufferView& view) noexcept
		{
			const auto numChannels = view.getNumChannelsMain();
			const auto numSamples = view.numSamples;
			const auto mixInfo = mix(numSamples);
			if (mixInfo.smoothing)
			{
				for (auto ch = 0; ch < numChannels; ++ch)
				{
					const auto dry = bandDry[ch].data();
					auto smpls = view.getSamplesMain(ch);
					for (auto s = 0; s < numSamples; ++s)
					{
						const auto m = mixInfo[s];
						const auto x0 = dry[s];
						const auto x1 = smpls[s];
						smpls[s] = processMix(m, x0, x1);
					}
				}
				return;
			}
			const auto m = mixInfo.val;
			if (m == 1.f)
				return;
			for (auto ch = 0; ch < numChannels; ++ch)
			{
				const auto dry = bandDry[ch].data();
				auto smpls = view.getSamplesMain(ch);
				for (auto s = 0; s < numSamples; ++s)
				{
					const auto x0 = dry[s];
					const auto x1 = smpls[s];
					smpls[s] = processMix(m, x0, x1);
				}
			}
		}

		void processDelta(ProcessorBufferView& view) noexcept
		{
			const auto numChannels = view.getNumChannelsMain();
			const auto numSamples = view.numSamples;
			const auto mixInfo = mix(numSamples);
			if (mixInfo.smoothing)
			{
				for (auto ch = 0; ch < numChannels; ++ch)
				{
					const auto dry = bandDry[ch].data();
					auto smpls = view.getSamplesMain(ch);
					for (auto s = 0; s < numSamples; ++s)
					{
						const auto m = mixInfo[s];
						const auto x0 = dry[s];
						const auto x1 = smpls[s];
						smpls[s] = m * (x1 - x0);
					}
				}
				return;
			}
			const auto m = mixInfo.val;
			if (m == 0.f)
				return view.clearMain();
			for (auto ch = 0; ch < numChannels; ++ch)
			{
				const auto dry = bandDry[ch].data();
				auto smpls = view.getSamplesMain(ch);
				for (auto s = 0; s < numSamples; ++s)
				{
					const auto x0 = dry[s];
					const auto x1 = smpls[s];
					smpls[s] = m * (x1 - x0);
				}
			}
		}

		void processMixOrDelta(ProcessorBufferView& view) noexcept
		{
#if PPDHasDelta
			if (delta)
				return processDelta(view);
#endif
			processMix(view);
		}
	};
}