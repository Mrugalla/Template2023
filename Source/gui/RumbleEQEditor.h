#pragma once
#include "Ruler.h"
#include "Knob2.h"
#include "../audio/dsp/RumbleEQ.h"

namespace gui
{
	struct RumbleEQEditor :
		public Comp
	{
		using EQ = dsp::RumbleEQ;

		struct PIDs
		{
			PID lowCut, highCut,
				bell1Freq, bell1Q, bell1Gain,
				bell2Freq, bell2Q, bell2Gain,
				bell3Freq, bell3Q, bell3Gain;
		};

		struct FreqResponseView :
			public Comp
		{
			FreqResponseView(Utils& u) :
				Comp(u),
				magnitudeData(),
				freqResponseCurve()
			{
			}

			void paint(Graphics& g) override
			{
				setCol(g, CID::Darken);
				g.fillRect(getLocalBounds());
				setCol(g, CID::Txt);
				g.strokePath(freqResponseCurve, Stroke(utils.thicc));
			}

			void update(const EQ& eq)
			{
				const auto width = static_cast<float>(getWidth());
				const auto height = static_cast<float>(getHeight());
				eq.getMagnitudes(magnitudeData);
				auto& mags = magnitudeData();
				convertMagDbToY(mags.data(), height);
				updateFreqResponsePath(mags.data(), width);
				repaint();
			}
		private:
			PIDs pIDs;
			EQ::MagnitudeData magnitudeData;
			Path freqResponseCurve;

			void convertMagDbToY(double* mags, float height)
			{
				const auto minMag = static_cast<float>(math::ampToDecibel(*std::min_element(mags, mags + EQ::MagnitudeData::Resolution)));
				const auto maxMag = static_cast<float>(math::ampToDecibel(*std::max_element(mags, mags + EQ::MagnitudeData::Resolution)));
				const auto magRange = maxMag - minMag;
				const auto magRangeInv = 1.f / magRange;
				for (auto i = 0; i < EQ::MagnitudeData::Resolution; ++i)
				{
					const auto magDb = math::ampToDecibel(mags[i]);
					const auto magNorm = (magDb - minMag) * magRangeInv;
					const auto y = height - (magNorm * height);
					mags[i] = static_cast<double>(y);
				}
			}

			void updateFreqResponsePath(const double* mags, float width)
			{
				freqResponseCurve.clear();
				freqResponseCurve.startNewSubPath(0.f, static_cast<float>(mags[0]));
				for (auto x = utils.thicc; x < width; x += utils.thicc)
				{
					const auto xRatio = x / width;
					const auto iF = xRatio * static_cast<float>(EQ::MagnitudeData::Resolution - 1);
					const auto t = std::floor(iF);
					const auto i1 = static_cast<size_t>(t);
					const auto i0 = i1 > 0 ? i1 - 1 : 0;
					const auto i2 = i1 < EQ::MagnitudeData::Resolution - 1 ? i1 + 1 : EQ::MagnitudeData::Resolution - 1;
					const auto i3 = i2 < EQ::MagnitudeData::Resolution - 1 ? i2 + 1 : EQ::MagnitudeData::Resolution - 1;
					const auto y0 = static_cast<float>(mags[i0]);
					const auto y1 = static_cast<float>(mags[i1]);
					const auto y2 = static_cast<float>(mags[i2]);
					const auto y3 = static_cast<float>(mags[i3]);
					const auto frac = iF - t;
					const auto y = math::lagrange(y0, y1, y2, y3, frac);
					freqResponseCurve.lineTo(x, y);
				}
			}
		};

		struct Knot :
			public Knob
		{
			Knot(Utils& u, PID pFreq, PID pGain, PID pQ) :
				Knob(u)
			{
				makeKnot(*this, pFreq, pGain, pQ, "adjust the filter's frequency, gain and q with this knot.");
			}
		};

		RumbleEQEditor(Utils& u, EQ& _eq, const PIDs& pIDs) :
			Comp(u),
			eq(_eq),
			freqResponse(u),
			rulerFreq(u),
			bell1(u, pIDs.bell1Freq, pIDs.bell1Gain, pIDs.bell1Q),
			bell2(u, pIDs.bell2Freq, pIDs.bell2Gain, pIDs.bell2Q),
			bell3(u, pIDs.bell3Freq, pIDs.bell3Gain, pIDs.bell3Q)
		{
			layout.init
			(
				{ 1 },
				{ 8, 1 }
			);

			addAndMakeVisible(rulerFreq);
			addAndMakeVisible(freqResponse);
			addAndMakeVisible(bell1);
			addAndMakeVisible(bell2);
			addAndMakeVisible(bell3);

			rulerFreq.makeIncExpansionOfGF();
			const auto minPitch = math::freqHzToNote(dsp::EQMinFreq);
			const auto maxPitch = math::freqHzToNote(dsp::EQMaxFreq);
			rulerFreq.update(minPitch, maxPitch);
			rulerFreq.setValToStrFunc([](float val)
			{
				const auto freqHz = math::noteToFreqHz(val);
				if (freqHz >= 1000.f)
					return String(static_cast<int>(freqHz / 1000.f)) + "khz";
				else
					return String(static_cast<int>(freqHz)) + "hz";
			});

			add(Callback([&]()
			{
				const bool updated = eq.areParametersUpdated();
				if (!updated)
					return;
				eq.reportEditorUpdate(true);
				update();
			}, 0, TimerCallbacks::kFPS::k15, true));
		}

		void resized() override
		{
			layout.resized(getLocalBounds());
			layout.place(freqResponse, 0, 0, 1, 1);
			layout.place(rulerFreq, 0, 1, 1, 1);

			const auto thicc = utils.thicc;
			const auto knotW = static_cast<int>(thicc * 10.f);

			bell1.setBounds(0, 0, knotW, knotW);
			bell2.setBounds(knotW, 0, knotW, knotW);
			bell3.setBounds(knotW * 2, 0, knotW, knotW);
			update();
		}
	private:
		EQ& eq;
		FreqResponseView freqResponse;
		Ruler rulerFreq;
		Knot bell1, bell2, bell3;

		void update()
		{
			freqResponse.update(eq);
			rulerFreq.update();
			eq.reportEditorUpdate(false);
		}
	};
}