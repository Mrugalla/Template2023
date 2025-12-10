#pragma once
#include <functional>
#include <array>
#include "../mtsesp/Client/libMTSClient.h"

namespace arch
{
	struct TuneSys
	{
		using Array = std::array<double, 128>;

		TuneSys();

		~TuneSys();

		const char* getScaleName() noexcept;

		bool hasMaster() noexcept;

		const bool wannaUpdate() const noexcept;

		template<typename Float>
		Float freqToNote(Float) noexcept;

		template<typename Float>
		Float noteToFreq(Float) noexcept;

		void operator()() noexcept;

		std::function<void(Array&)> update;
	private:
		MTSClient* mtsesp;
		Array freqBuf;
		bool mtsEnabled;
	};
}