#pragma once
#include "../Using.h"

namespace dsp
{
	struct SlewLimiter
	{
        enum Type { LP, HP, NumTypes };
        using FilerTypeFunc = std::function<double()>;
        using FilterTypeFuncs = std::array<FilerTypeFunc, NumTypes>;
       
        SlewLimiter() :
            y(0.),
            dist(0.),
            filterTypeFuncs()
        {
            filterTypeFuncs[LP] = [&x = y]()
            {
                return x;
            };
            filterTypeFuncs[HP] = [&x = dist]()
			{
				return x;
			};
        }

        void operator()(double* smpls, double slew,
            int numSamples, Type type) noexcept
        {
            for(auto s = 0; s < numSamples; ++s)
				smpls[s] = process(smpls[s], slew, type);
        }

    protected:
        double y, dist;
        FilterTypeFuncs filterTypeFuncs;

        double process(double x, double slew, Type type) noexcept
        {
            dist = x - y;
            const auto pol = dist < 0. ? -1. : 1.;
            y += dist * pol > slew ? slew * pol : dist;
            return filterTypeFuncs[type]();
        }
	};

    struct SlewLimiterStereo
    {
        using Type = SlewLimiter::Type;

        static double freqHzToSlewRate(double freq, double sampleRate) noexcept
        {
            return freq / sampleRate;
        }

        SlewLimiterStereo() :
			slews(),
            sampleRate(1.)
		{}

        /* samples, slewRate, numChannels, numSamples, filterType */
        void operator()(double** samples, double slew,
            int numChannels, int numSamples, Type type) noexcept
        {
            for (auto ch = 0; ch < numChannels; ++ch)
                slews[ch](samples[ch], slew, numSamples, type);
        }

    protected:
        std::array<SlewLimiter, 2> slews;
        double sampleRate;
    };
}