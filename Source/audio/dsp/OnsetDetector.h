#pragma once
#if PPDHasOnsetDetector
#include "Resonator.h"
#include "EnvelopeFollower.h"
#include "midi/Sysex.h"

#define NoDebug 0
#define DebugRatio 1
#define DebugOnset 2
#define DebugReso 3
#define DebugMode 0


namespace dsp
{
	// ✨ The onset detectow cwass detectsy the sampwe index of an onset, if it existsy >w< ✨
	// 
	//  ／l、     
	// （ﾟ､ ｡７   Nyaa~ it’s finding da beaties uwu
	//  l、 ~ヽ   
	//  じしf_, )ノ
	//
	// (⁄˘⁄ ⁄ ω⁄ ⁄ ˘⁄⁄) detectsy da boom-boom pointy
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
			reso(),
			envFols(),
			buffer(),
			strongHold(),
			sampleRate(1.),
			resoCutoff(5000.),
			resoQ(5000.),
			onset(-1),
			onsetOut(-1),
			sysex()
		{
			envFols[0].setAttack(0.f); envFols[0].setDecay(10.f);
			envFols[1].setAttack(1.f); envFols[1].setDecay(20.f);
		}

		// parameters:

		void setAttack(double ms) noexcept
		{
			envFols[1].setAttack(ms);
		}

		void setDecay(double ms, int i) noexcept
		{
			envFols[i].setDecay(ms);
		}

		void setResoCutoff(double f) noexcept
		{
			resoCutoff = f;
			reso.setCutoffFc(math::freqHzToFc(resoCutoff, sampleRate));
			reso.update();
		}

		void setResoQ(double q) noexcept
		{
			resoQ = q;
			reso.setBandwidth(math::freqHzToFc(resoQ, sampleRate));
			reso.update();
		}

		// process

		void prepare(double _sampleRate)
		{
			sampleRate = _sampleRate;
			for (auto& e : envFols)
				e.prepare(sampleRate);
			const auto holdLength = math::msToSamples(40., sampleRate);
			strongHold.setLength(static_cast<int>(holdLength));
			setResoCutoff(resoCutoff);
			setResoQ(resoQ);
		}

		int operator()(ProcessorBufferView& view) noexcept
		{
			collapseToMid(view);
			resonate(view.numSamples);
#if DebugMode == DebugReso
			dbgReso(view);
			return -1;
#endif
			rectify(view.numSamples);
			synthesizeEnvelopeFollowers(view.numSamples);
			findOnset(view.numSamples);
#if DebugMode == DebugRatio
			dbgRatio(view);
#elif DebugMode == DebugOnset
			dbgOnset(view);
#endif
			return onset;
		}

		void operator()(float* const* samples, MidiBuffer& midi,
			int numChannels, int numSamples) noexcept
		{
			onsetOut = -1;
			for(auto s = 0; s < numSamples; s += BlockSize)
			{
				const auto numSamplesBlock = std::min(BlockSize, numSamples - s);
				float* block[] = { &samples[0][s], &samples[1][s] };
				ProcessorBufferView view;
				view.assignMain(block, numChannels, numSamplesBlock);
				operator()(view);
				if (onsetOut == -1 && onset != -1)
				{
					onsetOut = onset + s;
					sendSysex(midi);
				}
			}
		}
	private:
		Resonator3 reso;
		std::array<EnvelopeFollower, 2> envFols;
		std::array<float, BlockSize> buffer;
		StrongHold strongHold;
		double sampleRate, resoCutoff, resoQ;
		int onset, onsetOut;
		Sysex sysex;

		void collapseToMid(ProcessorBufferView& view) noexcept
		{
			SIMD::copy(buffer.data(), view.getSamplesMain(0), view.numSamples);
			if (view.getNumChannelsMain() != 2)
				return;
			SIMD::add(buffer.data(), view.getSamplesMain(1), view.numSamples);
			SIMD::multiply(buffer.data(), .5f, view.numSamples);
		}

		void resonate(int numSamples) noexcept
		{
			auto p = buffer.data();
			for (auto s = 0; s < numSamples; ++s)
				p[s] = static_cast<float>(reso(p[s]));
		}

		void rectify(int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
				buffer[s] = std::abs(buffer[s]);
		}

		void synthesizeEnvelopeFollowers(int numSamples) noexcept
		{
			const auto envData = buffer.data();
			for (auto& e : envFols)
				e(envData, numSamples);
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

		void sendSysex(MidiBuffer& midi) noexcept
		{
			const auto bytes = sysex.makeBytesOnset();
			midi.addEvent(sysex.midify(bytes), onsetOut);
		}

#if DebugMode == DebugRatio
		void dbgRatio(ProcessorBufferView& view) noexcept
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
		void dbgOnset(ProcessorBufferView& view) noexcept
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
#elif DebugMode == DebugReso
		void dbgReso(ProcessorBufferView& view) noexcept
		{
			for (auto ch = 0; ch < view.getNumChannelsMain(); ++ch)
				SIMD::copy(view.getSamplesMain(ch), buffer.data(), view.numSamples);
		}
#endif
	};
}

#undef NoDebug
#undef DebugRatio
#undef DebugOnset
#undef DebugMode
#endif