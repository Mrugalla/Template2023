#include "EnvelopeFollower.h"

namespace dsp
{
	// Params

	EnvelopeFollower::Params::Params(float _gainDb, float _atkMs,
		float _dcyMs, float _smoothMs) :
		gainDb(_gainDb),
		sampleRate(1.),
		atkMs(_atkMs),
		dcyMs(_dcyMs),
		smoothMs(_smoothMs),
		gainPRM(0.f),
		atk(0.),
		dcy(0.),
		smooth(0.),
		gain(math::dbToAmp(_gainDb))
	{
	}

	void EnvelopeFollower::Params::prepare(double _sampleRate) noexcept
	{
		sampleRate = _sampleRate;
		setAtk(atkMs);
		setDcy(dcyMs);
		setSmooth(smoothMs);
		gainPRM.prepare(static_cast<float>(sampleRate), 4.f);
	}

	void EnvelopeFollower::Params::setGain(float db) noexcept
	{
		gainDb = db;
		gain = math::dbToAmp(gainDb);
	}

	void EnvelopeFollower::Params::setAtk(double ms) noexcept
	{
		atkMs = ms;
		atk = smooth::Lowpass::getXFromMs(atkMs, sampleRate);
	}

	void EnvelopeFollower::Params::setDcy(double ms) noexcept
	{
		dcyMs = ms;
		dcy = smooth::Lowpass::getXFromMs(dcyMs, sampleRate);
	}

	void EnvelopeFollower::Params::setSmooth(double ms) noexcept
	{
		smoothMs = ms;
		smooth = smooth::Lowpass::getXFromMs(smoothMs, sampleRate);
	}

	PRMInfo EnvelopeFollower::Params::getGain(int numSamples) noexcept
	{
		return gainPRM(gain, numSamples);
	}

	// EnvelopeFollower

	EnvelopeFollower::EnvelopeFollower() :
		params(),
		meter(0.f),
		buffer(),
		MinDb(math::dbToAmp(-60.)),
		envLP(0.),
		smooth(0.f),
		attackState(false)
	{
	}

	void EnvelopeFollower::prepare(double sampleRate) noexcept
	{
		params.prepare(sampleRate);
		reset(-120.);
	}

	void EnvelopeFollower::setGain(float db) noexcept
	{
		params.setGain(db);
	}

	void EnvelopeFollower::setAttack(double ms) noexcept
	{
		params.setAtk(ms);
	}

	void EnvelopeFollower::setDecay(double ms) noexcept
	{
		params.setDcy(ms);
	}

	void EnvelopeFollower::setSmooth(double ms) noexcept
	{
		params.setSmooth(ms);
		smooth.setX(params.smooth);
	}

	void EnvelopeFollower::operator()(ProcessorBufferView& view) noexcept
	{
		copyMid(view);
		operator()(buffer.data(), view.numSamples);
	}

	bool EnvelopeFollower::isSleepy() const noexcept
	{
		return !attackState && envLP.y1 < MinDb;
	}

	float EnvelopeFollower::operator[](int i) const noexcept
	{
		return buffer[i];
	}

	float EnvelopeFollower::getMeter() const noexcept
	{
		return meter.load();
	}

	void EnvelopeFollower::reset(double v)
	{
		const auto vF = static_cast<float>(v);
		meter.store(vF);
		smooth.reset(v);
		envLP.reset(v);
		attackState = false;
	}

	void EnvelopeFollower::operator()(float* smpls, int numSamples) noexcept
	{
		rectify(smpls, numSamples);
		applyGain(numSamples);
		synthesizeEnvelope(numSamples);
		smooth(buffer.data(), numSamples);
		processMeter(numSamples);
	}

	void EnvelopeFollower::copyMid(ProcessorBufferView& view) noexcept
	{
		auto envFolBuffer = buffer.data();
		const auto numChannels = view.getNumChannelsMain();
		const auto numSamples = view.numSamples;
		SIMD::copy(envFolBuffer, view.getSamplesMain(0), numSamples);
		if (numChannels == 1)
			return;
		SIMD::add(envFolBuffer, view.getSamplesMain(1), numSamples);
		SIMD::multiply(envFolBuffer, .5f, numSamples);
	}

	void EnvelopeFollower::rectify(float* smpls, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
			buffer[s] = std::abs(smpls[s]);
	}

	void EnvelopeFollower::applyGain(int numSamples) noexcept
	{
		const auto gainInfo = params.getGain(numSamples);
		auto data = buffer.data();
		if (gainInfo.smoothing)
			return SIMD::multiply(data, gainInfo.buf, numSamples);
		SIMD::multiply(data, params.gain, numSamples);
	}

	void EnvelopeFollower::synthesizeEnvelope(int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto s0 = envLP.y1;
			const auto s1 = static_cast<double>(buffer[s]);
			if (attackState)
				buffer[s] = static_cast<float>(processAttack(s0, s1));
			else
				buffer[s] = static_cast<float>(processDecay(s0, s1));
		}
	}

	double EnvelopeFollower::processAttack(double s0, double s1) noexcept
	{
		if (s0 <= s1)
			return envLP(s1);
		attackState = false;
		envLP.setX(params.dcy);
		return processDecay(s0, s1);
	}

	double EnvelopeFollower::processDecay(double s0, double s1) noexcept
	{
		if (s0 >= s1)
			return envLP(s1);
		attackState = true;
		envLP.setX(params.atk);
		return processAttack(s0, s1);
	}

	void EnvelopeFollower::processMeter(int numSamples) noexcept
	{
		auto begin = buffer.begin();
		const auto max = *std::max_element(begin, begin + numSamples);
		meter.store(max);
	}
}