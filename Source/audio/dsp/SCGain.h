#pragma once
#include "Using.h"
#include "../../arch/Param.h"

namespace dsp
{
	struct SCGain
	{
		SCGain(param::Param&);

		void reset() noexcept;

		void setListening(bool) noexcept;

		// samples, numChannels, numSamples
		void operator()(float* const*, int, int) noexcept;

		bool isListening() const noexcept;
	private:
		param::Param& scGain;
		std::atomic<bool> listening;
		std::atomic<float> maxPeak;
	};
}