#pragma once
#include <random>
#include <array>
#include "ProcessorBufferView.h"
#include "../../arch/RumbleAxiom.h"

namespace dsp
{
	struct RumbleSynth
	{
		static constexpr int NoiseOrder = 13;
		static constexpr int NoiseSize = 1 << NoiseOrder;
		static constexpr int NoiseMax = NoiseSize - 1;

		RumbleSynth() :
			random(),
			noise(),
			rHead(0),
			enabled(false)
		{
			fillNoise(420);
		}

		// parameters:
		void setEnabled(bool e) noexcept
		{
			enabled = e;
		}

		// process:
		void operator()(ProcessorBufferView& view) noexcept
		{
			if (!enabled)
				return;
			for(auto ch = 0; ch < view.getNumChannelsMain(); ++ch)
			{
				auto smpls = view.getSamplesMain(ch);
				for(auto s = 0; s < view.numSamples; ++s)
				{
					smpls[s] = static_cast<float>(noise[rHead]);
					rHead = (rHead + 1) & NoiseMax;
				}
			}
			rHead = random.nextInt(NoiseSize);
		}
	private:
		Random random;
		std::array<float, NoiseSize> noise;
		int rHead;
		bool enabled;

		void fillNoise(unsigned int seed) noexcept
		{
			std::mt19937 generator(seed);
			std::uniform_real_distribution<float> distribution(-1.f, 1.f);
			for (auto& n : noise)
				n = distribution(generator);
		}
	};
}