#include "RandomSeed.h"

namespace arch
{
	RandSeed::RandSeed(Props& _props, const String& _id, bool loadState) :
		user(_props),
		id(_id.removeCharacters(" ").toLowerCase()),
		mt(rd()),
		dist(0.f, 1.f),
		seed(0)
	{
		const auto oSeed = user.getIntValue(id, 0);
		if(!loadState || oSeed == 0)
			randomizeSeed();
		else
			setSeed(oSeed);
	}

	void RandSeed::randomizeSeed()
	{
		RandJUCE rand;
		setSeed(rand.nextInt());
	}

	void RandSeed::updateSeed(bool seedUp)
	{
		setSeed(seedUp ? seed + 1 : seed - 1);
	}

	void RandSeed::setSeed(int i)
	{
		seed = i;
		mt.seed(seed);
		user.setValue(id, seed);
	}

	float RandSeed::operator()()
	{
		return dist(mt);
	}
}