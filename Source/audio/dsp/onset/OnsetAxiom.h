#pragma once

namespace dsp
{
	static constexpr auto OnsetNumBandsDefault = 12.f;
	static constexpr auto OnsetNumBandsMax = 16;
	static constexpr auto OnsetLowestFreqHz = 287.76f;
	static constexpr auto OnsetHighestFreqHz = 14660.f;
	// Percent params (lin)
	static constexpr auto OnsetTimeMin = 0;
	static constexpr auto OnsetTimeMax = 8;
	static constexpr auto OnsetAtkDefault = 7.f;
	static constexpr auto OnsetDcyMax = 11;
	static constexpr auto OnsetDcyDefault = 5.62f;
	static constexpr auto OnsetBandwidthMin = -12;
	static constexpr auto OnsetBandwidthMax = 0;
	static constexpr auto OnsetBandwidthDefault = -4.f;
	// Decibel params (lin)
	static constexpr auto OnsetTiltMin = 0;
	static constexpr auto OnsetTiltMax = 36;
	static constexpr auto OnsetTiltDefault = 26.8f;
	static constexpr auto OnsetThresholdMin = -24;
	static constexpr auto OnsetThresholdMax = 0;
	static constexpr auto OnsetThresholdDefault = -7.3f;
	static constexpr auto OnsetHoldMin = 10;
	static constexpr auto OnsetHoldMax = 60;
	static constexpr auto OnsetHoldDefault = 30.f;
	// No Param
	static constexpr auto OnsetDecay0Percent = .354066985646;
}