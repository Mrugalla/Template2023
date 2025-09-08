#pragma once
#if PPDHasOnsetDetector
#include "OnsetAxiom.h"
#include "OnsetBuffer.h"
#include "../Resonator.h"
#include "../EnvelopeFollower.h"
#include "../midi/Sysex.h"

#define NoDebug 0
#define DebugRatioSum 1
#define DebugRatioMax 2
#define DebugSpecEnv 3
#define DebugMode 2

#define SendEvent true

namespace dsp
{
	// ✨ The onset detectow cwass detectsy the sampwe index of an onset, if 1 existsy >w< ✨
	// 
	//  ／l、     
	// （ﾟ､ ｡７   Nyaa~ it’s finding da beaties uwu
	//  l、 ~ヽ   
	//  じしf_, )ノ
	//
	// (⁄˘⁄ ⁄ ω⁄ ⁄ ˘⁄⁄) detectsy da boom-boom pointy
	struct OnsetDetector
	{
		OnsetDetector() :
			reso(),
			envFols(),
			buffer(),
			sampleRate(1.),
			freqHz(1.),
			attack(OnsetAtk1Default),
			decay(OnsetDcy0Default),
			bw(5000.),
			gain(1.f)
		{
		}

		// parameters:

		void setAttack(double a) noexcept
		{
			attack = a;
			const auto sampleRateInv = 1. / sampleRate;
			const auto waveLength = math::freqHzToSamples(freqHz, sampleRate);
			const auto ms = math::samplesToMs(waveLength, sampleRateInv);
			envFols[1].setAttack(ms * attack);
		}

		void setDecay(double d, int i) noexcept
		{
			decay = d;
			const auto sampleRateInv = 1. / sampleRate;
			const auto waveLength = math::freqHzToSamples(freqHz, sampleRate);
			const auto ms = math::samplesToMs(waveLength, sampleRateInv);
			envFols[i].setDecay(ms * decay);
		}

		void setCutoff(double f) noexcept
		{
			freqHz = f;
			reso.setCutoffFc(math::freqHzToFc(freqHz, sampleRate));
			reso.update();
		}

		void setBandwidth(double q) noexcept
		{
			bw = q;
			reso.setBandwidth(math::freqHzToFc(bw, sampleRate));
			reso.update();
		}

		void setGain(float g) noexcept
		{
			gain = g;
		}

		// process:

		void prepare(double _freqHz, double _sampleRate) noexcept
		{
			freqHz = _freqHz;
			sampleRate = _sampleRate;
			for (auto& e : envFols)
				e.prepare(sampleRate);
			setCutoff(freqHz);
			setBandwidth(bw);
			reso.reset();
			setAttack(attack);
			setDecay(decay, 0);
			setDecay(decay, 1);
		}

		void copyFrom(OnsetBuffer& other, int numSamples) noexcept
		{
			buffer.copyFrom(other, numSamples);
		}

		void resonate(int numSamples) noexcept
		{
			auto samples = buffer.getSamples();
			for (auto s = 0; s < numSamples; ++s)
				samples[s] = static_cast<float>(reso(samples[s]));
		}

		void synthesizeEnvelopeFollowers(int numSamples) noexcept
		{
			const auto samples = buffer.getSamples();
			for (auto& e : envFols)
				e(samples, numSamples);
		}

		void addTo(OnsetBuffer& _buffer, int numSamples) noexcept
		{
			const auto& e1 = envFols[0];
			const auto& e2 = envFols[1];
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto v0 = e1[s];
				const auto v1 = e2[s];
				const auto v2 = v1 + 1e-6f;
				oopsie(v2 == 0.f);
				const auto y = gain * v0 / v2;
				_buffer[s] += y;
			}
		}

		void operator()(int numSamples) noexcept
		{
			const auto& e1 = envFols[0];
			const auto& e2 = envFols[1];
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto v0 = e1[s];
				const auto v1 = e2[s];
				const auto v2 = v1 + 1e-6f;
				oopsie(v2 == 0.f);
				const auto y = gain * v0 / v2;
				buffer[s] = y;
			}
		}

		float processSample(int s) noexcept
		{
			const auto& e1 = envFols[0];
			const auto& e2 = envFols[1];
			const auto v0 = e1[s];
			const auto v1 = e2[s];
			const auto v2 = v1 + 1e-6f;
			oopsie(v2 == 0.f);
			const auto y = gain * v0 / v2;
			buffer[s] = y;
			return y;
		}

		// getters:

		OnsetBuffer& getBuffer() noexcept
		{
			return buffer;
		}

		float getMaxMag(int numSamples) const noexcept
		{
			return buffer.getMaxMag(numSamples);
		}

		const float& operator[](int i) const noexcept
		{
			return buffer[i];
		}
	private:
		Resonator3 reso;
		std::array<EnvelopeFollower, 2> envFols;
		OnsetBuffer buffer;
		double sampleRate, freqHz, attack, decay, bw;
		float gain;
	};

	struct OnsetStrongHold
	{
		OnsetStrongHold() :
			sampleRate(1.),
			lengthD(OnsetHoldDefault),
			timer(0),
			length(0)
		{
		}

		void prepare(double _sampleRate) noexcept
		{
			sampleRate = _sampleRate;
			setLength(lengthD);
		}

		void reset() noexcept
		{
			timer = 0;
		}

		void operator()(int numSamples) noexcept
		{
			timer += numSamples;
		}

		bool youShallNotPass() const noexcept
		{
			return timer < length;
		}

		bool youShallPass() const noexcept
		{
			return !youShallNotPass();
		}

		void setLength(double l) noexcept
		{
			lengthD = l;
			length = static_cast<int>(math::msToSamples(lengthD, sampleRate));
			timer = 0;
		}
	private:
		double sampleRate, lengthD;
		int timer, length;
	};

	class OnsetDetector2
	{
		static constexpr auto NumBands = 32;
		static constexpr auto LowestFreqHz = 1000.f;
		static constexpr auto HighestFreqHz = 15000.f;
		static constexpr auto NumBandsInv = 1.f / static_cast<float>(NumBands);
		static constexpr auto MaxBands = NumBands - 1;
	public:
		OnsetDetector2() :
			buffer(),
			detectors(),
			strongHold(),
			threshold(OnsetThresholdDefault),
			onset(-1), onsetOut(-1),
			sysex()
		{
			for (auto& d : detectors)
				d.setDecay(OnsetDcy1Default, 1);
			setTilt(OnsetTiltDefault);
			sysex.makeBytesOnset();
		}

		// parameters:

		void setAttack(double x) noexcept
		{
			for (auto& d : detectors)
				d.setAttack(x);
		}

		void setDecay(double x, int i) noexcept
		{
			for (auto& d : detectors)
				d.setDecay(x, i);
		}

		void setTilt(float db) noexcept
		{
			db *= .5f;
			const auto lowestGain = math::dbToAmp(-db);
			const auto highestGain = math::dbToAmp(db);
			const auto rangeGain = highestGain - lowestGain;
			for (auto i = 0; i < NumBands; ++i)
			{
				const auto iF = static_cast<float>(i);
				const auto iR = iF / static_cast<float>(MaxBands);
				const auto gain = lowestGain + iR * rangeGain;
				detectors[i].setGain(gain * NumBandsInv * NumBandsInv);
			}
		}

		void setThreshold(float db) noexcept
		{
			threshold = math::dbToAmp(db);
		}

		void setHoldLength(double ms) noexcept
		{
			strongHold.setLength(ms);
		}

		// process:

		void prepare(double sampleRate) noexcept
		{
			const auto lowestPitch = math::freqHzToNote2(LowestFreqHz);
			const auto highestPitch = math::freqHzToNote2(HighestFreqHz);
			const auto rangePitch = highestPitch - lowestPitch;
			for (auto i = 0; i < NumBands; ++i)
			{
				const auto iF = static_cast<float>(i);
				const auto iR = iF / static_cast<float>(MaxBands);
				const auto pitch = lowestPitch + iR * rangePitch;
				const auto freqHz = static_cast<double>(math::noteToFreqHz2(pitch));
				detectors[i].prepare(freqHz, sampleRate);
			}
			strongHold.prepare(sampleRate);
		}

		void operator()(ProcessorBufferView& view) noexcept
		{
			onset = -1;
			strongHold(view.numSamples);
			buffer.copyFromMid(view);
			buffer.rectify(view.numSamples);
			for (auto i = 0; i < NumBands; ++i)
			{
				auto& detector = detectors[i];
				detector.copyFrom(buffer, view.numSamples);
				detector.resonate(view.numSamples);
				detector.synthesizeEnvelopeFollowers(view.numSamples);
			}
#if DebugMode == DebugRatioSum
			buffer.clear(view.numSamples);
			for (auto i = 0; i < NumBands; ++i)
			{
				auto& detector = detectors[i];
				detector(buffer, view.numSamples);
			}
#elif DebugMode == DebugRatioMax
			for (auto s = 0; s < view.numSamples; ++s)
			{
				auto max = 0.f;
				for (auto i = 0; i < NumBands; ++i)
				{
					auto& detector = detectors[i];
					detector.processSample(s);
					if(max < detector[s])
						max = detector[s];
				}
				if (max > threshold)
				{
					if (strongHold.youShallPass())
						onset = s;
					strongHold.reset();
				}
				if(strongHold.youShallPass())
					buffer[s] = max;
				else
					buffer[s] = 0.f;
				
			}
#elif DebugMode == DebugSpecEnv
			for (auto s = 0; s < view.numSamples; ++s)
			{
				auto idx = 0;
				for (auto i = 1; i < NumBands; ++i)
				{
					auto& detector = detectors[i];
					detector.processSample(s);
					if (detector[s] > specEnvThreshold)
						idx = i;
				}
				const auto y = static_cast<float>(idx) / static_cast<float>(NumBands - 1);
				buffer[s] = y;
			}
#else
			// final implementation and stuff
#endif
			buffer.copyTo(view);
		}

		void operator()(float* const* samples, MidiBuffer& midi,
			int numChannels, int numSamples) noexcept
		{
			onsetOut = -1;
			for (auto s = 0; s < numSamples; s += BlockSize)
			{
				const auto numSamplesBlock = std::min(BlockSize, numSamples - s);
				float* block[] = { &samples[0][s], &samples[1][s] };
				ProcessorBufferView view;
				view.assignMain(block, numChannels, numSamplesBlock);
				operator()(view);
				if (onsetOut == -1 && onset != -1)
				{
					onsetOut = onset + s;
#if SendEvent
					midi.addEvent(sysex.midify(), onsetOut);
#endif
				}
			}
		}
	private:
		OnsetBuffer buffer;
		std::array<OnsetDetector, NumBands> detectors;
		OnsetStrongHold strongHold;
		float threshold;
		int onset, onsetOut;
		Sysex sysex;
	};
}

#undef NoDebug
#undef DebugRatioSum
#undef DebugRatioMax
#undef DebugSpecEnv
#undef DebugMode
#undef SendEvent
#endif

/*
todo:
make bandwidth related to pitch
	scale bandwidth with parameter
try different freq ranges and NumBands
find crash in bitwig
*/