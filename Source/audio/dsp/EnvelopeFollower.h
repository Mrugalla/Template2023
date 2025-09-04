#pragma once
#include "ProcessorBufferView.h"
#include "PRM.h"

namespace dsp
{
	struct EnvelopeFollower
	{
		using FilterFunc = std::function<void(float*, int)>;

		struct Params
		{
			// atkMs, dcyMs
			Params(float = 1.f, float = 100.f);

			// sampleRate
			void prepare(double) noexcept;

			// ms
			void setAtk(double) noexcept;

			// ms
			void setDcy(double) noexcept;
		private:
			double sampleRate, atkMs, dcyMs;
			PRM gainPRM;
		public:
			double atk, dcy;
		};

		EnvelopeFollower();

		void prepare(double) noexcept;

		// parameters:

		void setAttack(double) noexcept;

		void setDecay(double) noexcept;

		// process:

		void reset(double);

		void operator()(ProcessorBufferView&) noexcept;

		// smpls, numSamples
		void operator()(float*, int) noexcept;

		bool isSleepy() const noexcept;

		float operator[](int i) const noexcept;

		float getMeter() const noexcept;
	private:
		Params params;
		std::atomic<float> meter;
		std::array<float, BlockSize> buffer;
		const double MinDb;
		smooth::Lowpass envLP;
		bool attackState;

		void copyMid(ProcessorBufferView&) noexcept;

		// smpls, numSamples
		void rectify(float*, int) noexcept;

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
