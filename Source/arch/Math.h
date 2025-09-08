#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#define oopsie(x) jassert(!(x))

namespace math
{
    using Char = juce::juce_wchar;
	using String = juce::String;
	using Point = juce::Point<int>;
	using PointF = juce::Point<float>;
	
    static constexpr double Tau = 6.28318530718;
    static constexpr double Pi = 3.14159265359;
    static constexpr double PiInv = 1. / Pi;
    static constexpr double PiHalf = Pi * .5;
    static constexpr double PiHalfInv = 1. / PiHalf;

    int fibonacci(int) noexcept;

    bool isPrime(int) noexcept;

    int prime(int) noexcept;

    template<typename Float>
    bool bufferSilent(Float*, int) noexcept;

    // samples, numChannels, numSamples
    template<typename Float>
    bool bufferSilent(Float**, int, int) noexcept;

    // min, max, x
    template<typename Float>
    Float limit(Float, Float, Float) noexcept;

    template<typename Float>
    Float sinApprox(Float) noexcept;

	template<typename Float>
    Float cosApprox(Float) noexcept;

    // for x != -9652.5
    // this one is asymetric
    template<typename Float>
    Float tanhApprox(Float) noexcept;

    template<typename Float>
    Float tanhPoly5(Float) noexcept;

    template<typename Float>
    Float tanhPoly7(Float) noexcept;

    template<typename Float>
    Float tanhPoly7Horner(Float) noexcept;

    float invSqrt(float) noexcept;

    // the quake III hack
    float invSqrtQuake(float) noexcept;

    // not smooth
    float expApprox(float) noexcept;

    template <typename Float>
    Float slightlySmaller(Float) noexcept;

    template<typename Float>
    // secs, Fs
    Float secsToSamples(Float, Float) noexcept;

    template<typename Float>
	// samples, fsInv
    Float samplesToSecs(Float, Float) noexcept;

    template<typename Float>
	// ms, Fs
    Float msToSamples(Float, Float) noexcept;

    template<typename Float>
	// samples, fsInv
    Float samplesToMs(Float, Float) noexcept;

    template<typename Float>
	// ms, Fs
    Float msToInc(Float, Float) noexcept;

    template<typename Float>
	// freqHz, Fs
    Float freqHzToSamples(Float, Float) noexcept;

    template<typename Float>
	// smpls, numSamples
    Float getRMS(const Float*, const int) noexcept;

    template<typename Float>
	// note, rootNote, xen, masterTune
    Float noteToFreqHz(Float, Float = static_cast<Float>(69), Float = static_cast<Float>(12), Float = static_cast<Float>(440)) noexcept;

    template<typename Float>
	// note, rootNote, masterTune
    Float noteToFreqHz2(Float, Float = static_cast<Float>(69), Float = static_cast<Float>(440)) noexcept;

    template<typename Float>
	// freqHz, rootNote, xen, masterTune
    Float freqHzToNote(Float, Float = static_cast<Float>(69), Float = static_cast<Float>(12), Float = static_cast<Float>(440)) noexcept;

    template<typename Float>
	// freqHz, xen, rootNote
    Float freqHzToNote2(Float, Float = static_cast<Float>(12), Float = static_cast<Float>(69)) noexcept;

    template<typename Float>
	// freqHz, xen, rootNote, masterTune
    Float closestFreq(Float, Float = static_cast<Float>(12), Float = static_cast<Float>(69), Float = static_cast<Float>(440)) noexcept;

    template<typename Float>
    // freqHz, Fs
    Float freqHzToFc(Float, Float) noexcept;

    template<typename Float>
	// fc, Fs
    Float fcToFreqHz(Float, Float) noexcept;

    template<typename Float>
    Float ampToDecibel(Float) noexcept;

    template<typename Float>
    Float dbToAmp(Float) noexcept;

    template<typename Float>
	// db, threshold
    Float dbToAmp(Float, Float) noexcept;

    template<typename Float>
    // buffer, readHead
    Float lerp(const Float*, Float) noexcept;

    template<typename Float>
	// buffer, readHead, size
    Float lerp(const Float*, Float, int) noexcept;

    template<typename Float>
	// buffer, readHead, size
    Float cubicHermiteSpline(const Float*, Float, int) noexcept;

    template<typename Float>
	// buffer, readHead
    Float cubicHermiteSpline(const Float* buffer, Float readHead) noexcept;

    template<typename Float>
    // xen [1, n], oct [-n, n], semi [-12, 12], fine [-1, 1]
    Float getRetuneValue(Float, Float, Float, Float) noexcept;

    bool isDigit(Char) noexcept;

    bool isLetter(Char) noexcept;

    bool isLetterOrDigit(Char) noexcept;

    int getDigit(Char) noexcept;

    String pitchclassToString(int) noexcept;

    bool isWhiteKey(int) noexcept;

    bool isBlackKey(int) noexcept;

    template<typename Float>
    // smpls, numSamples
    void applySomeWindowingFunction(Float*, int) noexcept;

    float getNumBeats(const String&);

    float nextLowestPowTwoX(float) noexcept;

    bool stringNegates(const String&);

    Point getAspectRatio(const String&);

    // returns length
    inline int intToByteArray(std::array<uint8_t, 128>& byteArray, int val) noexcept
    {
        auto i = 0;
        while (val > 0 && i < byteArray.size() - 1)
        {
            byteArray[i] = val & 0x7F;
            val >>= 7;
            ++i;
        }
        byteArray[i] = 0;
		return i;
    }

    inline int byteArrayToInt(const uint8_t* byteArray, int size) noexcept
    {
        auto val = 0;
        for (auto i = size - 1; i >= 0; --i)
        {
            val <<= 7;
            val |= byteArray[i] & 0x7F;
        }
		return val;
	}
}