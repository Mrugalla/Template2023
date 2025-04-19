#include "Perlin.h"

namespace perlin
{
	void generateProceduralNoise(double* noise, int size, unsigned int seed)
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<float> dist(0.f, 1.f);

		for (auto s = 0; s < size; ++s, ++seed)
		{
			mt.seed(seed);
			noise[s] = dist(mt);
		}
	}

	double applyBias(double x, double bias) noexcept
	{
		if (bias == 0.)
			return x;
		const auto X = 2. * x * x * x * x * x;
		const auto Y = X * X * X;
		return x + bias * (std::tanh(Y) - x);
	}

	void applyBias(double* smpls, double bias, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
			smpls[s] = applyBias(smpls[s], bias);
	}

	void applyBias(double* const* samples, double bias, int numChannels, int numSamples) noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
			applyBias(samples[ch], bias, numSamples);
	}

	void applyBias(double* smpls, const double* bias, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
			smpls[s] = applyBias(smpls[s], bias[s]);
	}

	void applyBias(double* const* samples, const double* bias, int numChannels, int numSamples) noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
			applyBias(samples[ch], bias, numSamples);
	}

	double getInterpolatedNN(const double* noise, double phase) noexcept
	{
		return noise[static_cast<int>(std::round(phase)) + 1];
	}

	double getInterpolatedLerp(const double* noise, double phase) noexcept
	{
		return math::lerp(noise, phase + 1.5);
	}

	double getInterpolatedSpline(const double* noise, double phase) noexcept
	{
		return math::cubicHermiteSpline(noise, phase);
	}

	// PERLIN

	Perlin::Perlin() :
		// misc
		interpolationFuncs{ &getInterpolatedNN, &getInterpolatedLerp, &getInterpolatedSpline },
		sampleRateInv(1), sampleRate(1.),
		// phase
		phasor(),
		phaseBuffer(),
		noiseIdx(0)
	{
	}

	void Perlin::prepare(double _sampleRate) noexcept
	{
		sampleRate = _sampleRate;
		sampleRateInv = 1. / sampleRate;
	}

	void Perlin::updatePosition(double newPhase) noexcept
	{
		const auto newPhaseFloor = std::floor(newPhase);
		noiseIdx = static_cast<int>(newPhaseFloor) & NoiseSizeMax;
		phasor.phase.phase = newPhase - newPhaseFloor;
	}

	void Perlin::updateSpeed(double rateHzInv) noexcept
	{
		phasor.inc = rateHzInv;
	}

	void Perlin::operator()(double* const* samples, const double* noise, const double* gainBuffer,
		const PRMInfo& octavesInfo, const PRMInfo& phsInfo, const PRMInfo& widthInfo,
		Shape shape, int numChannels, int numSamples) noexcept
	{
		synthesizePhasor(phsInfo, numSamples);
		processOctaves(samples[0], octavesInfo, noise, gainBuffer, shape, numSamples);
		if (numChannels == 2)
			processWidth(samples, octavesInfo, widthInfo, noise, gainBuffer, shape, numSamples);
	}

	void Perlin::operator()(double* smpls, const double* noise, const double* gainBuffer,
		const PRMInfo& octavesInfo, Shape shape, int numSamples) noexcept
	{
		synthesizePhasor(numSamples);
		processOctaves(smpls, octavesInfo, noise, gainBuffer, shape, numSamples);
	}

	void Perlin::operator()(double* smpls, const double* noise, const double* gainBuffer,
		const PRMInfo& octavesInfo, double shape, int numSamples) noexcept
	{
		synthesizePhasor(numSamples);
		processOctaves(smpls, octavesInfo, noise, gainBuffer, shape, numSamples);
	}

	void Perlin::synthesizePhasor(const PRMInfo& phsInfo, int numSamples) noexcept
	{
		if (!phsInfo.smoothing)
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto phaseInfo = phasor();
				if (phaseInfo.retrig)
					noiseIdx = (noiseIdx + 1) & NoiseSizeMax;

				phaseBuffer[s] = phaseInfo.phase + phsInfo.val + static_cast<double>(noiseIdx);
			}
		else
			for (auto s = 0; s < numSamples; ++s)
			{
				const auto phaseInfo = phasor();
				if (phaseInfo.retrig)
					noiseIdx = (noiseIdx + 1) & NoiseSizeMax;

				phaseBuffer[s] = phaseInfo.phase + phsInfo[s] + static_cast<double>(noiseIdx);
			}
	}

	void Perlin::synthesizePhasor(int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto phaseInfo = phasor();
			if (phaseInfo.retrig)
				noiseIdx = (noiseIdx + 1) & NoiseSizeMax;

			phaseBuffer[s] = phaseInfo.phase + static_cast<double>(noiseIdx);
		}
	}

	double Perlin::getInterpolatedSample(const double* noise,
		double phase, Shape shape) const noexcept
	{
		const auto smpl0 = interpolationFuncs[static_cast<int>(shape)](noise, phase);
		return smpl0;
	}

	double Perlin::getInterpolatedSample(const double* noise,
		double phase, double shape) const noexcept
	{
		const auto smplNN = getInterpolatedNN(noise, phase);
		const auto smplCubic = getInterpolatedSpline(noise, phase);
		return smplNN + shape * (smplCubic - smplNN);
	}

	void Perlin::processOctaves(double* smpls, const PRMInfo& octavesInfo,
		const double* noise, const double* gainBuffer, Shape shape, int numSamples) noexcept
	{
		if (!octavesInfo.smoothing)
			processOctavesNotSmoothing(smpls, noise, gainBuffer, octavesInfo.val, shape, numSamples);
		else
			processOctavesSmoothing(smpls, octavesInfo.buf, noise, gainBuffer, shape, numSamples);
	}

	void Perlin::processOctaves(double* smpls, const PRMInfo& octavesInfo,
		const double* noise, const double* gainBuffer, double shape, int numSamples) noexcept
	{
		if (!octavesInfo.smoothing)
			processOctavesNotSmoothing(smpls, noise, gainBuffer, octavesInfo.val, shape, numSamples);
		else
			processOctavesSmoothing(smpls, octavesInfo.buf, noise, gainBuffer, shape, numSamples);
	}

	void Perlin::processOctavesNotSmoothing(double* smpls, const double* noise,
		const double* gainBuffer, double octaves,
		Shape shape, int numSamples) noexcept
	{
		const auto octFloor = std::floor(octaves);

		for (auto s = 0; s < numSamples; ++s)
		{
			auto sample = 0.;
			for (auto o = 0; o < octFloor; ++o)
			{
				const auto phase = getPhaseOctaved(phaseBuffer[s], o);
				const auto smpl = getInterpolatedSample(noise, phase, shape);
				sample += smpl * gainBuffer[o];
			}

			smpls[s] = sample;
		}

		auto gain = 0.;
		for (auto o = 0; o < octFloor; ++o)
			gain += gainBuffer[o];

		const auto octFrac = octaves - octFloor;
		if (octFrac != 0.)
		{
			const auto octFloorInt = static_cast<int>(octFloor);

			for (auto s = 0; s < numSamples; ++s)
			{
				const auto phase = getPhaseOctaved(phaseBuffer[s], octFloorInt);
				const auto smpl = getInterpolatedSample(noise, phase, shape);
				smpls[s] += octFrac * smpl * gainBuffer[octFloorInt];;
			}

			gain += octFrac * gainBuffer[octFloorInt];
		}

		SIMD::multiply(smpls, 1. / std::sqrt(gain), numSamples);
	}

	void Perlin::processOctavesNotSmoothing(double* smpls, const double* noise,
		const double* gainBuffer, double octaves,
		double shape, int numSamples) noexcept
	{
		const auto octFloor = std::floor(octaves);

		for (auto s = 0; s < numSamples; ++s)
		{
			auto sample = 0.;
			for (auto o = 0; o < octFloor; ++o)
			{
				const auto phase = getPhaseOctaved(phaseBuffer[s], o);
				const auto smpl = getInterpolatedSample(noise, phase, shape);
				sample += smpl * gainBuffer[o];
			}

			smpls[s] = sample;
		}

		auto gain = 0.;
		for (auto o = 0; o < octFloor; ++o)
			gain += gainBuffer[o];

		const auto octFrac = octaves - octFloor;
		if (octFrac != 0.)
		{
			const auto octFloorInt = static_cast<int>(octFloor);

			for (auto s = 0; s < numSamples; ++s)
			{
				const auto phase = getPhaseOctaved(phaseBuffer[s], octFloorInt);
				const auto smpl = getInterpolatedSample(noise, phase, shape);
				smpls[s] += octFrac * smpl * gainBuffer[octFloorInt];;
			}

			gain += octFrac * gainBuffer[octFloorInt];
		}

		SIMD::multiply(smpls, 1. / std::sqrt(gain), numSamples);
	}

	void Perlin::processOctavesSmoothing(double* smpls, const double* octavesBuf,
		const double* noise, const double* gainBuffer,
		Shape shape, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto octFloor = std::floor(octavesBuf[s]);

			auto sample = 0.;
			for (auto o = 0; o < octFloor; ++o)
			{
				const auto phase = getPhaseOctaved(phaseBuffer[s], o);
				const auto smpl = getInterpolatedSample(noise, phase, shape);
				sample += smpl * gainBuffer[o];
			}

			smpls[s] = sample;

			auto gain = 0.;
			for (auto o = 0; o < octFloor; ++o)
				gain += gainBuffer[o];

			const auto octFrac = octavesBuf[s] - octFloor;
			if (octFrac != 0.)
			{
				const auto octFloorInt = static_cast<int>(octFloor);

				const auto phase = getPhaseOctaved(phaseBuffer[s], octFloorInt);
				const auto smpl = getInterpolatedSample(noise, phase, shape);
				smpls[s] += octFrac * smpl * gainBuffer[octFloorInt];

				gain += octFrac * gainBuffer[octFloorInt];
			}

			smpls[s] /= std::sqrt(gain);
		}
	}

	void Perlin::processOctavesSmoothing(double* smpls, const double* octavesBuf,
		const double* noise, const double* gainBuffer,
		double shape, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto octFloor = std::floor(octavesBuf[s]);

			auto sample = 0.;
			for (auto o = 0; o < octFloor; ++o)
			{
				const auto phase = getPhaseOctaved(phaseBuffer[s], o);
				const auto smpl = getInterpolatedSample(noise, phase, shape);
				sample += smpl * gainBuffer[o];
			}

			smpls[s] = sample;

			auto gain = 0.;
			for (auto o = 0; o < octFloor; ++o)
				gain += gainBuffer[o];

			const auto octFrac = octavesBuf[s] - octFloor;
			if (octFrac != 0.)
			{
				const auto octFloorInt = static_cast<int>(octFloor);

				const auto phase = getPhaseOctaved(phaseBuffer[s], octFloorInt);
				const auto smpl = getInterpolatedSample(noise, phase, shape);
				smpls[s] += octFrac * smpl * gainBuffer[octFloorInt];

				gain += octFrac * gainBuffer[octFloorInt];
			}

			smpls[s] /= std::sqrt(gain);
		}
	}

	void Perlin::processWidth(double* const* samples, const PRMInfo& octavesInfo,
		const PRMInfo& widthInfo, const double* noise, const double* gainBuffer,
		Shape shape, int numSamples) noexcept
	{
		if (!widthInfo.smoothing)
			if (widthInfo.val == 0.)
				return SIMD::copy(samples[1], samples[0], numSamples);
			else
				SIMD::add(phaseBuffer.data(), widthInfo.val, numSamples);
		else
			SIMD::add(phaseBuffer.data(), widthInfo.buf, numSamples);

		processOctaves(samples[1], octavesInfo, noise, gainBuffer, shape, numSamples);
	}

	void Perlin::processWidth(double* const* samples, const PRMInfo& octavesInfo,
		const PRMInfo& widthInfo, const double* noise, const double* gainBuffer,
		double shape, int numSamples) noexcept
	{
		if (!widthInfo.smoothing)
			if (widthInfo.val == 0.)
				return SIMD::copy(samples[1], samples[0], numSamples);
			else
				SIMD::add(phaseBuffer.data(), widthInfo.val, numSamples);
		else
			SIMD::add(phaseBuffer.data(), widthInfo.buf, numSamples);

		processOctaves(samples[1], octavesInfo, noise, gainBuffer, shape, numSamples);
	}

	double Perlin::getPhaseOctaved(double phaseInfo, int o) const noexcept
	{
		const auto ox2 = 1 << o;
		const auto oPhase = phaseInfo * static_cast<double>(ox2);
		const auto oPhaseFloor = std::floor(oPhase);
		const auto oPhaseInt = static_cast<int>(oPhaseFloor) & NoiseSizeMax;
		return oPhase - oPhaseFloor + static_cast<double>(oPhaseInt);
	}

	// debug:
#if JUCE_DEBUG
	void discontinuityJassert(double* smpls, int numSamples, double threshold)
	{
		auto lastSample = smpls[0];
		for (auto s = 1; s < numSamples; ++s)
		{
			auto curSample = smpls[s];
			oopsie(std::abs(curSample - lastSample) > threshold);
			lastSample = curSample;
		}
	}

	void controlRange(double* const* samples, int numChannels, int numSamples) noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
			for (auto s = 0; s < numSamples; ++s)
			{
				oopsie(std::abs(samples[ch][s]) >= 1.);
				oopsie(std::abs(samples[ch][s]) <= -1.);
			}
	}
#endif

	// PERLIN2

	Perlin2::Perlin2() :
		mixer(),
		// misc
		sampleRateInv(1.),
		// perlin / noise
		noise(),
		gainBuffer(),
		gainBufferOct(),
		perlins(),
		// parameters
		octavesPRM(1.),
		widthPRM(0.),
		phsPRM(0.),
		rateBeats(1.),
		rateHz(1.),
		inc(1.),
		bpm(1.), bps(1.),
		// noise seed
		seed(),
		// project position
		posEstimate(-1)
	{
		setSeed(420);
		for (auto s = 0; s < NoiseOvershoot; ++s)
			noise[NoiseSize + s] = noise[s];
		for (auto o = 0; o < gainBuffer.size(); ++o)
			gainBuffer[o] = 1. / static_cast<double>(1 << o);
		for (auto o = 0; o < gainBufferOct.size(); ++o)
		{
			auto g = 0.;
			for (auto i = 0; i < o; ++i)
				g += gainBuffer[i];
			gainBufferOct[o] = std::sqrt(1. / g);
		}
	}

	void Perlin2::setSeed(int _seed)
	{
		seed.store(_seed);
		generateProceduralNoise(noise.data(), NoiseSize, static_cast<unsigned int>(_seed));
	}

	void Perlin2::prepare(double fs)
	{
		sampleRateInv = 1. / fs;
		mixer.prepare(fs, XFadeLengthMs);
		for (auto& perlin : perlins)
			perlin.prepare(fs);
		octavesPRM.prepare(fs, 10.);
		widthPRM.prepare(fs, 20.);
		phsPRM.prepare(fs, 20.);
	}

	void Perlin2::operator()(double** samples, int numChannels, int numSamples,
		const Transport& transport,
		double _rateHz, double _rateBeats,
		double octaves, double width, double phs, double bias,
		Shape shape, bool temposync) noexcept
	{
		const auto octavesInfo = octavesPRM(octaves, numSamples);
		const auto phsInfo = phsPRM(phs, numSamples);
		const auto widthInfo = widthPRM(width, numSamples);

		updatePerlin(transport, _rateBeats, _rateHz, numSamples, temposync);

		{
			auto& track = mixer[0];

			if (track.isEnabled())
			{
				auto xBuffer = mixer.getBuffer(0);
				double* xSamples[3] = { xBuffer[0], xBuffer[1], xBuffer[2] };
				track.synthesizeGainValues(xSamples[2], numSamples);

				perlins[0]
				(
					xSamples,
					noise.data(),
					gainBuffer.data(),
					octavesInfo,
					phsInfo,
					widthInfo,
					shape,
					numChannels,
					numSamples
					);

				track.copy(samples, xSamples, numChannels, numSamples);
			}
			else
				for (auto ch = 0; ch < numChannels; ++ch)
					SIMD::clear(samples[ch], numSamples);

		}

		for (auto i = 1; i < NumPerlins; ++i)
		{
			auto& track = mixer[i];

			if (track.isEnabled())
			{
				auto xBuffer = mixer.getBuffer(0);
				double* xSamples[3] = { xBuffer[0], xBuffer[1], xBuffer[2] };
				track.synthesizeGainValues(xSamples[2], numSamples);

				perlins[i]
					(
						xSamples,
						noise.data(),
						gainBuffer.data(),
						octavesInfo,
						phsInfo,
						widthInfo,
						shape,
						numChannels,
						numSamples
						);

					track.add(samples, xSamples, numChannels, numSamples);
			}
		}

		if (bias != 0.)
			processBias(samples, bias, numChannels, numSamples);

		fuckingApplyGainMate(samples, octaves, numChannels, numSamples);
	}

	void Perlin2::operator()(double* smpls, int numSamples,
		const Transport& transport,
		double _rateBeats, double octaves, double bias,
		Shape shape) noexcept
	{
		const auto octavesInfo = octavesPRM(octaves, numSamples);

		updatePerlin(transport, _rateBeats, numSamples);

		{
			auto& track = mixer[0];

			if (track.isEnabled())
			{
				auto xBuffer = mixer.getBuffer(0);
				double* xSamples[3] = { xBuffer[0], xBuffer[1], xBuffer[2] };
				track.synthesizeGainValues(xSamples[2], numSamples);

				perlins[0]
				(
					*xSamples,
					noise.data(),
					gainBuffer.data(),
					octavesInfo,
					shape,
					numSamples
					);

				track.copy(smpls, xSamples, numSamples);
			}
			else
				SIMD::clear(smpls, numSamples);

		}

		for (auto i = 1; i < NumPerlins; ++i)
		{
			auto& track = mixer[i];

			if (track.isEnabled())
			{
				auto xBuffer = mixer.getBuffer(0);
				double* xSamples[3] = { xBuffer[0], xBuffer[1], xBuffer[2] };
				track.synthesizeGainValues(xSamples[2], numSamples);

				perlins[i]
					(
						*xSamples,
						noise.data(),
						gainBuffer.data(),
						octavesInfo,
						shape,
						numSamples
						);

					track.add(smpls, xSamples, numSamples);
			}
		}

		if (bias != 0.)
			processBias(smpls, bias, numSamples);

		fuckingApplyGainMate(smpls, octaves, numSamples);
	}

	void Perlin2::operator()(double* smpls, int numSamples,
		const Transport& transport,
		double _rateBeats, double octaves, double bias,
		double shape) noexcept
	{
		const auto octavesInfo = octavesPRM(octaves, numSamples);

		updatePerlin(transport, _rateBeats, numSamples);

		{
			auto& track = mixer[0];

			if (track.isEnabled())
			{
				auto xBuffer = mixer.getBuffer(0);
				double* xSamples[3] = { xBuffer[0], xBuffer[1], xBuffer[2] };
				track.synthesizeGainValues(xSamples[2], numSamples);

				perlins[0]
				(
					*xSamples,
					noise.data(),
					gainBuffer.data(),
					octavesInfo,
					shape,
					numSamples
					);

				track.copy(smpls, xSamples, numSamples);
			}
			else
				SIMD::clear(smpls, numSamples);

		}

		for (auto i = 1; i < NumPerlins; ++i)
		{
			auto& track = mixer[i];

			if (track.isEnabled())
			{
				auto xBuffer = mixer.getBuffer(0);
				double* xSamples[3] = { xBuffer[0], xBuffer[1], xBuffer[2] };
				track.synthesizeGainValues(xSamples[2], numSamples);

				perlins[i]
					(
						*xSamples,
						noise.data(),
						gainBuffer.data(),
						octavesInfo,
						shape,
						numSamples
						);

					track.add(smpls, xSamples, numSamples);
			}
		}

		if (bias != 0.)
			processBias(smpls, bias, numSamples);

		fuckingApplyGainMate(smpls, octaves, numSamples);
	}

	void Perlin2::updatePerlin(const Transport& transport,
		double _rateBeats, double _rateHz, int numSamples, bool temposync) noexcept
	{
		updateSpeed(transport.bpm, _rateHz, _rateBeats, transport.timeSamples, temposync);

		if (transport.playing)
		{
			updatePosition(perlins[mixer.idx], transport.ppq, transport.timeSecs, temposync);
			posEstimate = transport.timeSamples + numSamples;
		}
		else
			posEstimate = transport.timeSamples;
	}

	void Perlin2::updatePerlin(const Transport& transport,
		double _rateBeats, int numSamples) noexcept
	{
		updateSpeed(transport.bpm, _rateBeats, transport.timeSamples);

		if (transport.playing)
		{
			updatePositionSync(perlins[mixer.idx], transport.ppq);
			posEstimate = transport.timeSamples + numSamples;
		}
		else
			posEstimate = transport.timeSamples;
	}

	void Perlin2::updateSpeed(double nBpm, double nRateHz, double nRateBeats,
		Int64 timeInSamples, bool temposync) noexcept
	{
		double nBps = nBpm / 60.;
		const auto nRateInv = .25 * nRateBeats;

		double nInc = 0.;
		if (temposync)
		{
			const auto bpSamples = nBps * sampleRateInv;
			nInc = nRateInv * bpSamples;
		}
		else
			nInc = nRateHz * sampleRateInv;

		if (isLooping(timeInSamples) || (changesSpeed(nBpm, nInc) && !mixer.stillFading()))
			initXFade(nInc, nBpm, nBps, nRateInv, nRateHz, nRateBeats);
	}

	void Perlin2::updateSpeed(double nBpm, double nRateBeats, Int64 timeInSamples) noexcept
	{
		double nBps = nBpm / 60.;
		const auto nRateInv = .25 * nRateBeats;

		const auto bpSamples = nBps * sampleRateInv;
		const auto nInc = nRateInv * bpSamples;

		if (isLooping(timeInSamples) || (changesSpeed(nBpm, nInc) && !mixer.stillFading()))
			initXFade(nInc, nBpm, nBps, nRateInv, 0., nRateBeats);
	}

	void Perlin2::updatePosition(Perlin& perlin, double ppqPosition,
		double timeInSecs, bool temposync) noexcept
	{
		if (temposync)
			updatePositionSync(perlin, ppqPosition);
		else
		{
			const auto nPhase = timeInSecs * rateHz;
			perlin.updatePosition(nPhase);
		}
	}

	void Perlin2::updatePositionSync(Perlin& perlin, double ppqPosition) noexcept
	{
		const auto ppq = ppqPosition - (bps * sampleRateInv);
		const auto nPhase = ppq * rateInv + .5;
		perlin.updatePosition(nPhase);
	}

	// CROSSFADE FUNCS
	bool Perlin2::isLooping(Int64 timeInSamples) noexcept
	{
		const auto error = std::abs(timeInSamples - posEstimate);
		return error > 1;
	}

	const bool Perlin2::keepsSpeed(double nBpm, double nInc) const noexcept
	{
		return nInc == inc && bpm == nBpm;
	}

	const bool Perlin2::changesSpeed(double nBpm, double nInc) const noexcept
	{
		return !keepsSpeed(nBpm, nInc);
	}

	void Perlin2::initXFade(double nInc, double nBpm, double nBps,
		double nRateInv, double _rateHz, double _rateBeats) noexcept
	{
		inc = nInc;
		bpm = nBpm;
		bps = nBps;
		rateInv = nRateInv;
		rateHz = _rateHz;
		rateBeats = _rateBeats;
		mixer.init();
		perlins[mixer.idx].updateSpeed(inc);
	}

	void Perlin2::processBias(double* const* samples, double bias,
		int numChannels, int numSamples) noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
			processBias(samples[ch], bias, numSamples);
	}

	void Perlin2::processBias(double* smpls, double bias,
		int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
		{
			const auto x = smpls[s];
			const auto a = std::abs(2. * (x - .5));
			const auto b = (1. + bias * 12.) * Pi;
			const auto c = b * a;
			if (c == 0.)
				smpls[s] = 1.;
			else
			{
				const auto d = std::sin(c) / c;
				const auto e = d * d;
				smpls[s] = x + bias * (e - x);
			}
		}
	}

	void Perlin2::fuckingApplyGainMate(double** samples, double octaves,
		int numChannels, int numSamples) noexcept
	{
		const auto oFloor = std::floor(octaves);
		const auto o0 = static_cast<int>(oFloor);
		const auto o1 = static_cast<int>(oFloor + 1);
		const auto oFrac = octaves - oFloor;
		const auto og0 = gainBufferOct[o0];
		const auto og1 = gainBufferOct[o1];
		const auto ogRange = og1 - og0;
		const auto oGain = og0 + oFrac * ogRange;
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			auto smpls = samples[ch];
			SIMD::multiply(smpls, oGain, numSamples);
			for (auto s = 0; s < numSamples; ++s)
				smpls[s] = math::limit(0., 1., smpls[s]);
		}
	}

	void Perlin2::fuckingApplyGainMate(double* smpls, double octaves, int numSamples) noexcept
	{
		const auto oFloor = std::floor(octaves);
		const auto o0 = static_cast<int>(oFloor);
		const auto o1 = static_cast<int>(oFloor + 1);
		const auto oFrac = octaves - oFloor;
		const auto og0 = gainBufferOct[o0];
		const auto og1 = gainBufferOct[o1];
		const auto ogRange = og1 - og0;
		const auto oGain = og0 + oFrac * ogRange;
		SIMD::multiply(smpls, oGain, numSamples);
		for (auto s = 0; s < numSamples; ++s)
			smpls[s] = math::limit(0., 1., smpls[s]);
	}
}