#pragma once
#include "Comp.h"

namespace gui
{
	struct CompTexture :
		public Comp
	{
		// utils, data, size, gain, scale
		CompTexture(Utils&, const void*, int, float, int);

		void paint(Graphics&) override;

		void resized() override;
	private:
		const void* data;
		int size, scale;
		float gain;
		Image img;
	};
}