#pragma once
#if PPDHasOnsetDetector
#include "OnsetAxiom.h"
#include "OnsetBuffer.h"
#include "../Resonator.h"
#include "../EnvelopeFollower.h"
#include "../midi/Sysex.h"

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
			freqHz(5000.), bwHz(5000.), bwPercent(1.),
			attack(OnsetAtkDefault),
			decay(OnsetDcyDefault),
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
			bwHz = q;
			updateBandwidth();
		}

		void setBandwidthPercent(double p) noexcept
		{
			bwPercent = p;
			updateBandwidth();
		}

		void setGain(float g) noexcept
		{
			gain = g;
		}

		void setFreqHz(double f) noexcept
		{
			freqHz = f;
			reso.setCutoffFc(math::freqHzToFc(freqHz, sampleRate));
			reso.update();
		}

		// process:

		void prepare(double _sampleRate) noexcept
		{
			sampleRate = _sampleRate;
			for (auto& e : envFols)
				e.prepare(sampleRate);
			setCutoff(freqHz);
			setBandwidth(bwHz);
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

		void addTo(OnsetBuffer& _buffer, int s) noexcept
		{
			const auto& e1 = envFols[0];
			const auto& e2 = envFols[1];
			const auto v0 = e1[s];
			const auto v1 = e2[s];
			const auto v2 = v1 + 1e-6f;
			oopsie(v2 == 0.f);
			const auto y = gain * v0 / v2;
			_buffer[s] += y;
		}

		float processSample(OnsetBuffer& _buffer, int s) noexcept
		{
			const auto& e1 = envFols[0];
			const auto& e2 = envFols[1];
			const auto v0 = e1[s];
			const auto v1 = e2[s];
			const auto v2 = v1 + 1e-6f;
			oopsie(v2 == 0.f);
			const auto y = gain * v0 / v2;
			_buffer[s] = y;
			return y;
		}

		float processSample(int s) noexcept
		{
			return processSample(buffer, s);
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
		double sampleRate, freqHz, bwHz, bwPercent, attack, decay;
		float gain;

		void updateBandwidth() noexcept
		{
			const auto b = bwHz * bwPercent;
			reso.setBandwidth(math::freqHzToFc(b, sampleRate));
			reso.update();
		}
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

	struct Wavelet
	{
		static constexpr int Order = 14;
		static constexpr int Size = 1 << Order;

		Wavelet() :
			callback(nullptr),
			wavelet(),
			mag(0.f),
			idx(0),
			length(0)
		{
		}

		void setFreqSamples(int n) noexcept
		{
			length = n;
			for (auto i = 0; i < Size; ++i)
			{
				const auto iF = static_cast<float>(i);
				const auto iR = iF / static_cast<float>(Size);
				const auto x = 2.f * (iR - .5f);
				const auto w = morletWindow(2.f * x);
				const auto c = std::cos(Tau * x / static_cast<float>(length));
				const auto s = std::sin(Tau * x / static_cast<float>(length));
				wavelet[i] = std::complex<float>(w * c, w * s);
			}
		}

		void setFreqHz(float hz, float sampleRate) noexcept
		{
			const auto waveLength = math::freqHzToSamples(hz, sampleRate);
			const auto n = std::max(1, static_cast<int>(std::round(waveLength)));
			setFreqSamples(n);
		}

		void operator()(float* smpls, int numSamples) noexcept
		{
			for(auto s = 0; s < numSamples; ++s)
			{
				const auto smpl = smpls[s];
				const auto y = std::abs(smpl * wavelet[idx]);
				mag = std::max(mag, y);
				++idx;
				if (idx == Size)
				{
					callback(mag);
					idx = 0;
					mag = 0.f;
				}
			}
		}

		std::function<void(float)> callback;

		int getLength() const noexcept
		{
			return length;
		}
	protected:
		std::array<std::complex<float>, Size> wavelet;
		float mag;
		int idx, length;

		// x [-1, 1]
		float morletWindow(float x) const noexcept
		{
			return std::exp(-.5f * x * x);
		}
	};

	struct RhythmWavelet
	{
		RhythmWavelet() :
			callback(),
			wl8th(),
			wl16th(),
			bpm(120.f),
			sampleRate(1.),
			mag8(0.f),
			mag16(0.f)
		{
			wl8th.callback = [&](float m)
			{
				mag8 = m;
				callback(mag8 > mag16 ? wl8th.getLength() : wl16th.getLength());
				DBG(mag8 << " :: " << mag16);
			};
			wl16th.callback = [&](float m)
			{
				mag16 = m;
				callback(mag8 > mag16 ? wl8th.getLength() : wl16th.getLength());
				DBG(mag8 << " :: " << mag16);
			};
		}

		void prepare(float _sampleRate) noexcept
		{
			sampleRate = _sampleRate;
			updateWavelets();
		}

		void setBpm(float _bpm) noexcept
		{
			bpm = _bpm;
			updateWavelets();
		}

		void operator()(float* smpls, int numSamples) noexcept
		{
			wl8th(smpls, numSamples);
			wl16th(smpls, numSamples);
		}

		std::function<void(int)> callback;
	protected:
		Wavelet wl8th, wl16th;
		float bpm, sampleRate;
		float mag8, mag16;

		void updateWavelets() noexcept
		{
			const auto bps = bpm / 60.f;
			const auto beatLenSamples = sampleRate / bps;
			const auto eighthLenSamples = beatLenSamples * .5f;
			const auto sixteenthLenSamples = beatLenSamples * .25f;
			wl8th.setFreqSamples(static_cast<int>(std::round(eighthLenSamples)));
			wl16th.setFreqSamples(static_cast<int>(std::round(sixteenthLenSamples)));
		}
	};

	class OnsetDetector2
	{
		static constexpr auto Decay0Percent = .354066985646;
	public:
		OnsetDetector2() :
			wavelet(),
			buffer(),
			detectors(),
			strongHold(),
			sampleRate(44100.), lowestPitch(math::freqHzToNote2(4000.)), highestPitch(math::freqHzToNote2(15000.)),
			threshold(OnsetThresholdDefault), tilt(0.f),
			numBands(16), onset(-1), onsetOut(-1),
			sysex()
		{
			wavelet.callback = [&](int len)
			{
				const auto lenD = static_cast<double>(len / 2);
				strongHold.setLength(math::samplesToMs(lenD, 1.f / sampleRate));
			};

			const auto bwPercentDefault = std::pow(2., static_cast<double>(OnsetBandwidthDefault));
			setBandwidth(bwPercentDefault);
			setDecay(OnsetDcyDefault);
			setTilt(OnsetTiltDefault);
			sysex.makeBytesOnset();
		}

		// parameters:

		void setAttack(double x) noexcept
		{
			for (auto& d : detectors)
				d.setAttack(x);
		}

		void setDecay(double x) noexcept
		{
			for (auto& d : detectors)
				d.setDecay(x, 1);
			auto d = Decay0Percent * x;
			for (auto& dtr : detectors)
				dtr.setDecay(d, 0);
		}

		void setTilt(float db) noexcept
		{
			tilt = db;
			updateTilt();
		}

		void setThreshold(float db) noexcept
		{
			threshold = math::dbToAmp(db);
		}

		void setHoldLength(double ms) noexcept
		{
			//strongHold.setLength(ms);
		}

		void setBandwidth(double b) noexcept
		{
			for(auto& d : detectors)
				d.setBandwidthPercent(b);
		}

		void setNumBands(int n) noexcept
		{
			numBands = n;
			updatePitchRange();
			updateTilt();
		}

		void setLowestPitch(double p) noexcept
		{
			lowestPitch = p;
			updatePitchRange();
		}

		void setHighestPitch(double p) noexcept
		{
			highestPitch = p;
			updatePitchRange();
		}

		void setBpm(float bpm) noexcept
		{
			wavelet.setBpm(bpm);
		}

		// process:

		void prepare(double _sampleRate) noexcept
		{
			wavelet.prepare(static_cast<float>(_sampleRate));
			sampleRate = _sampleRate;
			for(auto& d : detectors)
				d.prepare(sampleRate);
			updatePitchRange();
			strongHold.prepare(sampleRate);
		}

		void operator()(ProcessorBufferView& view) noexcept
		{
			onset = -1;
			strongHold(view.numSamples);
			buffer.copyFromMid(view);
			wavelet(buffer.getSamples(), view.numSamples);
			buffer.rectify(view.numSamples);
			for (auto i = 0; i < numBands; ++i)
			{
				auto& detector = detectors[i];
				detector.copyFrom(buffer, view.numSamples);
				detector.resonate(view.numSamples);
				detector.synthesizeEnvelopeFollowers(view.numSamples);
			}
			for (auto s = 0; s < view.numSamples; ++s)
			{
				auto max = 0.f;
				for (auto i = 0; i < numBands; ++i)
				{
					auto& detector = detectors[i];
					detector.processSample(s);
					if (max < detector[s])
						max = detector[s];
				}
				if (max > threshold)
				{
					if (strongHold.youShallPass())
						onset = s;
					strongHold.reset();
				}
			}
		}

		void operator()(float* const* samples, MidiBuffer& midi,
			int numChannels, int numSamples) noexcept
		{
			onsetOut = -1;
			for (auto s = 0; s < numSamples; s += BlockSize)
			{
				const auto remainingSamples = numSamples - s;
				const auto numSamplesBlock = std::min(BlockSize, remainingSamples);
				float* block[] = { &samples[0][s], &samples[1][s] };
				ProcessorBufferView view;
				view.assignMain(block, numChannels, numSamplesBlock);
				operator()(view);
				if (onsetOut == -1 && onset != -1)
				{
					onsetOut = onset + s;
					midi.addEvent(sysex.midify(), onsetOut);
				}
			}
		}
	private:
		RhythmWavelet wavelet;
		OnsetBuffer buffer;
		std::array<OnsetDetector, OnsetNumBandsMax> detectors;
		OnsetStrongHold strongHold;
		double sampleRate, lowestPitch, highestPitch;
		float threshold, tilt;
		int numBands, onset, onsetOut;
		Sysex sysex;

		void updatePitchRange() noexcept
		{
			const auto rangePitch = highestPitch - lowestPitch;
			for (auto i = 0; i < numBands; ++i)
			{
				const auto iF = static_cast<float>(i);
				const auto iR = iF / static_cast<float>(numBands - 1);
				const auto pitch = lowestPitch + iR * rangePitch;
				const auto freqHz = static_cast<double>(math::noteToFreqHz2(pitch));
				const auto pitchLow = pitch - .5f;
				const auto pitchHigh = pitch + .5f;
				const auto freqLow = static_cast<double>(math::noteToFreqHz2(pitchLow));
				const auto freqHigh = static_cast<double>(math::noteToFreqHz2(pitchHigh));
				const auto bwHz = freqHigh - freqLow;
				detectors[i].setFreqHz(freqHz);
				detectors[i].setBandwidth(bwHz);
			}
		}

		void updateTilt() noexcept
		{
			const auto lowestGain = math::dbToAmp(-tilt);
			const auto highestGain = math::dbToAmp(tilt);
			const auto rangeGain = highestGain - lowestGain;
			const auto numBandsInv = 1.f / static_cast<float>(numBands);
			const auto bandCompensate = numBandsInv * numBandsInv;
			for (auto i = 0; i < numBands; ++i)
			{
				const auto iF = static_cast<float>(i);
				const auto iR = iF / static_cast<float>(numBands);
				const auto gain = lowestGain + iR * rangeGain;
				detectors[i].setGain(gain * bandCompensate);
			}
		}
	};
}
#endif

/*
todo:
freq range und numBands parameters
*/