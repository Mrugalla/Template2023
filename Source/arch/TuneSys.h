#pragma once
#include <array>
#include "Math.h"
#include "../mtsesp/Client/libMTSClient.h"

namespace arch
{
	struct TuneSys
	{
		using Array = std::array<double, 128>;

		TuneSys() :
			update([](Array&) {}),
			mtsesp(MTS_RegisterClient()),
			freqBuf(),
			mtsEnabled(false)
		{
			makeDefault();
		}

		~TuneSys()
		{
			MTS_DeregisterClient(mtsesp);
		}

		bool wannaUpdate() noexcept
		{
			return MTS_Client_ShouldUpdateLibrary(mtsesp);
		}

		void setMTSEnabled(bool e) noexcept
		{
			mtsEnabled = e;
			if (operator()())
				return;
			makeDefault();
			update(freqBuf);
		}

		template<typename Float>
		Float freqToNote(Float freqHz) noexcept
		{
			if (mtsEnabled)
			{
				const auto fD = static_cast<double>(freqHz);
				return static_cast<Float>(MTS_FrequencyToNote(mtsesp, fD, -1)); // would be cool if this was not rounded
			}
			return static_cast<Float>(math::freqHzToNote2(freqHz));
		}

		template<typename Float>
		Float noteToFreq(Float noteNumber) noexcept
		{
			if (mtsEnabled)
			{
				const auto nnInt = static_cast<int>(std::round(noteNumber));
				const auto nnChar = static_cast<char>(nnInt);
				return static_cast<Float>(MTS_NoteToFrequency(mtsesp, nnChar, -1));
			}
			return static_cast<Float>(math::noteToFreqHz2(noteNumber));
		}

		bool operator()() noexcept
		{
			if (mtsEnabled)
			{
				bool anyChange = false;
				for (auto i = 0; i < 128; ++i)
				{
					const auto iCharly = static_cast<char>(i);
					const auto nFreqHz = MTS_NoteToFrequency(mtsesp, iCharly, -1);
					if (nFreqHz != freqBuf[i])
					{
						freqBuf[i] = nFreqHz;
						anyChange = true;
					}
				}
				if (anyChange)
					update(freqBuf);
				return true;
			}
			return false;
		}

		std::function<void(Array&)> update;
	private:
		MTSClient* mtsesp;
		Array freqBuf;
		bool mtsEnabled;

		void makeDefault() noexcept
		{
			for (auto i = 0; i < 128; ++i)
				freqBuf[i] = math::noteToFreqHz2(static_cast<double>(i));
		}
	};
}