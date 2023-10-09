#pragma once
#include "Using.h"

namespace gui
{
	struct TimerCallbacks :
		public Timer
	{
		using CB = std::function<void()>;
		enum class kFPS
		{
			k60,
			k30,
			k15,
			k7_5,
			k3_75,
			NumFPSs
		};
		static constexpr int NumFPSs = static_cast<int>(kFPS::NumFPSs);

		TimerCallbacks() :
			Timer(),
			callbacks(),
			idx(0)
		{
			startTimerHz(FPS);
		}

		void add(CB *cb, kFPS fps)
		{
			auto& cbs = callbacks[static_cast<int>(fps)];
			cbs.push_back(cb);
		}

		void remove(CB* cb)
		{
			for (auto i = 0; i < NumFPSs; ++i)
			{
				auto& cbs = callbacks[i];
				const auto it = std::find(cbs.begin(), cbs.end(), cb);
				if (it != cbs.end())
				{
					cbs.erase(it);
					return;
				}
			}
		}

	protected:
		using Callbacks = std::array<std::vector<CB*>, NumFPSs>;
		Callbacks callbacks;
		int idx;

		void timerCallback() override
		{
			++idx;

			for (auto i = 0; i < NumFPSs; ++i)
			{
				auto& cbs = callbacks[i];
				const auto fps = static_cast<kFPS>(i);
				const auto fpsOrder = 1 << static_cast<int>(fps);

				if (idx % fpsOrder == 0)
					for (auto cb : cbs)
						cb->operator()();
			}

			idx &= 15;
		}
	};

	using Callback = TimerCallbacks::CB;
	using cbFPS = TimerCallbacks::kFPS;
	using Callbacks = std::vector<Callback>;
}