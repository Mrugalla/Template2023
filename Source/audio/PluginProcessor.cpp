#include "PluginProcessor.h"
#include "dsp/Distortion.h"

namespace audio
{
	PluginProcessor::PluginProcessor(Params& _params, const arch::XenManager& _xen) :
		params(_params),
		xen(_xen),
		sampleRate(1.)
	{
	}

	void PluginProcessor::prepare(double _sampleRate)
	{
		sampleRate = _sampleRate;
	}

	void PluginProcessor::operator()(double** samples, dsp::MidiBuffer&, int numChannels, int numSamples) noexcept
	{
		const auto slewPitch = params(PID::Slew).getValModDenorm();
		const auto slewHz = xen.noteToFreqHzWithWrap(slewPitch, 1.f);
		const auto slewRate = slew.freqHzToSlewRate(slewHz, sampleRate);
		const auto type = int(std::round(params(PID::FilterType).getValModDenorm()));
		slew(samples, slewRate, numChannels, numSamples, dsp::SlewLimiter::Type(type));
	}

	void PluginProcessor::processBlockBypassed(double**, dsp::MidiBuffer&, int, int) noexcept
	{}

	void PluginProcessor::savePatch()
	{}

	void PluginProcessor::loadPatch()
	{}
}