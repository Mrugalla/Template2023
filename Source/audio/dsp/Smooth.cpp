#include "Smooth.h"

namespace dsp
{
	namespace smooth
	{
		// Block
		Block::Block(float startVal) :
			curVal(startVal)
		{
		}

		void Block::operator()(float* buffer, float dest, int numSamples) noexcept
		{
			const auto dist = dest - curVal;
			const auto inc = dist / static_cast<float>(numSamples);

			for (auto s = 0; s < numSamples; ++s)
			{
				buffer[s] = curVal;
				curVal += inc;
			}
		}

		void Block::operator()(float* buffer, int numSamples) noexcept
		{
			SIMD::fill(buffer, curVal, numSamples);
		}

		// Lowpass static

		double Lowpass::getXFromFc(double fc) noexcept
		{
			return std::exp(-TauD * fc);
		}

		double Lowpass::getXFromHz(double hz, double Fs) noexcept
		{
			return getXFromFc(hz / Fs);
		}

		double Lowpass::getXFromSamples(double lengthSamples) noexcept
		{
			const auto dInv = -1. / lengthSamples;
			const auto dExp = std::exp(dInv);
			return dExp;
		}

		double Lowpass::getXFromSecs(double secs, double Fs) noexcept
		{
			const auto lengthSamples = secs * Fs;
			return getXFromSamples(lengthSamples);
		}

		double Lowpass::getXFromMs(double ms, double Fs) noexcept
		{
			const auto secs = ms * .001;
			return getXFromSecs(secs, Fs);
		}

		// Lowpass

		void Lowpass::makeFromDecayInSamples(double d) noexcept
		{
			// decay in samples can not be negative, if it is you made a mistake before!
			// look it up, biatch!
			oopsie(d < 0.);
			if (d == 0.)
			{
				a0 = 1.;
				b1 = 0.;
				return;
			}
			const auto dInv = -1. / d;
			const auto dExp = std::exp(dInv);
			setX(dExp);
		}

		void Lowpass::makeFromDecayInFc(double fc) noexcept
		{
			setX(getXFromFc(fc));
		}

		void Lowpass::makeFromDecayInHz(double hz, double Fs) noexcept
		{
			setX(getXFromHz(hz, Fs));
		}

		void Lowpass::makeFromDecayInSecs(double d, double Fs) noexcept
		{
			makeFromDecayInSamples(d * Fs);
		}

		void Lowpass::makeFromDecayInSecs(float d, float Fs) noexcept
		{
			makeFromDecayInSecs(static_cast<double>(d), static_cast<double>(Fs));
		}

		void Lowpass::makeFromDecayInMs(double d, double Fs) noexcept
		{
			makeFromDecayInSecs(d * .001, Fs);
		}

		void Lowpass::makeFromDecayInMs(float d, float Fs) noexcept
		{
			makeFromDecayInMs(static_cast<double>(d), static_cast<double>(Fs));
		}

		void Lowpass::copyCutoffFrom(const Lowpass& other) noexcept
		{
			a0 = other.a0;
			b1 = other.b1;
		}

		Lowpass::Lowpass(double _startVal) :
			a0(1.),
			b1(0.),
			y1(_startVal),
			startVal(_startVal)
		{}

		void Lowpass::reset()
		{
			reset(startVal);
		}

		void Lowpass::reset(double v)
		{
			y1 = v;
		}

		void Lowpass::operator()(double* buffer, double val, int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
				buffer[s] = processSample(val);
		}

		void Lowpass::operator()(double* buffer, int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto y = processSample(buffer[s]);
				buffer[s] = y;
			}
		}

		void Lowpass::operator()(float* buffer, int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto y = processSample(buffer[s]);
				buffer[s] = static_cast<float>(y);
			}
		}

		double Lowpass::operator()(double sample) noexcept
		{
			return processSample(sample);
		}

		double Lowpass::processSample(double x0) noexcept
		{
			y1 = x0 * a0 + y1 * b1;
			return y1;
		}

		double Lowpass::processSample(float x0) noexcept
		{
			return processSample(static_cast<double>(x0));
		}

		void Lowpass::setX(double x) noexcept
		{
			a0 = 1. - x;
			b1 = x;
		}

		// Smooth

		void Smooth::makeFromDecayInMs(float smoothLenMs, float Fs) noexcept
		{
			lowpass.makeFromDecayInMs(static_cast<double>(smoothLenMs), static_cast<double>(Fs));
		}

		void Smooth::makeFromFreqInHz(float hz, float Fs) noexcept
		{
			lowpass.makeFromDecayInHz(static_cast<double>(hz), static_cast<double>(Fs));
		}

		Smooth::Smooth(float startVal) :
			block(startVal),
			lowpass(static_cast<double>(startVal)),
			cur(startVal),
			dest(startVal),
			smoothing(false)
		{
		}

		bool Smooth::operator()(float* bufferOut, float _dest, int numSamples) noexcept
		{
			dest = _dest;
			return operator()(bufferOut, numSamples);
		}

		bool Smooth::operator()(float* bufferOut, float _dest,
			int startIdx, int endIdx) noexcept
		{
			dest = _dest;
			auto numSamples = endIdx - startIdx;
			return operator()(&bufferOut[startIdx], numSamples);
		}

		bool Smooth::operator()(float* bufferOut, int numSamples) noexcept
		{
			if (!smoothing && cur == dest)
				return false;

			smoothing = true;

			block(bufferOut, dest, numSamples);
			lowpass(bufferOut, numSamples);

			cur = bufferOut[numSamples - 1];
			const auto eps = 1e-6f;
			const auto dist = dest - cur;
			const auto distSquared = dist * dist;
			if (distSquared < eps)
			{
				smoothing = false;
				cur = dest;
			}
			return smoothing;
		}

		float Smooth::operator()(float _dest) noexcept
		{
			return static_cast<float>(lowpass(_dest));
		}
	}
}