#pragma once
#include "EnvelopeFollower.h"

#define NoDebug 0
#define DebugRatio 1
#define DebugOnset 2
#define DebugMode 2

namespace dsp
{
	class OnsetDetector
	{
		struct StrongHold
		{
			StrongHold() :
				timer(0),
				length(0)
			{}

			void reset() noexcept
			{
				timer = 0;
			}

			void operator()(int numSamples) noexcept
			{
				timer += numSamples;
			}

			bool youShallPass() const noexcept
			{
				return timer >= length;
			}

			void setLength(int l) noexcept
			{
				length = l;
			}
		private:
			int timer, length;
		};
	public:
		OnsetDetector() :
			envFols(),
#if DebugMode == DebugRatio
			buffer(),
#endif
			strongHold(),
			onset(0)
		{
			for (auto& e : envFols)
			{
				e.setGain(0.f);
				e.setSmooth(0.f);
			}
			envFols[0].setAttack(0.f); envFols[0].setDecay(10.f);
			envFols[1].setAttack(1.f); envFols[1].setDecay(20.f);
		}

		// parameters:

		void setSmooth(double ms) noexcept
		{
			for (auto& e : envFols)
				e.setSmooth(ms);
		}

		void setAttack(double ms, int i) noexcept
		{
			envFols[i].setAttack(ms);
		}

		void setDecay(double ms, int i) noexcept
		{
			envFols[i].setDecay(ms);
		}

		// process

		void prepare(double sampleRate)
		{
			for (auto& e : envFols)
				e.prepare(sampleRate);
			const auto holdLength = math::msToSamples(40., sampleRate);
			strongHold.setLength(static_cast<int>(holdLength));
		}

		void operator()(ProcessorBufferView& view) noexcept
		{
			synthesizeEnvelopeFollowers(view);
			findOnset(view.numSamples);
#if DebugMode == DebugRatio
			dbgRatio(view);
#elif DebugMode == DebugOnset
			dbgOnset(view);
#endif
		}

	private:
		std::array<EnvelopeFollower, 2> envFols;
#if DebugMode == DebugRatio
		std::array<float, BlockSize> buffer;
#endif
		StrongHold strongHold;
		int onset;

		void synthesizeEnvelopeFollowers(ProcessorBufferView& view) noexcept
		{
			for (auto& e : envFols)
				e(view);
		}

		void findOnset(int numSamples) noexcept
		{
			strongHold(numSamples);
			onset = -1;
			const auto& e1 = envFols[0];
			const auto& e2 = envFols[1];
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto v0 = e1[s];
				const auto v1 = e2[s];
				const auto y = v0 / (v1 + 1e-6f);
#if DebugMode == DebugRatio
				buffer[s] = y;
				if (onset == -1 && y >= 1.f)
				{
					if (strongHold.youShallPass())
						onset = s;
					strongHold.reset();
				}
#else
				if (y >= 1.f)
				{
					if(strongHold.youShallPass())
						onset = s;
					strongHold.reset();
					return;
				}
#endif
			}
		}

#if DebugMode == DebugRatio
		void dbgRatio(ProcessorBufferView& view)
		{
			for (auto ch = 0; ch < view.getNumChannelsMain(); ++ch)
			{
				auto smpls = view.getSamplesMain(ch);
				for (auto s = 0; s < view.numSamples; ++s)
				{
					smpls[s] = buffer[s];
				}
			}
		}
#elif DebugMode == DebugOnset
		void dbgOnset(ProcessorBufferView& view)
		{
			for (auto ch = 0; ch < view.getNumChannelsMain(); ++ch)
				SIMD::clear(view.getSamplesMain(ch), view.numSamples);
			if (onset == -1)
				return;
			for (auto ch = 0; ch < view.getNumChannelsMain(); ++ch)
			{
				auto smpls = view.getSamplesMain(ch);
				smpls[onset] = 1.f;
			}
		}
#endif
	};
}

#undef NoDebug
#undef DebugRatio
#undef DebugOnset
#undef DebugMode