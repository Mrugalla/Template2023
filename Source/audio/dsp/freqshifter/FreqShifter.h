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

			void prepare(double ffGain, double freqFactor) noexcept
			{
				for (int j = 0; j < Order; ++j)
				{
					ComplexD coeff = Coeffs[j] * ffGain;
					r[j] = coeff.real();
					i[j] = coeff.imag();
					ComplexD pole = std::exp(Poles[j] * freqFactor);
					pr[j] = pole.real();
					pi[j] = pole.imag();
				}
			}

			ArrayD r, i, pr, pi;
		};

		struct HilbertTransform
		{
			HilbertTransform() :
				real(),
				imag()
			{
				reset(0., 0.);
			}

			void reset(double _real, double _imag) noexcept
			{
				for (auto& v : real)
					v = _real;
				for (auto& v : imag)
					v = _imag;
			}

			ComplexD operator()(const Coefficients& c, double x, double direct) noexcept
			{
				// Really we're just doing: state[i] = state[i] * poles[i] + x * coeffs[i]
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
			PhasorC(double _pos = 1., double _phase = 0.) noexcept :
				pos(_pos, 0.),
				phase(_phase)
			{
			}

			void reset(double _pos, double _phase) noexcept
			{
				pos = ComplexD(_pos, 0.);
				phase = _phase;
			}

			void operator()(double inc) noexcept
			{
				phase += inc;
				const auto phaseTau = phase * TauD;
				pos = std::polar(1., phaseTau);
				phase -= std::floor(phase);
			}

			ComplexD pos;
			double phase;
		};
	public:
		FreqShifter() :
			coeffs(),
			hilberts(),
			phasors(),
			direct(0.),
			sampleRateInv(1.),
			y0(0.),
			shift(13.),
			inc(0.),
			phaseOffset(0.),
			feedback(0.),
			reflect(false)
		{
		}

		void prepare(double sampleRate) noexcept
		{
			sampleRateInv = 1. / sampleRate;
			const auto freqFactor = std::min(0.46, 20000. * sampleRateInv);
			const auto ffGain = PassbandGain * freqFactor;
			direct = Direct * ffGain;
			coeffs.prepare(ffGain, freqFactor);
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
			inc = shift * sampleRateInv;
		}

		void setPhaseOffset(double phase) noexcept
		{
			phaseOffset = phase;
		}

		// fb]-1, 1[
		void setFeedback(double fb) noexcept
		{
			feedback = fb * fb;
		}

		// process:

		void reset() noexcept
		{
			for(auto& phasor: phasors)
				phasor.reset(1., phaseOffset);
			for(auto& hilbert: hilberts)
				hilbert.reset(0., 0.);
			y0 = 0.;
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
					const auto y1 = feedback * y0;
					const auto hlbrt = hilbert(coeffs, y1 + x * phasors[0].pos, direct);
					const auto analyticSignal = phasors[1].pos * hlbrt;
					y0 = analyticSignal.real();
					smpls[i] = static_cast<float>(y0);
				}
				const auto idx = (inc < 0.) ? reflect : 1 - reflect;
				phasors[idx](inc);
			}
		}
	private:
		Coefficients coeffs;
		std::array<HilbertTransform, 2> hilberts;
		std::array<PhasorC, 2> phasors;
		double direct, sampleRateInv, y0;
		//
		double shift, inc, phaseOffset, feedback;
		int reflect;
	};
}