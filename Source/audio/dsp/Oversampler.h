#pragma once
#include "WHead.h"

namespace dsp
{
	
	struct ImpulseResponse
	{
		static constexpr int Size = 1 << 8;
		using Buffer = std::array<double, Size>;

		ImpulseResponse();

		double& operator[](int);

		const double& operator[](int) const;

		/*
		* Fs, fc, bw, upsampling
		nyquist == Fs / 2
		fc < nyquist
		bw < nyquist
		fc + bw < nyquist
		*/
		void makeLowpass(double, double, double, bool);

		/*
		* Fs, fc, upsampling
		nyquist == Fs / 2
		fc < nyquist
		*/
		void makeLowpass(double, double, bool);

		int getLatency() const noexcept;

	private:
		Buffer buffer;
	public:
		int size;
	};

	using ConvolverBuffer = std::array<ImpulseResponse::Buffer, NumChannels>;

	struct Convolver
	{
		Convolver(const ImpulseResponse&);

		/* samples, wHead, numChannels, numSamples */
		void processBlock(double* const*, const int*, int, int) noexcept;

		/* smpls, ring, whead, numSamples */
		void processBlock(double*, double*, const int*, int) noexcept;

		/* smpl, ring, w */
		double processSample(double, double*, int) noexcept;

	private:
		const ImpulseResponse& ir;
		ConvolverBuffer ringBuffer;
	};

	struct Oversampler
	{
		static constexpr double LPCutoff = 20000.;
		using OversamplerBuffer = std::array<std::array<double, BlockSize2x>, NumChannels>;

		struct BufferInfo
		{
			double *smplsL, *smplsR;
			int numChannels, numSamples;
		};

		Oversampler();

		/* sampleRate, enabled */
		void prepare(const double, bool);

		/* samples, numChannels, numSamples */
		BufferInfo upsample(double* const*, int, int) noexcept;

		/* samplesOut, numSamples */
		void downsample(double* const*, int) noexcept;

		int getLatency() const noexcept;

	private:
		double sampleRate;
		OversamplerBuffer bufferUp;
		BufferInfo bufferInfo;
		ImpulseResponse irUp, irDown;
		WHead2x wHead;
		Convolver filterUp, filterDown;
	public:
		double sampleRateUp;
		int numSamplesUp;
		bool enabled;
	};
}