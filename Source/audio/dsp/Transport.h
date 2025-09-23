#pragma once
#include "Using.h"

namespace dsp
{
	struct Transport
	{
		struct PhaseInfo
		{
			double phase, inc;
		};

		struct Info
		{
			Info() :
				ppq(0.),
				bpm(1.),
				timeSecs(0.),
				numerator(3.),
				denominator(4.),
				timeSamples(0),
				numChannels(0),
				playing(true)
			{
			}

			double getBeatsPerSec() const noexcept
			{
				return bpm * SixtyInv;
			}

			PhaseInfo getPhaseInfo(double rateSync, double sampleRateInv) const noexcept
			{
				const auto d4 = denominator / 4.;
				const auto r = d4 / (rateSync * numerator);

				const auto beatsPerSecs = getBeatsPerSec();
				const auto inc = beatsPerSecs * sampleRateInv * r;

				const auto pos = ppq * r;

				return { pos, inc };
			}

			double getLengthSamples(double numQuarterNotes, double sampleRate) const noexcept
			{
				const auto beatsPerSecs = getBeatsPerSec();
				return sampleRate * numQuarterNotes / beatsPerSecs;
			}

			double ppq, bpm, timeSecs, numerator, denominator;
			Int64 timeSamples;
			int numChannels;
			bool playing;
		};

		Transport() :
			info(),
			callback([](const Info&) {}),
			sampleRateInv(0.)
		{
		}

		void prepare(double _sampleRateInv) noexcept
		{
			sampleRateInv = _sampleRateInv;
		}

		void operator()(const PlayHead* playHead, int numChannels) noexcept
		{
			if (juce::JUCEApplicationBase::isStandaloneApp() || playHead == nullptr)
				return;
			const auto phx = playHead->getPosition();
			if (!phx.hasValue())
				return;
			const auto& posInfo = *phx;
			update(posInfo, numChannels);
		}

		void operator()(int numSamples) noexcept
		{
			proceed(numSamples);
		}

		Info info;
		std::function<void(const Info&)> callback;
	private:
		double sampleRateInv;

		void update(const PlayHead::PositionInfo& phx, int numChannels) noexcept
		{
			info.numChannels = numChannels;

			bool wannaUpdate = false;

			const auto ppq = phx.getPpqPosition();
			const auto bpm = phx.getBpm();
			const auto timeSecs = phx.getTimeInSeconds();
			const auto timeSig = phx.getTimeSignature();
			const auto timeSamples = phx.getTimeInSamples();
			
			const auto ppqVal = ppq.hasValue() ? *ppq : 0.;
			const auto bpmVal = bpm.hasValue() ? *bpm : 90.;
			const auto timeSecsVal = timeSecs.hasValue() ? *timeSecs : 0.;
			const auto timeSamplesVal = timeSamples.hasValue() ? *timeSamples : 0;
			const auto playingVal = phx.getIsPlaying();

			info.ppq = ppqVal;
			info.timeSecs = timeSecsVal;
			info.timeSamples = timeSamplesVal;

			if (info.bpm != bpmVal)
			{
				info.bpm = bpmVal;
				wannaUpdate = true;
			}
			
			if (timeSig.hasValue())
			{
				const auto& ts = *timeSig;
				if (info.denominator != ts.denominator ||
					info.numerator != ts.numerator)
				{
					info.denominator = ts.denominator;
					info.numerator = ts.numerator;
					wannaUpdate = true;
				}
			}
			
			if (info.playing != playingVal)
			{
				info.playing = playingVal;
				wannaUpdate = true;
			}
			
			if (wannaUpdate)
				callback(info);
		}

		void proceed(int numSamples) noexcept
		{
			info.timeSamples += numSamples;
			info.timeSecs = static_cast<double>(info.timeSamples) * sampleRateInv;
			info.ppq += static_cast<double>(numSamples) * sampleRateInv * info.bpm * SixtyInv;
		}
	};
}

/*
todo:
wannaUpdate on ppq and irregular timeSamples
seperate callbacks for different event types
*/