#pragma once
#include "PRM.h"
#include "ProcessorBufferView.h"

namespace dsp
{
	struct Gain
	{
		// defaultValueDb
		Gain(float);

		// sampleRate, smoothLengthMs
		void prepare(float, float) noexcept;

		// parameters:

		void setGain(float) noexcept;

		// process:

		// bufferView, numChannels, numSamples
		void operator()(BufferView2, int, int) noexcept;

		void operator()(ProcessorBufferView&) noexcept;

		// view
		void applyInverse(ProcessorBufferView&) noexcept;

		// smpls, numSamples
		void applyInverse(float*, int) noexcept;

	private:
		PRM gainPRM;
		PRMInfo gainInfo;
	};
}