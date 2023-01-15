#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>

#include "Processor.h"

namespace gui
{
	static constexpr double AspectRatio = 16. / 9.;

    // JUCE GRAPHICS
	using AudioProcessorEditor = juce::AudioProcessorEditor;
    using Graphics = juce::Graphics;
    // OPENGL
    using OpenGLContext = juce::OpenGLContext;
    using OpenGLTexture = juce::OpenGLTexture;
	using OpenGLShaderProgram = juce::OpenGLShaderProgram;
	using OpenGLRenderer = juce::OpenGLRenderer;
    // CUSTOM
    using AudioProcessor = audio::Processor;

    struct Editor :
        public AudioProcessorEditor,
        public OpenGLRenderer
    {
        Editor(AudioProcessor&);
        ~Editor();

        void newOpenGLContextCreated() override;
        void renderOpenGL() override;
        void openGLContextClosing() override;

        void paint(juce::Graphics&) override;
        void resized() override;

        AudioProcessor& audioProcessor;

        // OPENGL
        unsigned int arraybuffer;
        float square[8]
        {
            0.f,0.f,
            1.f,0.f,
            0.f,1.f,
            1.f,1.f 
        };
        OpenGLContext context;
        OpenGLTexture baseTex;
        OpenGLShaderProgram baseShader;
    };
}