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
		using Complex = std::complex<double>;
		using ArrayD = std::array<double, Order>;
		using ArrayC = std::array<Complex, Order>;

		static constexpr ArrayC Coeffs =
		{
			Complex{-0.000224352093802, 0.00543499018201},
			Complex{0.0107500557815, -0.0173890685681},
			Complex{-0.0456795873917, 0.0229166931429},
			Complex{0.11282500582, 0.00278413661237},
			Complex{-0.208067578452, -0.104628958675},
			Complex{0.28717837501, 0.33619239719},
			Complex{-0.254675294431, -0.683033899655},
			Complex{0.0481081835026, 0.954061589374},
			Complex{0.227861357867, -0.891273574569},
			Complex{-0.365411839137, 0.525088317271},
			Complex{0.280729061131, -0.155131206606},
			Complex{-0.0935061787728, 0.00512245855404}
		};

		static constexpr ArrayC Poles =
		{
			Complex{-0.00495335976478, 0.0092579876872},
			Complex{-0.017859491302, 0.0273493725543},
			Complex{-0.0413714373155, 0.0744756910287},
			Complex{-0.0882148408885, 0.178349677457},
			Complex{-0.17922965812, 0.39601340223},
			Complex{-0.338261800753, 0.829229533354},
			Complex{-0.557688699732, 1.61298538328},
			Complex{-0.735157736148, 2.79987398682},
			Complex{-0.719057381172, 4.16396166128},
			Complex{-0.517871025209, 5.29724826804},
			Complex{-0.280197469471, 5.99598602388},
			Complex{-0.0852751354531, 6.3048492377}
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
				Complex coeff = Coeffs[i] * freqFactor * passbandGain;
				coeffsR[i] = coeff.real();
				coeffsI[i] = coeff.imag();
				Complex pole = std::exp(Poles[i] * freqFactor);
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

		Complex operator()(double x, int ch) noexcept
		{
			// Really we're just doing: state[i] = state[i]*poles[i] + x*coeffs[i]
			// but std::complex is slow without -ffast-math, so we've unwrapped it
			State state = states[ch], newState;
			for (auto i = 0; i < Order; ++i)
			{
				newState.real[i] = state.real[i] * polesR[i] - state.imag[i] * polesI[i] + x * coeffsR[i];
				newState.imag[i] = state.real[i] * polesI[i] + state.imag[i] * polesR[i] + x * coeffsI[i];
			}
			states[ch] = newState;

			auto resultR = x * direct;
			auto resultI = 0.;
			for (auto i = 0; i < Order; ++i)
			{
				resultR += newState.real[i];
				resultI += newState.imag[i];
			}
			return { resultR, resultI };
		}

		Complex operator()(float x, int ch) noexcept
		{
			return operator()(static_cast<double>(x), ch);
		}

		Complex operator()(Complex x, int ch) noexcept
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
			for (int i = 0; i < Order; ++i) {
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
		using Complex = std::complex<double>;

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
			shiftBefore(1.),
			shiftAfter(1.),
			shift(shiftHzToUnit(50.)),
			reflect(0)
		{
		}

		void prepare(double sampleRate) noexcept
		{
			sampleRateInv = 1. / sampleRate;
			hilbert.prepare(sampleRate);
		}

		void reset() noexcept
		{
			shiftPhaseBefore = shiftPhaseAfter = 0.;
			shiftBefore = shiftAfter = Complex(1., 0.);
			hilbert.reset();
		}

		void operator()(ProcessorBufferView& view) noexcept
		{
			const auto hz = unitToShiftHz(shift);
			const auto phaseStep = hz * sampleRateInv;
			const bool noReflectDown = (reflect == 0 || reflect == 2);
			const bool duplicateUp = (reflect == 2 || reflect == 3);
			for (auto i = 0; i < view.numSamples; ++i)
			{
				for (auto ch = 0; ch < view.getNumChannelsMain(); ++ch)
				{
					auto smpls = view.getSamplesMain(ch);
					const auto x = static_cast<double>(smpls[i]);
					// In general, this may alias at the high end when shifting up
					// but our Hilbert has a 20kHz lowpass, so that's enough
					// room for our maximum +1000Hz shift
					const Complex y = shiftAfter * hilbert(x * shiftBefore, ch);
					smpls[i] = (float)y.real();
				}
				const bool shiftInput = (phaseStep < 0.) ? noReflectDown : duplicateUp;
				if (shiftInput)
				{
					shiftPhaseBefore += phaseStep;
					shiftBefore = std::polar(1., shiftPhaseBefore * TauD);
				}
				else
				{
					shiftPhaseAfter += phaseStep;
					shiftAfter = std::polar(1., shiftPhaseAfter * TauD);
				}
			}
			shiftPhaseBefore -= std::floor(shiftPhaseBefore);
			shiftPhaseAfter -= std::floor(shiftPhaseAfter);
		}

	private:
		HilbertTransform hilbert;
		double shiftPhaseBefore, shiftPhaseAfter, sampleRateInv;
		Complex shiftBefore, shiftAfter;
		//
		double shift;
		int reflect;
	};
}