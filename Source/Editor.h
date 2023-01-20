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
    static constexpr int FPS = 60;

    // JUCE GRAPHICS
	using AudioProcessorEditor = juce::AudioProcessorEditor;
    using Timer = juce::Timer;
    using Graphics = juce::Graphics;
    using String = juce::String;
    using Just = juce::Justification;
    // OPENGL
    using GLContext = juce::OpenGLContext;
    using GLTexture = juce::OpenGLTexture;
	using GLShaderProgram = juce::OpenGLShaderProgram;
	using GLRenderer = juce::OpenGLRenderer;
	using GLHelpers = juce::OpenGLHelpers;
    // CUSTOM
    using AudioProcessor = audio::Processor;
    
    struct Vertex
    {
        enum { X, Y, NumPosDimensions };
        enum { Red, Green, Blue, Alpha, NumColourDimensions };
        
		Vertex(float x = 0.f, float y = 0.f,
            float r = 0.f, float g = 0.f, float b = 0.f, float a = 0.f) :
			pos{ x, y },
			col{ r, g, b, a }
		{}

        float pos[NumPosDimensions];
        float col[NumColourDimensions];
    };

    using namespace juce::gl;

    struct Editor :
        public AudioProcessorEditor,
        public GLRenderer,
        public Timer
    {
        Editor(AudioProcessor&);
        ~Editor();
        
        void newOpenGLContextCreated() override;
        void renderOpenGL() override;
        void openGLContextClosing() override;

        void paint(Graphics&) override;
        void resized() override;
        
        void timerCallback() override;

        AudioProcessor& audioProcessor;

        // OPENGL
        std::vector<Vertex> vertexBuffer;
        std::vector<GLuint> indexBuffer;
        
        GLContext context;
        GLShaderProgram shaderProgram;

        GLuint vbo, ibo;
    };
}