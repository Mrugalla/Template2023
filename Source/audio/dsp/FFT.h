#pragma once
#include <complex>
#include "../Using.h"

namespace dsp
{
	class FFT
	{
		struct Complex
		{
			Complex(double _real = 0., double _imag = 0.) :
				real(_real),
				imag(_imag)
			{}

			Complex operator*(double val) const noexcept
			{
				return { real * val, imag };
			}

			void operator+=(const Complex& c) noexcept
			{
				real += c.real;
				imag += c.imag;
			}

			double getMagnitude() noexcept
			{
				return imag * imag + real * real;
			}

			double getPhase() noexcept
			{
				return std::atan2(imag, real);
			}

			double real, imag;
		};

	public:
		static constexpr int Order = 9;
		static constexpr int Size = 1 << Order;
		static constexpr int Size2 = Size * 2;
		static constexpr double Size2Inv = 1. / Size2;
		static constexpr double Size2InvTau = Size2Inv * Tau;
		using BufferD = std::array<double, Size>;
		using Buffer2Complex = std::array<Complex, Size2>;
	
		FFT() :
			fifo(),
			bins(),
			idx(0)
		{}

		void operator()(double* smpls, int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
				processSample(smpls[s]);
		}

		void processSample(double x) noexcept
		{
			fifo[idx] = x;
			if (++idx == Size)
			{
				idx = 0;
				applyFFT();
			}
		}

		void applyFFT() noexcept
		{
			for (int k = 0; k < Size2; ++k)
			{
				auto& bin = bins[k];
				bin = 0.;

				for (int n = 0; n < Size; ++n)
				{
					const auto smpl = fifo[n];
					const auto kn = static_cast<double>(k * n);
					const auto arg = kn * Size2InvTau;
					const auto real = std::cos(arg);
					const auto imag = -std::sin(arg);
					const Complex c(real, imag);
					bin += c * smpl;
				}
			}
		}

		BufferD fifo;
		Buffer2Complex bins;
		int idx;
	};

	inline void fftTest()
	{
		FFT fft;

		FFT::BufferD smpls;
		for (auto s = 0; s < FFT::Size; ++s)
			smpls[s] = std::sin(Tau * s / FFT::Size * 1000.);

		fft(smpls.data(), FFT::Size);

		for (auto& bin : fft.bins)
			DBG(bin.getMagnitude() << " :: " << bin.getPhase());
	}
}