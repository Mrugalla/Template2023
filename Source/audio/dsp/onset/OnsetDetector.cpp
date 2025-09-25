#include "OnsetDetector.h"

namespace dsp
{
	// ONSET CORE:

	OnsetCore::OnsetCore() :
		reso(),
		envFols(),
		buffer(),
		sampleRate(1.),
		freqHz(5000.), bwHz(5000.), bwPercent(1.),
		attack(OnsetAtkDefault),
		decay(OnsetDcyDefault),
		gain(1.f)
	{
	}

	// parameters:

	void OnsetCore::setAttack(double a) noexcept
	{
		attack = a;
		const auto sampleRateInv = 1. / sampleRate;
		const auto waveLength = math::freqHzToSamples(freqHz, sampleRate);
		const auto ms = math::samplesToMs(waveLength, sampleRateInv);
		envFols[1].setAttack(ms * attack);
	}

	void OnsetCore::setDecay(double d, int i) noexcept
	{
		decay = d;
		const auto sampleRateInv = 1. / sampleRate;
		const auto waveLength = math::freqHzToSamples(freqHz, sampleRate);
		const auto ms = math::samplesToMs(waveLength, sampleRateInv);
		envFols[i].setDecay(ms * decay);
	}

	void OnsetCore::setBandwidth(double q) noexcept
	{
		bwHz = q;
		updateBandwidth();
	}

	void OnsetCore::setBandwidthPercent(double p) noexcept
	{
		bwPercent = p;
		updateBandwidth();
	}

	void OnsetCore::setGain(float g) noexcept
	{
		gain = g;
	}

	void OnsetCore::setFreqHz(double f) noexcept
	{
		freqHz = f;
		reso.setCutoffFc(math::freqHzToFc(freqHz, sampleRate));
	}

	void OnsetCore::updateFilter() noexcept
	{
		reso.update();
	}

	// process:

	void OnsetCore::prepare(double _sampleRate) noexcept
	{
		sampleRate = _sampleRate;
		for (auto& e : envFols)
			e.prepare(sampleRate);
		setFreqHz(freqHz);
		setBandwidth(bwHz);
		updateFilter();
		reso.reset();
		setAttack(attack);
		setDecay(decay, 0);
		setDecay(decay, 1);
	}

	void OnsetCore::copyFrom(OnsetBuffer& other, int numSamples) noexcept
	{
		buffer.copyFrom(other, numSamples);
	}

	void OnsetCore::resonate(int numSamples) noexcept
	{
		auto samples = buffer.getSamples();
		for (auto s = 0; s < numSamples; ++s)
			samples[s] = static_cast<float>(reso(samples[s]));
	}

	void OnsetCore::synthesizeEnvelopeFollowers(int numSamples) noexcept
	{
		const auto samples = buffer.getSamples();
		for (auto& e : envFols)
			e(samples, numSamples);
	}

	void OnsetCore::operator()(int numSamples) noexcept
	{
		const auto& e1 = envFols[0];
		const auto& e2 = envFols[1];
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto v0 = e1[s];
			const auto v1 = e2[s];
			const auto v2 = v1 + 1e-6f;
			oopsie(v2 == 0.f);
			const auto y = gain * v0 / v2;
			buffer[s] = y;
		}
	}

	void OnsetCore::addTo(OnsetBuffer& _buffer, int s) noexcept
	{
		const auto& e1 = envFols[0];
		const auto& e2 = envFols[1];
		const auto v0 = e1[s];
		const auto v1 = e2[s];
		const auto v2 = v1 + 1e-6f;
		oopsie(v2 == 0.f);
		const auto y = gain * v0 / v2;
		_buffer[s] += y;
	}

	float OnsetCore::processSample(OnsetBuffer& _buffer, int s) noexcept
	{
		const auto& e1 = envFols[0];
		const auto& e2 = envFols[1];
		const auto v0 = e1[s];
		const auto v1 = e2[s];
		const auto v2 = v1 + 1e-6f;
		oopsie(v2 == 0.f);
		const auto y = gain * v0 / v2;
		_buffer[s] = y;
		return y;
	}

	float OnsetCore::processSample(int s) noexcept
	{
		return processSample(buffer, s);
	}

	// getters:

	OnsetBuffer& OnsetCore::getBuffer() noexcept
	{
		return buffer;
	}

	float OnsetCore::getMaxMag(int numSamples) const noexcept
	{
		return buffer.getMaxMag(numSamples);
	}

	const float& OnsetCore::operator[](int i) const noexcept
	{
		return buffer[i];
	}

	void OnsetCore::updateBandwidth() noexcept
	{
		const auto b = bwHz * bwPercent;
		reso.setBandwidth(math::freqHzToFc(b, sampleRate));
	}

	// STRONG HOLD:

	OnsetStrongHold::OnsetStrongHold() :
		sampleRate(1.),
		lengthD(OnsetHoldDefault),
		timer(0),
		length(0)
	{
	}

	void OnsetStrongHold::prepare(double _sampleRate) noexcept
	{
		sampleRate = _sampleRate;
		setLength(lengthD);
	}

	void OnsetStrongHold::reset() noexcept
	{
		timer = 0;
	}

	void OnsetStrongHold::operator()(int numSamples) noexcept
	{
		timer += numSamples;
	}

	bool OnsetStrongHold::youShallNotPass() const noexcept
	{
		return timer < length;
	}

	bool OnsetStrongHold::youShallPass() const noexcept
	{
		return !youShallNotPass();
	}

	void OnsetStrongHold::setLength(double l) noexcept
	{
		lengthD = l;
		length = static_cast<int>(math::msToSamples(lengthD, sampleRate));
		timer = 0;
	}

	// ONSET DETECTOR:

	OnsetDetector::OnsetDetector() :
		buffer(),
		detectors(),
		strongHold(),
		sampleRate(1.),
		lowestPitch(math::freqHzToNote2(OnsetLowestFreqHz)),
		highestPitch(math::freqHzToNote2(OnsetHighestFreqHz)),
		threshold(OnsetThresholdDefault), tilt(OnsetTiltDefault),
		numBands(static_cast<int>(OnsetNumBandsDefault)), onset(-1), onsetOut(-1),
		sysex()
	{
		const auto bwPercentDefault = std::pow(2., static_cast<double>(OnsetBandwidthDefault));
		setBandwidth(bwPercentDefault);
		setDecay(OnsetDcyDefault);
		setTilt(OnsetTiltDefault);
		sysex.makeBytesOnset();
	}

	// parameters:

	void OnsetDetector::setAttack(double x) noexcept
	{
		for (auto& d : detectors)
			d.setAttack(x);
	}

	void OnsetDetector::setDecay(double x) noexcept
	{
		for (auto& d : detectors)
			d.setDecay(x, 1);
		auto d = OnsetDecay0Percent * x;
		for (auto& dtr : detectors)
			dtr.setDecay(d, 0);
	}

	void OnsetDetector::setTilt(float db) noexcept
	{
		tilt = db;
		updateTilt();
	}

	void OnsetDetector::setThreshold(float db) noexcept
	{
		threshold = math::dbToAmp(db);
	}

	void OnsetDetector::setHoldLength(double ms) noexcept
	{
		strongHold.setLength(ms);
	}

	void OnsetDetector::setBandwidth(double b) noexcept
	{
		for (auto& d : detectors)
			d.setBandwidthPercent(b);
	}

	void OnsetDetector::setNumBands(int n) noexcept
	{
		numBands = n;
		updatePitchRange();
		updateTilt();
	}

	void OnsetDetector::setLowestPitch(double p) noexcept
	{
		lowestPitch = p;
		updatePitchRange();
	}

	void OnsetDetector::setHighestPitch(double p) noexcept
	{
		highestPitch = p;
		updatePitchRange();
	}

	// process:

	void OnsetDetector::prepare(double _sampleRate) noexcept
	{
		sampleRate = _sampleRate;
		updatePitchRange();
		for (auto& d : detectors)
			d.prepare(sampleRate);
		strongHold.prepare(sampleRate);
	}

	void OnsetDetector::operator()(ProcessorBufferView& view) noexcept
	{
		onset = -1;
		strongHold(view.numSamples);
		buffer.copyFromMid(view);
		buffer.rectify(view.numSamples);
		for (auto i = 0; i < numBands; ++i)
		{
			auto& detector = detectors[i];
			detector.copyFrom(buffer, view.numSamples);
			detector.resonate(view.numSamples);
			detector.synthesizeEnvelopeFollowers(view.numSamples);
		}
		for (auto s = 0; s < view.numSamples; ++s)
		{
			auto val = 0.f;
#if PPDOnsetAlgo == PPDOnsetAlgoMax
			for (auto i = 0; i < numBands; ++i)
			{
				auto& detector = detectors[i];
				detector.processSample(s);
				if (val < detector[s])
					val = detector[s];
			}
#elif PPDOnsetAlgo == PPDOnsetAlgoSum
			for (auto i = 0; i < numBands; ++i)
			{
				auto& detector = detectors[i];
				detector.processSample(s);
				val += detector[s];
			}
			val = std::sqrt(val / static_cast<float>(numBands));
#endif
			if (val > threshold)
			{
				if (strongHold.youShallPass())
					onset = s;
				strongHold.reset();
			}
		}
	}

	void OnsetDetector::operator()(float* const* samples, MidiBuffer& midi,
		int numChannels, int numSamples) noexcept
	{
		onsetOut = -1;
		for (auto s = 0; s < numSamples; s += BlockSize)
		{
			const auto remainingSamples = numSamples - s;
			const auto numSamplesBlock = std::min(BlockSize, remainingSamples);
			float* block[] = { &samples[0][s], &samples[1][s] };
			ProcessorBufferView view;
			view.assignMain(block, numChannels, numSamplesBlock);
			operator()(view);
			if (onsetOut == -1 && onset != -1)
			{
				onsetOut = onset + s;
				midi.addEvent(sysex.midify(), onsetOut);
			}
		}
	}

	void OnsetDetector::updatePitchRange() noexcept
	{
		const auto rangePitch = highestPitch - lowestPitch;
		for (auto i = 0; i < numBands; ++i)
		{
			const auto iF = static_cast<float>(i);
			const auto iR = iF / static_cast<float>(numBands - 1);
			const auto pitch = lowestPitch + iR * rangePitch;
			const auto freqHz = static_cast<double>(math::noteToFreqHz2(pitch));
			const auto pitchLow = pitch - .5f;
			const auto pitchHigh = pitch + .5f;
			const auto freqLow = static_cast<double>(math::noteToFreqHz2(pitchLow));
			const auto freqHigh = static_cast<double>(math::noteToFreqHz2(pitchHigh));
			const auto bwHz = freqHigh - freqLow;
			auto& detector = detectors[i];
			detector.setFreqHz(freqHz);
			detector.setBandwidth(bwHz);
			detector.updateFilter();
		}
	}

	void OnsetDetector::updateTilt() noexcept
	{
		const auto lowestGain = math::dbToAmp(-tilt);
		const auto highestGain = math::dbToAmp(tilt);
		const auto rangeGain = highestGain - lowestGain;
		const auto numBandsInv = 1.f / static_cast<float>(numBands);
		const auto bandCompensate = numBandsInv * numBandsInv;
		for (auto i = 0; i < numBands; ++i)
		{
			const auto iF = static_cast<float>(i);
			const auto iR = iF / static_cast<float>(numBands);
			const auto gain = lowestGain + iR * rangeGain;
			detectors[i].setGain(gain * bandCompensate);
		}
	}
}