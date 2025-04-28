#pragma once
#include "juce_audio_processors/juce_audio_processors.h"
#include "../arch/XenManager.h"
#include "../arch/State.h"

namespace param
{
	using String = juce::String;
	
	String toID(const String&);

	enum class PID
	{
		// high level parameters
		Macro,
#if PPDHasSidechain
		Sidechain,
		SCGain,
#endif
#if PPDIsNonlinear
		GainIn,
		UnityGain,
#endif
#if PPDIO == PPDIODryWet
		GainDry,
		GainWet,
#elif PPDIO == PPDIOWetMix
		GainWet,
		Mix,
#if PPDHasDelta
		Delta,
#endif
#endif
		GainOut,
#if PPDHasStereoConfig
		StereoConfig,
#endif
#if PPDHasHQ
		HQ,
#endif
#if PPDHasLookahead
		Lookahead,
#endif
		// tuning parameters
#if PPDHasTuningEditor
		Xen,
		XenSnap,
		MasterTune,
		AnchorPitch,
		PitchbendRange,
		TempC, TempDb, TempD, TempEb, TempE, TempF, TempGb, TempG, TempAb, TempA, TempBb, TempB,
		TempC2, TempDb2, TempD2, TempEb2, TempE2, TempF2, TempGb2, TempG2, TempAb2, TempA2, TempBb2, TempB2,
		TempC3, TempDb3, TempD3, TempEb3, TempE3, TempF3, TempGb3, TempG3, TempAb3, TempA3, TempBb3, TempB3,
		TempC4, TempDb4, TempD4, TempEb4, TempE4, TempF4, TempGb4, TempG4, TempAb4, TempA4, TempBb4, TempB4,
#endif
		Power,

		// low level parameters
		//
		
		NumParams
	};
	static constexpr int NumParams = static_cast<int>(PID::NumParams);
	static constexpr int MinLowLevelIdx = static_cast<int>(PID::Power) + 1;
	static constexpr int NumLowLevelParams = NumParams - MinLowLevelIdx;

	// pID, offset
	PID ll(PID, int) noexcept;

	// pID, offset
	PID offset(PID, int) noexcept;

	String toString(PID);

	PID toPID(const String&);

	// pIDs, text, seperatorChr
	void toPIDs(std::vector<PID>&, const String&, const String&);

	String toTooltip(PID);

	enum class Unit
	{
		Power,
		Solo,
		Mute,
		Percent,
		Hz,
		Beats,
		Degree,
		Octaves,
		OctavesFloat,
		Semi,
		Fine,
		Ms,
		Decibel,
		Ratio,
		Polarity,
		StereoConfig,
		Voices,
		Pan,
		Xen,
		Note,
		Pitch,
		Q,
		Slope,
		Legato,
		Custom,
		FilterType,
		Vowel,
		NumUnits
	};

	using CharPtr = juce::CharPointer_UTF8;

	String toString(Unit);

	using ValToStrFunc = std::function<String(float)>;
	using StrToValFunc = std::function<float(const String&)>;

	using Range = juce::NormalisableRange<float>;
	using ParameterBase = juce::AudioProcessorParameter;
	
	using Xen = arch::XenManager&;
	using State = arch::State;

	class Param :
		public ParameterBase
	{
		static constexpr float BiasEps = .000001f;
	public:
		struct CB
		{
			float denorm() const noexcept
			{
				return param.range.convertFrom0to1(norm);
			}

			const Param& param;
			float norm;
			int numChannels;
		};
		using ParameterChangedCallback = std::function<void(CB)>;

		struct Mod
		{
			Mod();

			std::atomic<float> depth, bias;
		};

		// pID, range, valDenormDefault, valToStr, strToVal, unit
		Param(const PID, const Range&, const float,
			const ValToStrFunc&, const StrToValFunc&,
			const Unit = Unit::NumUnits);

		void prepare() noexcept;

		void savePatch(State&) const;

		void loadPatch(const State&);

		// called by host, normalized, thread-safe
		float getValue() const override;

		float getValueDenorm() const noexcept;

		// called by host, normalized, avoid locks, not used by editor
		void setValue(float) override;

		// setValue without the lock
		void setValueFromEditor(float) noexcept;

		// called by editor
		bool isInGesture() const noexcept;

		// norm
		void setValueWithGesture(float);

		void beginGesture();

		void endGesture();

		float getModDepth() const noexcept;

		void setModDepth(float) noexcept;

		// modSource
		float calcValModOf(float) const noexcept;

		float getValMod() const noexcept;

		float getValModDenorm() const noexcept;

		void setModBias(float) noexcept;

		float getModBias() const noexcept;

		void setModulationDefault() noexcept;

		// norm
		void setDefaultValue(float) noexcept;

		void startModulation() noexcept;

		void modulate(float) noexcept;

		// numChannels
		void endModulation(int) noexcept;

		float getDefaultValue() const override;

		String getName(int) const override;

		// units of param (hz, % etc.)
		String getLabel() const override;

		// string of norm val
		String getText(float /*norm*/, int) const override;

		// string to norm val
		float getValueForText(const String&) const override;

		// string to denorm val
		float getValForTextDenorm(const String&) const;

		String _toString();

		int getNumSteps() const override;

		bool isLocked() const noexcept;
		void setLocked(bool) noexcept;
		void switchLock() noexcept;

		void setModDepthAbsolute(bool) noexcept;

		// start, end, bias[0,1], x
		static float biased(float, float, float, float) noexcept;

		const PID id;
		const Range range;
		ParameterChangedCallback callback;
	protected:
		float valDenormDefault, valInternal, curValMod;
		Mod mod;
		std::atomic<float> valNorm, valMod;
		ValToStrFunc valToStr;
		StrToValFunc strToVal;
		Unit unit;

		std::atomic<bool> locked, inGesture;

		bool modDepthAbsolute;
	};

	class Params
	{
		using AudioProcessor = juce::AudioProcessor;
		using Parameters = std::vector<Param*>;
	public:
		Params(AudioProcessor&
#if PPDHasTuningEditor
			, const Xen&
#endif
		);

		void prepare() noexcept;

		void loadPatch(const State&);

		void savePatch(State&) const;

		// nameOrID
		int getParamIdx(const String&) const;

		size_t numParams() const noexcept;

		// modSrc, numChannels
		void modulate(float, int) noexcept;

		bool isModDepthAbsolute() const noexcept;
		void setModDepthAbsolute(bool) noexcept;
		void switchModDepthAbsolute() noexcept;

		Param* operator[](int) noexcept;
		const Param* operator[](int) const noexcept;
		Param* operator[](PID) noexcept;
		const Param* operator[](PID) const noexcept;

		Param& operator()(int) noexcept;
		const Param& operator()(int) const noexcept;
		Param& operator()(PID) noexcept;
		const Param& operator()(PID) const noexcept;

		Parameters& data() noexcept;
		const Parameters& data() const noexcept;
	protected:
		Parameters params;
		std::atomic<float> modDepthAbsolute;
	};
}