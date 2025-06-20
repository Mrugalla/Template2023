#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <BinaryData.h>

#include "../Processor.h"
#include <functional>
#include <optional>

namespace gui
{
    using Colour = juce::Colour;
    using Gradient = juce::ColourGradient;
    using String = juce::String;
    using WChar = juce::juce_wchar;
    using Font = juce::Font;
	using GlyphArrangement = juce::GlyphArrangement;
    using FontOptions = juce::FontOptions;
    using Props = juce::PropertiesFile;
    using AppProps = juce::ApplicationProperties;
    using Cursor = juce::MouseCursor;
    using Image = juce::Image;
	using ScaledImage = juce::ScaledImage;
    using Graphics = juce::Graphics;
    using Mouse = juce::MouseEvent;
    using MouseWheel = juce::MouseWheelDetails;
    using Just = juce::Justification;
    using Timer = juce::Timer;
    using Path = juce::Path;
    using Point = juce::Point<int>;
    using PointF = juce::Point<float>;
    using PointD = juce::Point<double>;
    using Bounds = juce::Rectangle<int>;
    using BoundsF = juce::Rectangle<float>;
    using BoundsD = juce::Rectangle<double>;
    using Line = juce::Line<int>;
    using LineF = juce::Line<float>;
    using LineD = juce::Line<double>;
    using Image = juce::Image;
    using RectPlacement = juce::RectanglePlacement;
    using Stroke = juce::PathStrokeType;
    using Affine = juce::AffineTransform;
    using Random = juce::Random;
    using KeyPress = juce::KeyPress;
    using ValueTree = juce::ValueTree;
    using File = juce::File;
    using RangedDirectoryIterator = juce::RangedDirectoryIterator;
    using Identifier = juce::Identifier;
    using RangeF = juce::NormalisableRange<float>;
    using RangeD = juce::NormalisableRange<double>;
    using Time = juce::Time;
    using FileChooser = juce::FileChooser;
    using Component = juce::Component;
    using SystemClipboard = juce::SystemClipboard;
    using SIMD = juce::FloatVectorOperations;
    using ImageFileFormat = juce::ImageFileFormat;
    using PNGImageFormat = juce::PNGImageFormat;
    using FileOutputStream = juce::FileOutputStream;
    using Var = juce::var;
    using StringArray = juce::StringArray;
    using AudioFormatManager = juce::AudioFormatManager;
    using AudioBufferF = juce::AudioBuffer<float>;
	using RectanglePlacement = juce::RectanglePlacement;
	using ResamplingQuality = Graphics::ResamplingQuality;
	using ColourSelector = juce::ColourSelector;
	using ImageCache = juce::ImageCache;
	using Int64 = juce::int64;
	using WavAudioFormat = juce::WavAudioFormat;
	using AudioFormatWriter = juce::AudioFormatWriter;
	using MouseCursor = juce::MouseCursor;
    using FileDragAndDropTarget = juce::FileDragAndDropTarget;
	using DragAndDropTarget = juce::DragAndDropTarget;
	using DragAndDropContainer = juce::DragAndDropContainer;
    using DnDSrc = DragAndDropTarget::SourceDetails;
	using URL = juce::URL;
	using FineAssTransform = juce::AffineTransform;
	using CharUTF8 = juce::CharPointer_UTF8;

    using Processor = audio::Processor;

    using PID = param::PID;
    using Param = param::Param;
    using Params = param::Params;

    static constexpr float Tau = 6.28318530718f;
    static constexpr float Pi = Tau * .5f;;
    static constexpr float PiHalf = Tau * .25f;
    static constexpr float PiQuart = Tau * .125f;
	static constexpr float PiEight = Tau * .0625f;
	static constexpr float PiSixteenth = Tau * .03125f;
	static constexpr float Pi32 = Pi / 32.f;
    static constexpr int EditorWidth = 596;
    static constexpr int EditorHeight = 468;
    static constexpr int EditorMinWidth = 100;
    static constexpr int EditorMinHeight = 100;
    static constexpr int FPS = 60;

    static constexpr double FPSD = static_cast<double>(FPS);
    static constexpr double FPSInv = 1. / FPSD;

    bool isLineBreak(WChar) noexcept;

    bool isTextCharacter(WChar) noexcept;

    namespace font
    {
        Font nel();
        Font flx();
        Font text();
        Font headline();
    }

    namespace ascii
    {
        inline String mouth()
        {
            return String(CharUTF8("\xe2\x80\xbf"));
        }

        inline String brokenHeart()
        {
            return String(CharUTF8("\xf0\x9f\x92\x94"));
        }

        inline String bear()
        {
            const auto a = String(CharUTF8("\xca\x95"));
            const auto b = String(CharUTF8("\xe2\x80\xa2"));
            const auto c = String(CharUTF8("\xe1\xb4\xa5"));
			return a + b + c + b + a;
        }

        inline String cuteFace()
        {
            const auto flower = String(CharUTF8("\xe2\x9c\xbf"));
            const auto eye = String(CharUTF8("\xe2\x97\xa0"));
			return "(" + flower + eye + mouth() + eye + ")";
        }

        inline String rolfFace()
        {
            const auto r = String(CharUTF8("\xe2\x89\xa7"));
            return "(" + r + mouth() + r + ")";
        }

        inline String pwesets()
        {
            const auto dotLow = String(CharUTF8("\xef\xbd\xa1"));
            const auto dotHigh = String(CharUTF8("\xef\xbe\x9f"));
            const auto dotCentre = String(CharUTF8("\xef\xbd\xa5"));
            const auto star = String(CharUTF8("\xe2\x98\x86"));
            const auto arm = String(CharUTF8("\xe3\x82\x9e"));
			const auto star2 = String(CharUTF8("\xe2\x9c\xa7"));
            const auto music = String(CharUTF8("\xf0\x9f\x8e\xb6"));
            return dotLow + dotHigh + dotCentre + star + "(*^w^*)" + star + dotCentre + dotHigh + ": " + music
                + " pwesets " + music + " :" + dotCentre + dotHigh + star2 + ":" + dotCentre + dotHigh + star2 + "(*^" + mouth() + "^)" + arm + star + dotCentre + dotHigh + dotLow;
        }
    }
}