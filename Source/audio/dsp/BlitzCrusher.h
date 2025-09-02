#pragma once
#include "ProcessorBufferView.h"

namespace dsp
{
	struct BlitzCrusher
	{
		struct Params
		{
			Params() :
				b(.5f),
				c(.5f),
				a(3)
			{}

			float b, c; // [0,1]
			int a; // [1, n]
		};

		BlitzCrusher() :
			params()
		{}

		void operator()(ProcessorBufferView& view) noexcept
		{
			for (auto ch = 0; ch < view.getNumChannelsMain(); ++ch)
			{
				auto smpls = view.getSamplesMain(ch);
				for (auto s = 0; s < view.numSamples; ++s)
					smpls[s] = process(smpls[s]);
			}
		}

	private:
		Params params;

		float process(float x) noexcept
		{
			auto add = params.c, y = 0.f;
			for (auto i = 0; i < params.a; ++i)
			{
				if(y < x)
					y += add;
				else
					y -= add;
				add *= params.b;
			}
			return y;
		}
	};
}