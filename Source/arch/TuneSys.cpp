#include "TuneSys.h"

namespace arch
{
	TuneSys::TuneSys() :
		update([](Array&) {}),
		mtsesp(MTS_RegisterClient()),
		freqBuf(),
		mtsEnabled(false)
	{
		operator()(); // initial fill
	}

	TuneSys::~TuneSys()
	{
		MTS_DeregisterClient(mtsesp);
	}

	const char* TuneSys::getScaleName() noexcept
	{
		return MTS_GetScaleName(mtsesp);
	}

	bool TuneSys::hasMaster() noexcept
	{
		return MTS_HasMaster(mtsesp);
	}

	const bool TuneSys::wannaUpdate() const noexcept
	{
		return MTS_Client_ShouldUpdateLibrary(mtsesp);
	}

	template<typename Float>
	Float TuneSys::freqToNote(Float freqHz) noexcept
	{
		const auto fD = static_cast<double>(freqHz);
		return static_cast<Float>(MTS_FrequencyToNote(mtsesp, fD, -1)); // would be cool if this was not rounded
	}

	template<typename Float>
	Float TuneSys::noteToFreq(Float noteNumber) noexcept
	{
		const auto nnInt = static_cast<int>(std::round(noteNumber));
		const auto nnChar = static_cast<char>(nnInt);
		return static_cast<Float>(MTS_NoteToFrequency(mtsesp, nnChar, -1));
	}

	void TuneSys::operator()() noexcept
	{
		bool anyChange = false;
		for (auto i = 0; i < 128; ++i)
		{
			const auto noteNumber = static_cast<char>(i);
			const auto nFreqHz = MTS_NoteToFrequency(mtsesp, noteNumber, -1);
			if (nFreqHz != freqBuf[i])
			{
				freqBuf[i] = nFreqHz;
				anyChange = true;
			}
		}
		if (anyChange)
			update(freqBuf);
	}

	template float TuneSys::freqToNote<float>(float) noexcept;
	template double TuneSys::freqToNote<double>(double) noexcept;
	template float TuneSys::noteToFreq<float>(float) noexcept;
	template double TuneSys::noteToFreq<double>(double) noexcept;
}