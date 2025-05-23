#include "Range.h"
#include <cmath>

namespace makeRange
{
	Range biased(float start, float end, float bias) noexcept
	{
		// https://www.desmos.com/calculator/ps8q8gftcr
		const auto a = bias * .5f + .5f;
		const auto a2 = 2.f * a;
		const auto aM = 1.f - a;

		const auto r = end - start;
		const auto aR = r * a;
		if (bias != 0.f)
			return
		{
				start, end,
				[a2, aM, aR](float min, float, float x)
				{
					const auto denom = aM - x + a2 * x;
					if (denom == 0.f)
						return min;
					return min + aR * x / denom;
				},
				[a2, aM, aR](float min, float, float x)
				{
					const auto denom = a2 * min + aR - a2 * x - min + x;
					if (denom == 0.f)
						return 0.f;
					auto val = aM * (x - min) / denom;
					return val > 1.f ? 1.f : val;
				},
				[](float min, float max, float x)
				{
					return x < min ? min : x > max ? max : x;
				}
		};
		else return { start, end };
	}

	Range biasedSatisfy(float start, float end, float bias) noexcept
	{
		// https://www.desmos.com/calculator/nevb75ufdw
		const auto p = (bias + 1.f) * .5f;
		const auto f1k = std::log(.5f) / std::log(p);
		const auto k1f = std::log(.5f) / std::log(1.f - p);

		return
		{
			start, end,
			[b = f1k, bInv = 1.f / f1k, r = start - end](float, float max, float x)
			{
				return max + r * std::pow(1.f - std::pow(x, b), bInv);
			},
			[b = k1f, bInv = 1.f / k1f, rInv = 1.f / (end - start)](float min, float, float x)
			{
				return 1.f - std::pow(1.f - std::pow((x - min) * rInv, b), bInv);
			},
			[](float min, float max, float x)
			{
				return x < min ? min : x > max ? max : x;
			}
		};
	}

	Range biasedSatisfy(int start, int end, float bias) noexcept
	{
		return biasedSatisfy(static_cast<float>(start), static_cast<float>(end), bias);
	}

	Range stepped(float start, float end, float steps) noexcept
	{
		return { start, end, steps };
	}

	Range stepped(int start, int end, int steps) noexcept
	{
		return { static_cast<float>(start), static_cast<float>(end), static_cast<float>(steps) };
	}

	Range toggle() noexcept
	{
		return stepped(0, 1);
	}

	Range lin(float start, float end) noexcept
	{
		const auto range = end - start;

		return
		{
			start,
			end,
			[range](float min, float, float normalized)
			{
				return min + normalized * range;
			},
			[inv = 1.f / range](float min, float, float denormalized)
			{
				return (denormalized - min) * inv;
			},
			[](float min, float max, float x)
			{
				return juce::jlimit(min, max, x);
			}
		};
	}

	Range lin(int start, int end) noexcept
	{
		return lin(static_cast<float>(start), static_cast<float>(end));
	}

	Range withCentre(float start, float end, float centre) noexcept
	{
		const auto r = end - start;
		const auto v = (centre - start) / r;

		return makeRange::biased(start, end, 2.f * v - 1.f);
	}

	Range withCentre(int start, int end, int centre) noexcept
	{
		return withCentre(static_cast<float>(start), static_cast<float>(end), static_cast<float>(centre));
	}

	Range foleysLogRange(float min, float max) noexcept
	{
		return
		{
			min, max,
			[](float start, float end, float normalised)
			{
				return start + (std::pow(2.f, normalised * 10.f) - 1.f) * (end - start) / 1023.f;
			},
			[](float start, float end, float value)
			{
				return (std::log(((value - start) * 1023.f / (end - start)) + 1.f) / std::log(2.f)) * .1f;
			},
			[](float start, float end, float value)
			{
				// optimised for frequencies: >3 kHz: 2 decimals
				if (value > 3000.f)
					return juce::jlimit(start, end, 100.f * juce::roundToInt(value / 100.f));

			// optimised for frequencies: 1-3 kHz: 1 decimal
			if (value > 1000.f)
				return juce::jlimit(start, end, 10.f * juce::roundToInt(value * .1f));

			return juce::jlimit(start, end, std::round(value));
		}
		};
	}

	Range foleysLogRange(int min, int max) noexcept
	{
		return foleysLogRange(static_cast<float>(min), static_cast<float>(max));
	}

	Range quad(float min, float max, int numSteps) noexcept
	{
		return
		{
			min, max,
			[numSteps, range = max - min](float start, float, float x)
			{
				for (auto i = 0; i < numSteps; ++i)
					x *= x;
				return start + x * range;
			},
			[numSteps, rangeInv = 1.f / (max - min)](float start, float, float x)
			{
				x = (x - start) * rangeInv;
				for (auto i = 0; i < numSteps; ++i)
					x = std::sqrt(x);
				return x;
			},
			[](float start, float end, float x)
			{
				return juce::jlimit(start, end, x);
			}
		};
	}

	Range quad(int min, int max, int numSteps) noexcept
	{
		return quad(static_cast<float>(min), static_cast<float>(max), numSteps);
	}

	Range beats(float minDenominator, float maxDenominator, bool withZero)
	{
		std::vector<float> table;

		const auto minV = std::log2(minDenominator);
		const auto maxV = std::log2(maxDenominator);

		const auto numWholeBeatsF = static_cast<float>(minV - maxV);
		const auto numWholeBeatsInv = 1.f / numWholeBeatsF;

		const auto numWholeBeats = static_cast<int>(numWholeBeatsF);
		const auto numValues = numWholeBeats * 3 + 1 + (withZero ? 1 : 0);
		table.reserve(numValues);
		if (withZero)
			table.emplace_back(0.f);

		for (auto i = 0; i < numWholeBeats; ++i)
		{
			const auto iF = static_cast<float>(i);
			const auto x = iF * numWholeBeatsInv;

			const auto curV = minV - x * numWholeBeatsF;
			const auto baseVal = std::pow(2.f, curV);

			const auto valWhole = 1.f / baseVal;
			const auto valTriplet = valWhole * 1.666666666667f;
			const auto valDotted = valWhole * 1.75f;

			table.emplace_back(valWhole);
			table.emplace_back(valTriplet);
			table.emplace_back(valDotted);
		}
		table.emplace_back(1.f / maxDenominator);

		static constexpr float Eps = 1.f - std::numeric_limits<float>::epsilon();
		static constexpr float EpsInv = 1.f / Eps;

		const auto numValuesF = static_cast<float>(numValues);
		const auto numValuesInv = 1.f / numValuesF;
		const auto numValsX = numValuesInv * EpsInv;
		const auto normValsY = numValuesF * Eps;

		const auto minVal = table.front();
		const auto maxVal = table.back();

		return
		{
			minVal, maxVal,
			[table, normValsY](float, float, float normalized)
			{
				const auto valueIdx = normalized * normValsY;
				return table[static_cast<int>(valueIdx)];
			},
			[table, numValsX](float, float, float denormalized)
			{
				for (auto i = 0; i < table.size(); ++i)
				{
					const auto val = table[i];
					if (denormalized <= val)
						return static_cast<float>(i) * numValsX;
				}
					
				return 0.f;
			},
			[](float start, float end, float denormalized)
			{
				return juce::jlimit(start, end, denormalized);
			}
		};
	}
}
