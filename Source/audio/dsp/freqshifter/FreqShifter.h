// read the license file of this header's folder, if you want to use this code!
#pragma once
#include "../Distortion.h"
#include "../ProcessorBufferView.h"

namespace dsp
{
	class FreqShifter
	{
		static constexpr int Order = 12;
		static constexpr double Direct = 0.000262057212648 * 2.;
		static constexpr double PassbandGain = 2.;
		using ArrayD = std::array<double, Order>;
		using ArrayC = std::array<ComplexD, Order>;
		using ComplexPair = std::array<ComplexD, 2>;

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
				imag(),
				direct(0.)
			{
				reset(0., 0.);
			}

			void prepare(double _direct) noexcept
			{
				direct = _direct;
			}

			void reset(double _real, double _imag) noexcept
			{
				for (auto& v : real)
					v = _real;
				for (auto& v : imag)
					v = _imag;
			}

			ComplexD operator()(const Coefficients& c, double x) noexcept
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

			ComplexD operator()(const Coefficients& c, float x) noexcept
			{
				return operator()(c, static_cast<double>(x));
			}

			ComplexD operator()(const Coefficients& c, ComplexD x) noexcept
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
			double direct;
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

		struct PhasorBuffer
		{
			PhasorBuffer() :
				buffers(),
				phasors{ PhasorC(1., 0.), PhasorC(1., 0.) },
				inc(0.),
				phaseOffset(0.),
				reflect(0)
			{
				const auto& pos = phasors[0].pos;
				for (auto& pairs : buffers)
					for (auto& pair : pairs)
						pair = pos;
			}

			// parameters:

			void setInc(double _inc) noexcept
			{
				inc = _inc;
			}

			void setReflect(int r) noexcept
			{
				reflect = r;
			}

			void setPhaseOffset(double phase) noexcept
			{
				phaseOffset = phase;
			}

			// process:

			void reset() noexcept
			{
				for (auto& phasor : phasors)
					phasor.reset(1., phaseOffset);
			}

			void operator()(int numSamples) noexcept
			{
				for(auto s = 0; s < numSamples; ++s)
				{
					buffers[s][0] = phasors[0].pos;
					buffers[s][1] = phasors[1].pos;
					const auto idx = (inc < 0.) ? reflect : 1 - reflect;
					phasors[idx](inc);
				}
			}

			const ComplexPair& operator[](int s) const noexcept
			{
				return buffers[s];
			}
		private:
			std::array<ComplexPair, BlockSize> buffers;
			std::array<PhasorC, 2> phasors;
			double inc, phaseOffset;
			int reflect;
		};

		struct FreqShifterMono
		{
			FreqShifterMono() :
				phasorBuffer(),
				hilbert(),
				phaseOffset(0.),
				y0(0.),
				feedback(0.),
				sampleRateInv(1.)
			{
			}

			void prepare(double _sampleRateInv, double direct) noexcept
			{
				sampleRateInv = _sampleRateInv;
				hilbert.prepare(direct);
			}

			// parameters:

			void setReflect(int r) noexcept
			{
				phasorBuffer.setReflect(r);
			}

			void setShift(double shift) noexcept
			{
				phasorBuffer.setInc(shift * sampleRateInv);
			}

			void setPhaseOffset(double p) noexcept
			{
				phaseOffset = p;
				phasorBuffer.setPhaseOffset(phaseOffset);
			}

			void setFeedback(double fb) noexcept
			{
				feedback = fb;
			}

			// process:

			void reset() noexcept
			{
				if (phaseOffset == 0.)
					return; // phase continuous mode!
				phasorBuffer.reset();
				hilbert.reset(0., 0.);
				y0 = 0.;
			}

			void operator()(float* smpls, const Coefficients& coeffs,
				int numSamples) noexcept
			{
				phasorBuffer(numSamples);
				for (auto s = 0; s < numSamples; ++s)
				{
					const auto& pair = phasorBuffer[s];
					const auto x = static_cast<double>(smpls[s]);
					const auto y1 = rationalclip(feedback * y0);
					const auto hlbrt = hilbert(coeffs, y1 + x * pair[0]);
					const auto analyticSignal = pair[1] * hlbrt;
					y0 = analyticSignal.real();
					smpls[s] = static_cast<float>(y0);
				}
			}
		private:
			PhasorBuffer phasorBuffer;
			HilbertTransform hilbert;
			double phaseOffset, y0, feedback, sampleRateInv;
		};
	public:
		FreqShifter() :
			coeffs(),
			shifters(),
			sampleRateInv(1.),
			bpm(90.),
			shiftHz(13.),
			shiftBeats(1.),
			shiftHzWidth(0.),
			shiftBeatsWidth(0.),
			phaseOffset(0.),
			phaseOffsetWidth(0.),
			feedback(0.),
			feedbackWidth(0.),
			temposync(false)
		{
		}

		void prepare(double sampleRate) noexcept
		{
			sampleRateInv = 1. / sampleRate;
			const auto freqFactor = std::min(0.46, 20000. * sampleRateInv);
			const auto ffGain = PassbandGain * freqFactor;
			coeffs.prepare(ffGain, freqFactor);
			const auto direct = Direct * ffGain;
			for(auto& shifter : shifters)
				shifter.prepare(sampleRateInv, direct);
			updateShift(2);
			reset();
		}

		// parameters:

		void setBpm(double b, int numChannels) noexcept
		{
			bpm = b;
			if(temposync)
				updateShift(numChannels);
		}

		void setReflect(int r) noexcept
		{
			for(auto& shifter: shifters)
				shifter.setReflect(r);
		}

		void setTemposync(bool t, int numChannels) noexcept
		{
			temposync = t;
			updateShift(numChannels);
		}

		void setShiftHz(double hz, int numChannels) noexcept
		{
			shiftHz = hz;
			updateShift(numChannels);
		}

		void setShiftBeats(double b, int numChannels) noexcept
		{
			shiftBeats = b;
			updateShift(numChannels);
		}

		void setPhaseOffset(double p, int numChannels) noexcept
		{
			phaseOffset = p;
			updatePhaseOffset(numChannels);
		}

		void setFeedback(double fb, int numChannels) noexcept
		{
			feedback = fb;
			updateFeedback(numChannels);
		}

		void setShiftHzWidth(double hz, int numChannels) noexcept
		{
			shiftHzWidth = hz;
			updateShift(numChannels);
		}

		void setShiftBeatsWidth(double b, int numChannels) noexcept
		{
			shiftBeatsWidth = b;
			updateShift(numChannels);
		}

		void setPhaseOffsetWidth(double p, int numChannels) noexcept
		{
			phaseOffsetWidth = p;
			updatePhaseOffset(numChannels);
		}

		void setFeedbackWidth(double fb, int numChannels) noexcept
		{
			feedbackWidth = fb;
			updateFeedback(numChannels);
		}

		// process:

		void reset() noexcept
		{
			for(auto& shifter: shifters)
				shifter.reset();
		}

		void operator()(ProcessorBufferView& view) noexcept
		{
			for (auto ch = 0; ch < view.getNumChannelsMain(); ++ch)
			{
				auto smpls = view.getSamplesMain(ch);
				auto& shifter = shifters[ch];
				shifter(smpls, coeffs, view.numSamples);
			}
		}
	private:
		Coefficients coeffs;
		std::array<FreqShifterMono, 2> shifters;
		double sampleRateInv, bpm, shiftHz, shiftBeats, shiftHzWidth, shiftBeatsWidth, phaseOffset, phaseOffsetWidth, feedback, feedbackWidth;
		bool temposync;

		void updateShift(int numChannels) noexcept
		{
			if (temposync)
			{
				const auto bps = bpm * SixtyInv;
				const auto shiftL = bps / shiftBeats;
				shifters[0].setShift(shiftL);
				if (numChannels == 2)
				{
					const auto shiftR = bps / (shiftBeats + shiftBeatsWidth);
					shifters[1].setShift(shiftR);
				}
				return;
			}
			const auto shift = shiftHz;
			shifters[0].setShift(shift - shiftHzWidth);
			if (numChannels == 2)
				shifters[1].setShift(shift + shiftHzWidth);
		}

		void updatePhaseOffset(int numChannels) noexcept
		{
			shifters[0].setPhaseOffset(phaseOffset - phaseOffsetWidth);
			if (numChannels == 2)
				shifters[1].setPhaseOffset(phaseOffset + phaseOffsetWidth);
		}

		void updateFeedback(int numChannels) noexcept
		{
			shifters[0].setFeedback(feedback - feedbackWidth);
			if (numChannels == 2)
				shifters[1].setFeedback(feedback + feedbackWidth);
		}
	};
}

/*
todo:
temposync: sync phasor with ppq?
find out if useful parameter to only reset phasor and not filter on reset
*/