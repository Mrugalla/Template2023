// read the license file of this header's folder, if you want to use this code!
#pragma once
#include <complex>
#include "../Using.h"

namespace dsp
{
	class FreqShifter
	{
		static constexpr int Order = 12;
		static constexpr double Direct = 0.000262057212648 * 2.;
		static constexpr double PassbandGain = 2.;
		using ArrayD = std::array<double, Order>;
		using ArrayC = std::array<ComplexD, Order>;

		static constexpr ArrayC Coeffs =
		{
			ComplexD{-0.000224352093802, 0.00543499018201},
			ComplexD{0.0107500557815, -0.0173890685681},
			ComplexD{-0.0456795873917, 0.0229166931429},
			ComplexD{0.11282500582, 0.00278413661237},
			ComplexD{-0.208067578452, -0.104628958675},
			ComplexD{0.28717837501, 0.33619239719},
			ComplexD{-0.254675294431, -0.683033899655},
			ComplexD{0.0481081835026, 0.954061589374},
			ComplexD{0.227861357867, -0.891273574569},
			ComplexD{-0.365411839137, 0.525088317271},
			ComplexD{0.280729061131, -0.155131206606},
			ComplexD{-0.0935061787728, 0.00512245855404}
		};

		static constexpr ArrayC Poles =
		{
			ComplexD{-0.00495335976478, 0.0092579876872},
			ComplexD{-0.017859491302, 0.0273493725543},
			ComplexD{-0.0413714373155, 0.0744756910287},
			ComplexD{-0.0882148408885, 0.178349677457},
			ComplexD{-0.17922965812, 0.39601340223},
			ComplexD{-0.338261800753, 0.829229533354},
			ComplexD{-0.557688699732, 1.61298538328},
			ComplexD{-0.735157736148, 2.79987398682},
			ComplexD{-0.719057381172, 4.16396166128},
			ComplexD{-0.517871025209, 5.29724826804},
			ComplexD{-0.280197469471, 5.99598602388},
			ComplexD{-0.0852751354531, 6.3048492377}
		};

		struct Coefficients
		{
			Coefficients() :
				r(),
				i(),
				pr(),
				pi()
			{ }

			ArrayD r, i, pr, pi;
		};

		struct HilbertTransform
		{
			HilbertTransform() :
				real(),
				imag()
			{
				reset();
			}

			void reset() noexcept
			{
				for (auto& v : real)
					v = 0.;
				for (auto& v : imag)
					v = 0.;
			}

			ComplexD operator()(const Coefficients& c, double x, double direct) noexcept
			{
				// Really we're just doing: state[i] = state[i]*poles[i] + x*coeffs[i]
				// but std::complex is slow without -ffast-math, so we've unwrapped it
				for (auto i = 0; i < Order; ++i)
				{
					const auto nReal = real[i] * c.pr[i] - imag[i] * c.pi[i] + x * c.r[i];
					const auto nImag = real[i] * c.pi[i] + imag[i] * c.pr[i] + x * c.i[i];
					real[i] = nReal;
					imag[i] = nImag;
				}
				auto resultR = x * direct;
				auto resultI = 0.;
				for (auto i = 0; i < Order; ++i)
				{
					resultR += real[i];
					resultI += imag[i];
				}
				return { resultR, resultI };
			}

			ComplexD operator()(const Coefficients& c, float x, double direct) noexcept
			{
				return operator()(c, static_cast<double>(x), direct);
			}

			ComplexD operator()(const Coefficients& c, ComplexD x, double direct) noexcept
			{
				for (int i = 0; i < Order; ++i)
				{
					const auto nReal = real[i] * c.pr[i] - imag[i] * c.pi[i] + x.real() * c.r[i] - x.imag() * c.i[i];
					const auto nImag = real[i] * c.pi[i] + imag[i] * c.pr[i] + x.real() * c.i[i] + x.imag() * c.r[i];
					real[i] = nReal;
					imag[i] = nImag;
				}
				auto resultR = x.real() * direct;
				auto resultI = x.imag() * direct;
				for (int i = 0; i < Order; ++i)
				{
					resultR += real[i];
					resultI += imag[i];
				}
				return { resultR, resultI };
			}
		private:
			ArrayD real, imag;
		};

		struct PhasorC
		{
			PhasorC(double _angle = 1., double _phase = 0.) noexcept :
				angle(_angle, 0.),
				phase(_phase)
			{
			}

			void operator()(double phaseStep) noexcept
			{
				phase += phaseStep;
				const auto a = phase * TauD;
				angle = std::polar(1., a);
				phase -= std::floor(phase);
			}

			ComplexD angle;
			double phase;
		};
	public:
		FreqShifter() :
			coeffs(),
			hilberts(),
			phasors(),
			direct(0.),
			sampleRateInv(1.),
			shift(13.),
			phaseStep(0.),
			reflect(false)
		{
		}

		void prepare(double sampleRate) noexcept
		{
			sampleRateInv = 1. / sampleRate;

			const auto freqFactor = std::min(0.46, 20000. * sampleRateInv);
			const auto ffGain = PassbandGain * freqFactor;
			direct = Direct * ffGain;
			for (int i = 0; i < Order; ++i)
			{
				ComplexD coeff = Coeffs[i] * ffGain;
				coeffs.r[i] = coeff.real();
				coeffs.i[i] = coeff.imag();
				ComplexD pole = std::exp(Poles[i] * freqFactor);
				coeffs.pr[i] = pole.real();
				coeffs.pi[i] = pole.imag();
			}
			setShift(shift);
			reset();
		}

		// parameters:

		void setReflect(int r) noexcept
		{
			reflect = r;
		}

		void setShift(double s) noexcept
		{
			shift = s;
			phaseStep = shift * sampleRateInv;
		}

		// process:

		void reset() noexcept
		{
			phasors[0].angle = phasors[1].angle = ComplexD(1., 0.);
			phasors[0].phase = phasors[1].phase = 0.;
			for(auto& hilbert: hilberts)
				hilbert.reset();
		}

		void operator()(ProcessorBufferView& view) noexcept
		{
			for (auto i = 0; i < view.numSamples; ++i)
			{
				for (auto ch = 0; ch < view.getNumChannelsMain(); ++ch)
				{
					auto smpls = view.getSamplesMain(ch);
					const auto x = static_cast<double>(smpls[i]);
					auto& hilbert = hilberts[ch];
					const auto hlbrt = hilbert(coeffs, x * phasors[0].angle, direct);
					const auto analyticSignal = phasors[1].angle * hlbrt;
					const auto y = static_cast<float>(analyticSignal.real());
					smpls[i] = y;
				}
				const auto idx = (phaseStep < 0.) ? reflect : 1 - reflect;
				phasors[idx](phaseStep);
			}
		}
	private:
		Coefficients coeffs;
		std::array<HilbertTransform, 2> hilberts;
		std::array<PhasorC, 2> phasors;
		double direct, sampleRateInv;
		//
		double shift, phaseStep;
		int reflect;
	};
}