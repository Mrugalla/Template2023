#include "WHead.h"

namespace dsp
{
	WHead::WHead() :
		buf(),
		wHead(0),
		delaySize(1)
	{}

	void WHead::prepare(int _delaySize) noexcept
	{
		delaySize = _delaySize;
		if (delaySize != 0)
			wHead = wHead % delaySize;
	}

	void WHead::operator()(int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s, wHead = (wHead + 1) % delaySize)
			buf[s] = wHead;
	}

	int WHead::operator[](int i) const noexcept
	{
		return buf[i];
	}

	const int* WHead::data() const noexcept
	{
		return buf.data();
	}

	int* WHead::data() noexcept
	{
		return buf.data();
	}

	void WHead::shift(int shift, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			buf[s] = buf[s] + shift;
			if (buf[s] > delaySize)
				buf[s] -= delaySize;
			else if (buf[s] < 0)
				buf[s] += delaySize;
		}
		wHead = buf[numSamples - 1];
	}
}