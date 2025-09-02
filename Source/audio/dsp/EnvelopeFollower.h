#pragma once
#include "ProcessorBufferView.h"
#include "PRM.h"

namespace dsp
{
	struct EnvelopeFollower
	{
		struct Params
		{
			// gainDb, atkMs, dcyMs, smoothMs
			Params(float = 0.f, float = 1.f,
				float = 100.f, float = 1.f);

			// sampleRate
			void prepare(double) noexcept;

			// db
			void setGain(float) noexcept;

			// ms
			void setAtk(double) noexcept;

			// ms
			void setDcy(double) noexcept;

			// ms
			void setSmooth(double) noexcept;

			PRMInfo getGain(int) noexcept;
		private:
			float gainDb;
			double sampleRate, atkMs, dcyMs, smoothMs;
			PRM gainPRM;
		public:
			double atk, dcy, smooth;
			float gain;
		};

		EnvelopeFollower();

		void prepare(double) noexcept;

		// parameters:

		void setGain(float) noexcept;

		void setAttack(double) noexcept;

		void setDecay(double) noexcept;

		void setSmooth(double) noexcept;

		// process:

		void reset(double);

		void operator()(ProcessorBufferView&) noexcept;

		bool isSleepy() const noexcept;

		float operator[](int i) const noexcept;

		float getMeter() const noexcept;
	private:
		Params params;
		std::atomic<float> meter;
		std::array<float, BlockSize> buffer;
		const double MinDb;
		smooth::Lowpass envLP, smooth;
		bool attackState;

		// smpls, numSamples
		void operator()(float*, int) noexcept;

		void copyMid(ProcessorBufferView&) noexcept;

		// smpls, numSamples
		void rectify(float*, int) noexcept;

		// numSamples
		void applyGain(int) noexcept;

		// numSamples
		void synthesizeEnvelope(int) noexcept;

		// s0, s1
		double processAttack(double, double) noexcept;

		// s0, s1
		double processDecay(double, double) noexcept;

		// numSamples
		void processMeter(int) noexcept;
	};
}
