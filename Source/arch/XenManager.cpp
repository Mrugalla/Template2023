#include "XenManager.h"
#include "Math.h"
#include <cmath>

namespace arch
{
	XenManager::XenManager() :
		xen(12.f),
		masterTune(440.f),
		baseNote(69.f),
		temperaments()
	{
		for (auto& t : temperaments)
			t = 0.f;
	}

	void XenManager::setTemperament(float tmprVal, int noteVal) noexcept
	{
		temperaments[noteVal] = tmprVal;
		const auto idx2 = noteVal + PPDMaxXen;
		if (idx2 >= temperaments.size())
			temperaments[idx2] = tmprVal;
	}

	void XenManager::operator()(float _xen, float _masterTune, float _baseNote) noexcept
	{
		xen = _xen;
		masterTune = _masterTune;
		baseNote = _baseNote;
	}

	template<typename Float>
	Float XenManager::noteToFreqHz(Float note) const noexcept
	{
		if (note < static_cast<Float>(0))
			note = static_cast<Float>(0);
		else if (note > static_cast<Float>(PPDMaxXen))
			note = static_cast<Float>(PPDMaxXen);
		
		const auto tmprmt = static_cast<Float>(temperaments[static_cast<int>(std::round(note))].load());

		return math::noteInFreqHz(note + tmprmt, static_cast<Float>(baseNote), static_cast<Float>(xen), static_cast<Float>(masterTune));
	}

	template<typename Float>
	Float XenManager::noteToFreqHzWithWrap(Float note, Float lowestFreq, Float highestFreq) const noexcept
	{
		auto freq = noteToFreqHz(note);
		while (freq < lowestFreq)
			freq *= static_cast<Float>(2);
		while (freq >= highestFreq)
			freq *= static_cast<Float>(.5);
		return freq;
	}

	template<typename Float>
	Float XenManager::freqHzToNote(Float hz) noexcept
	{
		return math::freqHzInNote(hz, static_cast<Float>(baseNote), static_cast<Float>(xen), static_cast<Float>(masterTune));
	}

	float XenManager::getXen() const noexcept
	{
		return xen;
	}

	template float XenManager::noteToFreqHz<float>(float note) const noexcept;
	template double XenManager::noteToFreqHz<double>(double note) const noexcept;

	template float XenManager::noteToFreqHzWithWrap<float>(float note, float lowestFreq, float highestFreq) const noexcept;
	template double XenManager::noteToFreqHzWithWrap<double>(double note, double lowestFreq, double highestFreq) const noexcept;

	template float XenManager::freqHzToNote<float>(float hz) noexcept;
	template double XenManager::freqHzToNote<double>(double hz) noexcept;
}