#pragma once
#include "EventBuffer.h"

namespace dsp
{
	using BufferView2 = std::array<float*, 2>;
	using BufferView3 = std::array<float*, 3>;
	using BufferView4 = std::array<float*, 4>;

	struct BufferView2X
	{
		BufferView2X() :
			view(),
			numChannels(0)
		{
		}

		void assign(float* const* samples, int _numChannels) noexcept
		{
			for (auto ch = 0; ch < _numChannels; ++ch)
			{
				auto smpls = samples[ch];
				view[ch] = smpls;
			}
			numChannels = _numChannels;
		}

		void applyGain(float gain, int numSamples) noexcept
		{
			for (auto ch = 0; ch < numChannels; ++ch)
				SIMD::multiply(view[ch], gain, numSamples);
		}

		const float* operator[](int ch) const noexcept
		{
			return view[ch];
		}

		float* operator[](int ch) noexcept
		{
			return view[ch];
		}

		float* const* data() const noexcept 
		{
			return view.data();
		}

		BufferView2 view;
		int numChannels;
	};

	struct ProcessorBufferView
	{
		ProcessorBufferView() :
			main(), sc(),
			msg(),
			numSamples(0),
			scEnabled(false),
			hasMessage(false)
		{
		}

		void assignMain(float* const* samples,
			int numChannels, int _numSamples) noexcept
		{
			main.assign(samples, numChannels);
			numSamples = _numSamples;
			scEnabled = false;
		}

		void assignSC(float* const* samples, float scGain,
			int numChannels, bool listenSC) noexcept
		{
			sc.assign(samples, numChannels);
			if (scGain == 0.f)
			{
				scEnabled = false;
				return;
			}
			if (scGain != 1.f)
				sc.applyGain(scGain, numSamples);
			if (listenSC)
			{
				for (auto ch = 0; ch < main.numChannels; ++ch)
				{
					auto smplsMain = main.view[ch];
					auto chSC = ch;
					if (chSC >= sc.numChannels)
						chSC -= sc.numChannels;
					const auto smplsSC = sc.view[chSC];
					SIMD::copy(smplsMain, smplsSC, numSamples);
				}
			}
			scEnabled = !listenSC;
		}

		void useMainForSCIfRequired() noexcept
		{
			if (scEnabled)
				return;
			for (auto ch = 0; ch < main.numChannels; ++ch)
				sc.view[ch] = main.view[ch];
			sc.numChannels = main.numChannels;
		}

		void fillBlock(ProcessorBufferView& buffer, int s, int _numSamples) noexcept
		{
			numSamples = _numSamples;
			main.numChannels = buffer.main.numChannels;
			sc.numChannels = buffer.sc.numChannels;
			for (auto ch = 0; ch < main.numChannels; ++ch)
				main.view[ch] = &buffer.main.view[ch][s];
			for (auto ch = 0; ch < sc.numChannels; ++ch)
				sc.view[ch] = &buffer.sc.view[ch][s];
			scEnabled = buffer.scEnabled;
			hasMessage = false;
		}

		void fillBlock(ProcessorBufferView& buffer, int s) noexcept
		{
			const auto dif = buffer.numSamples - s;
			numSamples = dif < dsp::BlockSize ? dif : dsp::BlockSize;
			fillBlock(buffer, s, numSamples);
		}

		void fillBlock(ProcessorBufferView& buffer, const MidiMessage& _msg,
			int s, int _numSamples) noexcept
		{
			fillBlock(buffer, s, _numSamples);
			msg = _msg;
			hasMessage = true;
		}

		void clearMain() noexcept
		{
			for (auto ch = 0; ch < main.numChannels; ++ch)
				SIMD::clear(main.view[ch], numSamples);
		}

		BufferView2X getViewMain() noexcept
		{
			return main;
		}

		BufferView2X getViewSC() noexcept
		{
			return sc;
		}

		float* getSamplesMain(int ch) noexcept
		{
			return main.view[ch];
		}

		const float* getSamplesMain(int ch) const noexcept
		{
			return main.view[ch];
		}

		float* getSamplesSC(int ch) noexcept
		{
			return sc.view[ch];
		}

		const float* getSamplesSC(int ch) const noexcept
		{
			return sc.view[ch];
		}

		float* const* getSamplesMain() const noexcept
		{
			return main.data();
		}

		float* const* getSamplesSC() const noexcept
		{
			return sc.data();
		}

		int getNumChannelsMain() const noexcept
		{
			return main.numChannels;
		}

		int getNumChannelsSC() const noexcept
		{
			return sc.numChannels;
		}

		int getNumSamples() const noexcept
		{
			return numSamples;
		}

		BufferView2X main, sc;
		MidiMessage msg;
		int numSamples;
		bool scEnabled, hasMessage;
	};
}