// read the license file of this header's folder, if you want to use this code!
#pragma once
#include <complex>
#include "../Using.h"

namespace dsp
{
	class HilbertTransform
	{
		static constexpr int Order = 12;
		static constexpr double Direct = 0.000262057212648;
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

		struct State
		{
			ArrayD real, imag;

			void reset() noexcept
			{
				for (auto& v : real)
					v = 0.;
				for (auto& v : imag)
					v = 0.;
			}
		};
	public:
		HilbertTransform() :
			coeffsR(),
			coeffsI(),
			polesR(),
			polesI(),
			states(),
			direct(0.),
			passbandGain(2.)
		{ }

		void prepare(double sampleRate) noexcept
		{
			const auto freqFactor = std::min(0.46, 20000. / sampleRate);
			direct = Direct * 2. * passbandGain * freqFactor;
			for (int i = 0; i < Order; ++i)
			{
				ComplexD coeff = Coeffs[i] * freqFactor * passbandGain;
				coeffsR[i] = coeff.real();
				coeffsI[i] = coeff.imag();
				ComplexD pole = std::exp(Poles[i] * freqFactor);
				polesR[i] = pole.real();
				polesI[i] = pole.imag();
			}
			reset();
		}

		void reset() noexcept
		{
			for (auto& state : states)
				state.reset();
		}

		ComplexD operator()(double x, int ch) noexcept
		{
			// Really we're just doing: state[i] = state[i]*poles[i] + x*coeffs[i]
			// but std::complex is slow without -ffast-math, so we've unwrapped it
			State& state = states[ch];
			State newState;
			for (auto i = 0; i < Order; ++i)
			{
				newState.real[i] = state.real[i] * polesR[i] - state.imag[i] * polesI[i] + x * coeffsR[i];
				newState.imag[i] = state.real[i] * polesI[i] + state.imag[i] * polesR[i] + x * coeffsI[i];
			}
			state = newState;

			auto resultR = x * direct;
			auto resultI = 0.;
			for (auto i = 0; i < Order; ++i)
			{
				resultR += newState.real[i];
				resultI += newState.imag[i];
			}
			return { resultR, resultI };
		}

		ComplexD operator()(float x, int ch) noexcept
		{
			return operator()(static_cast<double>(x), ch);
		}

		ComplexD operator()(ComplexD x, int ch) noexcept
		{
			State state = states[ch], newState;
			for (int i = 0; i < Order; ++i)
			{
				newState.real[i] = state.real[i] * polesR[i] - state.imag[i] * polesI[i] + x.real() * coeffsR[i] - x.imag() * coeffsI[i];
				newState.imag[i] = state.real[i] * polesI[i] + state.imag[i] * polesR[i] + x.real() * coeffsI[i] + x.imag() * coeffsR[i];
			}
			states[ch] = newState;

			auto resultR = x.real() * direct;
			auto resultI = x.imag() * direct;
			for (int i = 0; i < Order; ++i)
			{
				resultR += newState.real[i];
				resultI += newState.imag[i];
			}
			return { resultR, resultI };
		}
	private:
		ArrayD coeffsR, coeffsI, polesR, polesI;
		std::array<State, 2> states;
		double direct, passbandGain;
	};

	class FreqShifter
	{
		double unitToShiftHz(double x) noexcept
		{
			return 100. * x / (1.1 - x * x);
		}

		double shiftHzToUnit(double y) noexcept
		{
			return (std::sqrt(2500. + 1.1 * y * y) - 50.) / y;
		}
	public:
		FreqShifter() :
			hilbert(),
			shiftPhaseBefore(0.),
			shiftPhaseAfter(0.),
			sampleRateInv(1.),
			shiftBefore(1.),
			shiftAfter(1.),
			shift(50.),
			phaseStep(0.),
			reflect(false)
		{
		}

		void prepare(double sampleRate) noexcept
		{
			sampleRateInv = 1. / sampleRate;
			setShift(shift);
			hilbert.prepare(sampleRate);
		}

		// parameters:

		void setReflect(bool r) noexcept
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
			shiftPhaseBefore = shiftPhaseAfter = 0.;
			shiftBefore = shiftAfter = ComplexD(1., 0.);
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
					const auto analyticSignal = shiftAfter * hilbert(x * shiftBefore, ch);
					const auto y = static_cast<float>(analyticSignal.real());
					smpls[i] = y;
				}
				const bool shiftInput = (phaseStep < 0.) ? !reflect : reflect;
				if (shiftInput)
				{
					shiftPhaseBefore += phaseStep;
					const auto angle = shiftPhaseBefore * TauD;
					shiftBefore = std::polar(1., angle);
					shiftPhaseBefore -= std::floor(shiftPhaseBefore);
				}
				else
				{
					shiftPhaseAfter += phaseStep;
					const auto angle = shiftPhaseAfter * TauD;
					shiftAfter = std::polar(1., angle);
					shiftPhaseAfter -= std::floor(shiftPhaseAfter);
				}
			}
		}

	private:
		HilbertTransform hilbert;
		double shiftPhaseBefore, shiftPhaseAfter, sampleRateInv;
		ComplexD shiftBefore, shiftAfter;
		//
		double shift, phaseStep;
		bool reflect;
	};
}