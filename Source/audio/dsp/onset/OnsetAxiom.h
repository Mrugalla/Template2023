#pragma once

namespace dsp
{
	// Percent params (lin)
	static constexpr auto OnsetAtkDcyTiltMin = 0;
	static constexpr auto OnsetAtk1Max = 8;
	static constexpr auto OnsetAtk1Default = 7.f;
	static constexpr auto OnsetDcy0Max = 8;
	static constexpr auto OnsetDcy0Default = 1.48f;
	static constexpr auto OnsetDcy1Max = 11;
	static constexpr auto OnsetDcy1Default = 4.18f;
	// Decibel params (lin)
	static constexpr auto OnsetTiltMax = 36;
	static constexpr auto OnsetTiltDefault = 22.9f;
	static constexpr auto OnsetThresholdMin = -40;
	static constexpr auto OnsetThresholdMax = 0;
	static constexpr auto OnsetThresholdDefault = -24.4f;
	// Millisecond params (quad)
	static constexpr auto OnsetHoldMin = 0;
	static constexpr auto OnsetHoldMax = 60;
	static constexpr auto OnsetHoldQuad = 2;
	static constexpr auto OnsetHoldDefault = 16.f;
}