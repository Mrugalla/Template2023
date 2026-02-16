#pragma once
#include "../../arch/State.h"
#include "Using.h"

namespace dsp
{
	inline File getSamplesDirectory(arch::State& state)
	{
		const auto& user = *state.props.getUserSettings();
		const auto dirDev = user.getFile().getParentDirectory();
		const auto dirSamples = dirDev.getChildFile("Samples");
		if (!dirSamples.exists())
			dirSamples.createDirectory();
		return dirSamples;
	}

	struct RumbleSampler
	{
		RumbleSampler(arch::State& state)
		{
			auto dir = getSamplesDirectory(state);
			DBG(dir.getFullPathName());
		}
	};
}