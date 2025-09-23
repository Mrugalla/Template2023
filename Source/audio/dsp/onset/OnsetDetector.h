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
	// (⁄˘⁄ ⁄ ω⁄ ⁄ ˘⁄⁄) detectsy da boom-boom pointy
	struct OnsetCore
	{
		OnsetCore();

		// parameters:

		void setAttack(double) noexcept;

		void setDecay(double, int) noexcept;

		void setBandwidth(double) noexcept;

		void setBandwidthPercent(double) noexcept;

		void setGain(float) noexcept;

		void setFreqHz(double) noexcept;

		void updateFilter() noexcept;

		// process:

		// sampleRate
		void prepare(double) noexcept;

		// other, numSamples
		void copyFrom(OnsetBuffer&, int) noexcept;

		// numSamples
		void resonate(int) noexcept;

		// numSamples
		void synthesizeEnvelopeFollowers(int) noexcept;

		// numSamples
		void operator()(int) noexcept;

		// buffer, s
		void addTo(OnsetBuffer&, int) noexcept;

		// buffer, s
		float processSample(OnsetBuffer&, int) noexcept;

		// s
		float processSample(int) noexcept;

		// getters:

		OnsetBuffer& getBuffer() noexcept;

		// numSamples
		float getMaxMag(int) const noexcept;

		const float& operator[](int) const noexcept;
	private:
		Resonator3 reso;
		std::array<EnvelopeFollower, 2> envFols;
		OnsetBuffer buffer;
		double sampleRate, freqHz, bwHz, bwPercent, attack, decay;
		float gain;

		void updateBandwidth() noexcept;
	};

	struct OnsetStrongHold
	{
		OnsetStrongHold();

		// sampleRate
		void prepare(double) noexcept;

		void reset() noexcept;

		// numSamples
		void operator()(int) noexcept;

		bool youShallNotPass() const noexcept;

		bool youShallPass() const noexcept;

		void setLength(double) noexcept;
	private:
		double sampleRate, lengthD;
		int timer, length;
	};

	struct OnsetDetector
	{
		OnsetDetector();

		// parameters:

		void setAttack(double) noexcept;

		void setDecay(double) noexcept;

		void setTilt(float) noexcept;

		void setThreshold(float) noexcept;

		void setHoldLength(double) noexcept;

		void setBandwidth(double) noexcept;

		void setNumBands(int) noexcept;

		void setLowestPitch(double) noexcept;

		void setHighestPitch(double) noexcept;

		// process:

		// sampleRate
		void prepare(double) noexcept;

		void operator()(ProcessorBufferView&) noexcept;

		// samples, midi, numChannels, numSamples
		void operator()(float* const*, MidiBuffer&, int, int) noexcept;
	private:
		OnsetBuffer buffer;
		std::array<OnsetCore, OnsetNumBandsMax> detectors;
		OnsetStrongHold strongHold;
		double sampleRate, lowestPitch, highestPitch;
		float threshold, tilt;
		int numBands, onset, onsetOut;
		Sysex sysex;

		void updatePitchRange() noexcept;

		void updateTilt() noexcept;
	};
}
#endif

/*
todo:
*/