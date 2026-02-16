#pragma once
#include <array>
#include "ProcessorBufferView.h"
#include "../../arch/RumbleAxiom.h"
#include <juce_dsp/juce_dsp.h>

namespace dsp
{
	struct RumbleFilter
	{
		using Filter = juce::dsp::IIR::Filter<float>;
		using ProcessContext = juce::dsp::ProcessContextReplacing<float>;
		using Coeffs = juce::dsp::IIR::Coefficients<float>;
		using ProcessDuplicator = juce::dsp::ProcessorDuplicator<Filter, Coeffs>;

		static constexpr int Slope = 2;

		enum class Type
		{
			LowCut, HighCut, Peak
		};

		struct Params
		{
			Params(float _f, float _q, float _g, Type t) :
				freqHz(_f),
				q(_q),
				gain(math::dbToAmp(_g)),
				type(t)
			{}

			float freqHz, q, gain;
			Type type;
		};

		RumbleFilter(Params&& p) :
			params(p),
			filters(),
			sampleRate(44100.),
			qThingie(1.f / std::sqrt(2.f))
		{
		}

		// parameters:
		void setFreq(float freqHz) noexcept
		{
			params.freqHz = freqHz;
			update();
		}

		void setGain(float gain) noexcept
		{
			params.gain = math::dbToAmp(gain);
			update();
		}

		void setQ(float q) noexcept
		{
			params.q = q;
			update();
		}

		// process:
		void prepare(const juce::dsp::ProcessSpec& spec) noexcept
		{
			sampleRate = spec.sampleRate;
			for (auto& filter : filters)
				filter.prepare(spec);
			update();
		}

		void operator()(ProcessContext& context) noexcept
		{
			for (auto& filter : filters)
				filter.process(context);
		}

		void getMagnitudes(const double* freqs, double* mags, size_t size, int slope) const noexcept
		{
			filters[slope].state->getMagnitudeForFrequencyArray(freqs, mags, size, sampleRate);
		}
	private:
		Params params;
		std::array<ProcessDuplicator, Slope> filters;
		double sampleRate;
		const float qThingie;

		void update() noexcept
		{
			switch (params.type)
			{
				case Type::LowCut:
					*filters[0].state = *juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass
					(
						sampleRate,
						params.freqHz
					);
					break;
				case Type::HighCut:
					*filters[0].state = *juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass
					(
						sampleRate,
						params.freqHz
					);
					break;
				case Type::Peak:
				default:
					*filters[0].state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter
					(
						sampleRate,
						params.freqHz,
						params.q * qThingie,
						params.gain
					);
					break;
			}
			filters[1].state = filters[0].state;
		}
	};

	struct RumbleEQ
	{
		static constexpr int NumFilters = 5;
		using Filter = RumbleFilter::Filter;
		using Type = RumbleFilter::Type;

		struct MagnitudeData
		{
			static constexpr size_t Resolution = 256;

			MagnitudeData() :
				freqs(),
				mags()
			{
				const auto minPitch = math::freqHzToNote(EQMinFreq);
				const auto maxPitch = math::freqHzToNote(EQMaxFreq);
				const auto pitchRange = maxPitch - minPitch;
				for (auto i = 0; i < Resolution; ++i)
				{
					const auto iF = static_cast<float>(i);
					const auto iRatio = iF / static_cast<float>(Resolution);
					const auto pitch = minPitch + iRatio * pitchRange;
					const auto freq = math::noteToFreqHz(pitch);
					freqs[i] = static_cast<double>(freq);
				}
			}

			std::array<double, Resolution>& operator()() noexcept
			{
				for (auto j = 0; j < Resolution; ++j)
				{
					auto y = 1.0;
					for (auto i = 0; i < NumFilters; ++i)
						for (auto s = 0; s < RumbleFilter::Slope; ++s)
							y *= mags[i][s][j];
					mags[0][0][j] = y;
				}
				return mags[0][0];
			}

			std::array<double, Resolution> freqs;
			std::array<std::array<std::array<double, Resolution>, RumbleFilter::Slope>, NumFilters> mags;
		};

		RumbleEQ() :
			filters
			{
				RumbleFilter({EQMinFreq, 1.f, 1.f, Type::LowCut}),
				RumbleFilter({80.f, 80.f, .2f, Type::Peak}),
				RumbleFilter({170.f, 80.f, -.3f, Type::Peak}),
				RumbleFilter({360.f, 80.f, .1f, Type::Peak}),
				RumbleFilter({EQMaxFreq, 1.f, 1.f, Type::HighCut})
			},
			parametersUpdated(false),
			editorUpdating(false)
		{
		}

		// parameters:
		void setLowCutFreq(float freqHz) noexcept
		{
			filters[0].setFreq(freqHz);
			parametersUpdated.store(true);
		}

		void setHighCutFreq(float freqHz) noexcept
		{
			filters[NumFilters - 1].setFreq(freqHz);
			parametersUpdated.store(true);
		}

		void setBellFreq(float freqHz, int idx) noexcept
		{
			filters[1 + idx].setFreq(freqHz);
			parametersUpdated.store(true);
		}

		void setBellGain(float gain, int idx) noexcept
		{
			filters[1 + idx].setGain(gain);
			parametersUpdated.store(true);
		}

		void setBellQ(float q, int idx) noexcept
		{
			filters[1 + idx].setQ(q);
			parametersUpdated.store(true);
		}

		// process:
		void prepare(double sampleRate) noexcept
		{
			juce::dsp::ProcessSpec spec;
			spec.sampleRate = sampleRate;
			spec.maximumBlockSize = dsp::BlockSize;
			spec.numChannels = 2;
			for (auto& f : filters)
				f.prepare(spec);
		}

		void operator()(ProcessorBufferView& view) noexcept
		{
			auto samples = view.getSamplesMain();
			const auto numChannels = static_cast<size_t>(view.getNumChannelsMain());
			const auto numSamples = static_cast<size_t>(view.numSamples);
			juce::dsp::AudioBlock<float> block(samples, numChannels, numSamples);
			auto context = juce::dsp::ProcessContextReplacing<float>(block);
			for (auto& f : filters)
				f(context);
		}

		// gui:
		void getMagnitudes(MagnitudeData& data) const noexcept
		{
			const auto freqs = data.freqs.data();
			for(auto i = 0; i < NumFilters; ++i)
			{
				auto& mags = data.mags[i];
				for (auto j = 0; j < RumbleFilter::Slope; ++j)
					filters[i].getMagnitudes(freqs, mags[j].data(), MagnitudeData::Resolution, j);
			}
		}

		bool areParametersUpdated() const noexcept
		{
			return parametersUpdated.load() && !editorUpdating.load();
		}

		void reportEditorUpdate(bool e) noexcept
		{
			editorUpdating.store(e);
			if(!e)
				parametersUpdated.store(false);
		}
	private:
		std::array<RumbleFilter, NumFilters> filters;
		std::atomic<bool> parametersUpdated, editorUpdating;
	};
}