#include "TimerCallback.h"

namespace gui
{
	TimerCallbacks::TimerCallbacks() :
		Timer(),
		callbacks(),
		idx(0)
	{
		startTimerHz(FPS);
	}

	void TimerCallbacks::add(CB* cb, kFPS fps)
	{
		auto& cbs = callbacks[static_cast<int>(fps)];
		cbs.push_back(cb);
	}

	void TimerCallbacks::remove(CB* cb)
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

	void TimerCallbacks::timerCallback()
	{
		++idx;

		for (auto i = 0; i < NumFPSs; ++i)
		{
			auto& cbs = callbacks[i];
			const auto fps = static_cast<kFPS>(i);
			const auto fpsOrder = 1 << static_cast<int>(fps);

			if (idx % fpsOrder == 0)
				for (auto cb : cbs)
					cb->cb();
		}

		idx &= 15;
	}

	float secsToInc(float secs, cbFPS fps) noexcept
	{
		switch (fps)
		{
		case cbFPS::k30:
			return 1.f / (30.f * secs);
		case cbFPS::k15:
			return 1.f / (15.f * secs);
		case cbFPS::k7_5:
			return 1.f / (7.5f * secs);
		case cbFPS::k3_75:
			return 1.f / (3.75f * secs);
		default:
			return 1.f / (60.f * secs);
		}
	}

	float msToInc(float ms, cbFPS fps) noexcept
	{
		return secsToInc(ms * .001f, fps);
	}
}