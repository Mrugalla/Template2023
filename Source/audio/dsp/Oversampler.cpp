#include "Oversampler.h"

namespace dsp
{
	ImpulseResponse::ImpulseResponse() :
		buffer(),
		size(0)
	{
	}

	double& ImpulseResponse::operator[](int i)
	{
		return buffer[i];
	}

	const double& ImpulseResponse::operator[](int i) const
	{
		return buffer[i];
	}

	/*
	* Fs, fc, bw, upsampling
	nyquist == Fs / 2
	fc < nyquist
	bw < nyquist
	fc + bw < nyquist
	*/
	void ImpulseResponse::makeLowpass(double Fs, double fc,
		double bw, bool upsampling)
	{
		const auto nyquist = Fs * .5;
		if (fc > nyquist || bw > nyquist || fc + bw > nyquist)
			return;

		fc /= Fs;
		bw /= Fs;

		auto M = static_cast<int>(4. / bw);
		if (M % 2 != 0)
			++M; // M is even number

		const auto Mf = static_cast<double>(M);
		const auto MHalf = Mf * .5;
		const auto MInv = 1. / Mf;

		size = std::min(Size, M + 1);

		const auto h = [&](double i)
			{ // sinc
				i -= MHalf;
				if (i != 0.)
					return std::sin(Tau * fc * i) / i;
				return Tau * fc;
			};

		const auto w = [&, tau2 = Tau * 2.](double i)
			{ // blackman window
				i *= MInv;
				return .42 - .5 * std::cos(Tau * i) + .08 * std::cos(tau2 * i);
			};

		for (auto n = 0; n < size; ++n)
		{
			auto nF = static_cast<double>(n);
			buffer[n] = h(nF) * w(nF);
		}

		const auto targetGain = upsampling ? 2. : 1.;
		auto sum = 0.; // normalize
		for (auto n = 0; n < size; ++n)
			sum += buffer[n];
		const auto sumInv = targetGain / sum;
		for (auto n = 0; n < size; ++n)
			buffer[n] *= sumInv;
	}

	/*
	* Fs, fc, upsampling
	nyquist == Fs / 2
	fc < nyquist
	*/
	void ImpulseResponse::makeLowpass(double Fs, double fc, bool upsampling)
	{
		const auto bw = Fs * .25 - fc - 1.;
		makeLowpass(Fs, fc, bw, upsampling);
	}

	int ImpulseResponse::getLatency() const noexcept
	{
		return size / 2;
	}

	using ConvolverBuffer = std::array<ImpulseResponse::Buffer, NumChannels>;

	Convolver::Convolver(const ImpulseResponse& ir) :
		ir(ir),
		ringBuffer()
	{
	}

	void Convolver::processBlock(double* const* samples, const int* wHead,
		int numChannels, int numSamples) noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			auto smpls = samples[ch];
			auto ring = ringBuffer[ch].data();

			processBlock(smpls, ring, wHead, numSamples);
		}
	}

	void Convolver::processBlock(double* smpls, double* ring,
		const int* wHead, int numSamples) noexcept
	{
		for (auto s = 0; s < numSamples; ++s)
			smpls[s] = processSample(smpls[s], ring, wHead[s]);
	}

	double Convolver::processSample(double smpl, double* ring, int w) noexcept
	{
		ring[w] = smpl;

		auto r = w;
		auto y = ring[r] * ir[0];

		const auto size = ir.size;
		const auto max = size - 1;
		for (auto i = 1; i < size; ++i)
		{
			--r;
			if (r == -1)
				r = max;

			y += ring[r] * ir[i];
		}

		return y;
	}

	/* samplesUp, samplesIn, numChannels, numSamples1x */
	void zeroStuff(double* const* samplesUp, const double* const* samplesIn,
		int numChannels, int numSamples1x) noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			auto upBuf = samplesUp[ch];
			const auto inBuf = samplesIn[ch];

			for (auto s = 0; s < numSamples1x; ++s)
			{
				const auto s2 = s * 2;
				upBuf[s2] = inBuf[s];
				upBuf[s2 + 1] = 0.;
			}
		}
	}

	/* samplesOut, samplesUp, numChannels, numSamples1x */
	void decimate(double* const* samplesOut, const double* const* samplesUp,
		int numChannels, int numSamples1x) noexcept
	{
		for (auto ch = 0; ch < numChannels; ++ch)
		{
			auto outBuf = samplesOut[ch];
			const auto upBuf = samplesUp[ch];

			for (auto s = 0; s < numSamples1x; ++s)
				outBuf[s] = upBuf[s * 2];
		}
	}

	Oversampler::Oversampler() :
		sampleRate(0.),
		bufferUp(),
		bufferInfo(),
		irUp(), irDown(),
		wHead(),
		filterUp(irUp), filterDown(irDown),
		sampleRateUp(0.),
		numSamplesUp(0),
		enabled(false)
	{
	}

	void Oversampler::prepare(const double _sampleRate, bool _enabled)
	{
		sampleRate = _sampleRate;
		enabled = _enabled;

		if (enabled)
		{
			sampleRateUp = sampleRate * 2.;

			irUp.makeLowpass(sampleRateUp, LPCutoff, true);
			irDown.makeLowpass(sampleRateUp, LPCutoff, false);
			const auto irSize = static_cast<int>(irUp.size);
			wHead.prepare(irSize);
		}
		else
		{
			sampleRateUp = sampleRate;
		}
	}

	Oversampler::BufferInfo Oversampler::upsample(double* const* samples,
		int numChannels, int numSamples) noexcept
	{
		bufferInfo.numChannels = numChannels;

		if (enabled)
		{
			bufferInfo.numSamples = numSamplesUp = numSamples * 2;
			bufferInfo.smplsL = bufferUp[0].data();
			bufferInfo.smplsR = bufferUp[1].data();
			double* samplesUp[] = { bufferInfo.smplsL, bufferInfo.smplsR };

			wHead(numSamplesUp);
			const auto wHeadData = wHead.data();

			zeroStuff(samplesUp, samples, numChannels, numSamples);
			filterUp.processBlock(samplesUp, wHeadData, numChannels, numSamplesUp);
		}
		else
		{
			bufferInfo.numSamples = numSamples;
			bufferInfo.smplsL = samples[0];
			bufferInfo.smplsR = samples[1];
		}

		return bufferInfo;
	}

	void Oversampler::downsample(double* const* samplesOut, int numSamples) noexcept
	{
		if (enabled)
		{
			const auto numChannels = bufferInfo.numChannels;
			const auto wHeadData = wHead.data();

			double* samplesUp[] = { bufferInfo.smplsL, bufferInfo.smplsR };

			// filter 2x + decimating
			filterDown.processBlock(samplesUp, wHeadData, numChannels, numSamplesUp);
			decimate(samplesOut, samplesUp, numChannels, numSamples);
		}
	}

	int Oversampler::getLatency() const noexcept
	{
		return enabled ? (irUp.getLatency() + irDown.getLatency() / 2) : 0;
	}
}