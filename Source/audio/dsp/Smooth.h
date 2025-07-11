#pragma once
#include "Using.h"

namespace dsp
{
	namespace smooth
	{
		// a block-based parameter smoother.
		struct Block
		{
			// startVal
			Block(float = 0.f);

			// buffer, val, numSamples
			void operator()(float*, float, int) noexcept;

			// buffer, numSamples
			void operator()(float*, int) noexcept;

			float curVal;
		};

		struct Lowpass
		{
			// decay
			static double getXFromFc(double) noexcept;
			// decay, Fs
			static double getXFromSamples(double) noexcept;
			// decay, Fs
			static double getXFromHz(double, double) noexcept;
			// decay, Fs
			static double getXFromSecs(double, double) noexcept;
			// decay, Fs
			static double getXFromMs(double, double) noexcept;

			// decay
			void makeFromDecayInSamples(double) noexcept;
			// decay, Fs
			void makeFromDecayInSecs(double, double) noexcept;
			// decay, Fs
			void makeFromDecayInSecs(float, float) noexcept;
			// fc
			void makeFromDecayInFc(double) noexcept;
			// decay, Fs
			void makeFromDecayInHz(double, double) noexcept;
			// decay, Fs
			void makeFromDecayInMs(double, double) noexcept;
			// decay, Fs
			void makeFromDecayInMs(float, float) noexcept;

			void copyCutoffFrom(const Lowpass&) noexcept;

			// startVal
			Lowpass(double = 0.);

			// resets to startVal
			void reset();

			// value
			void reset(double);

			// buffer, val, numSamples
			void operator()(double*, double, int) noexcept;

			// buffer, numSamples
			void operator()(double*, int) noexcept;

			// buffer, numSamples
			void operator()(float*, int) noexcept;

			// val
			double operator()(double) noexcept;

			void setX(double) noexcept;

			double a0, b1, y1, startVal;

			double processSample(double) noexcept;

			double processSample(float) noexcept;
		};

		struct Smooth
		{
			// smoothLenMs, Fs
			void makeFromDecayInMs(float, float) noexcept;

			// freqHz, Fs
			void makeFromFreqInHz(float, float) noexcept;

			// startVal
			Smooth(float = 0.);

			void operator=(Smooth& other) noexcept
			{
				block.curVal = other.block.curVal;
				lowpass.copyCutoffFrom(other.lowpass);
				cur = other.cur;
				dest = other.dest;
				smoothing = other.smoothing;
			}

			/*
			// bufferOut, bufferIn, numSamples
			// currently not sure if this makes sense
			void operator()(float*, float*, int) noexcept;
			*/

			// buffer, val, numSamples
			bool operator()(float*, float, int) noexcept;

			// buffer, val, startIdx, endIdx
			bool operator()(float*, float, int, int) noexcept;

			// buffer, numSamples
			bool operator()(float*, int) noexcept;

			// value (this method is not for parameters!)
			float operator()(float) noexcept;

		protected:
			Block block;
			Lowpass lowpass;
			float cur, dest;
			bool smoothing;
		};
	}
}