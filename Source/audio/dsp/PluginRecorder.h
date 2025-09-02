#pragma once
#include "ProcessorBufferView.h"

namespace dsp
{
	struct PluginRecorder
	{
		PluginRecorder();

		// sampleRate
		void prepare(float);

		void operator()(const ProcessorBufferView&) noexcept;

		const AudioBuffer& getRecording();
	private:
		AudioBuffer recording, outBuffer;
		int writeHead;

		// samples, numChannels, numSamples
		void normalize(float* const*, int, int) noexcept;
	};
}


