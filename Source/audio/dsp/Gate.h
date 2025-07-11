#pragma once
#include "Smooth.h"

namespace dsp
{
	struct Gate
	{
		Gate() :
			envFols{ -60., -60. },
			sampleRate(1.), attackMs(4.), releaseMs(24.),
			attack(0.), release(0.),
			thresholdDb(-25.0f), ratioDb(40.0f), kneeDb(12.f)
		{ }

		// Parameters

		void setThresholdDb(float x) noexcept
		{
			thresholdDb = x;
		}

		void setRatioDb(float x) noexcept
		{
			ratioDb = x;
		}

		void setKneeDb(float x) noexcept
		{
			kneeDb = x;
		}

		void setAttackMs(double x) noexcept
		{
			attackMs = x;
			attack = smooth::Lowpass::getXFromMs(attackMs, sampleRate);
		}

		void setReleaseMs(double x) noexcept
		{
			releaseMs = x;
			release = smooth::Lowpass::getXFromMs(releaseMs, sampleRate);
		}

		// Process

		void prepare(double _sampleRate)
		{
			sampleRate = _sampleRate;
			setAttackMs(attackMs);
			setReleaseMs(releaseMs);
			for (auto& envFol : envFols)
				envFol.reset();
		}

		float operator()(float x) noexcept
		{
			if (x == 0.f)
				return 0.f;
			const auto xRect = std::abs(x);
			const auto xDb = math::ampToDecibel(xRect);
			const auto envDb = getEnv(xDb);
			const auto transferFuncDb = downwardsExpander(envDb);
			const auto gainDb = transferFuncDb - envDb;
			const auto gain = math::dbToAmp(gainDb);
			return x * gain;
		}
	private:
		std::array<smooth::Lowpass, 3> envFols;
		double sampleRate, attackMs, releaseMs, attack, release;
		float thresholdDb, ratioDb, kneeDb;

		float downwardsExpander(float xDb) noexcept
		{
			const auto k2 = kneeDb * .5f;
			const auto t0 = thresholdDb - k2;

			if (xDb < t0)
				return ratioDb * (xDb - thresholdDb) + thresholdDb;

			const auto t1 = thresholdDb + k2;

			if (xDb > t1)
				return xDb;
			oopsie(kneeDb == 0.f);
			auto x0 = (xDb - t1);
			x0 *= x0;
			const auto m = -(ratioDb - 1.f) / (2.f * kneeDb);

			return xDb + m * x0;
		}

		float getEnv(float xDb) noexcept
		{
			auto i = 0.;
			for (auto& envFol : envFols)
			{
				const auto a = 1. / std::pow(2., i);

				if (xDb < thresholdDb)
					envFol.setX(release * a);
				else
					envFol.setX(attack * a);
				xDb = static_cast<float>(envFol(xDb));
				++i;
			}
			return xDb;
		}
	};
}