#include "Editor.h"

namespace gui
{
    Editor::Editor(AudioProcessor& p) :
        AudioProcessorEditor(&p),
        audioProcessor(p),
        // OPENGL
        arraybuffer(0),
        context(),
        baseTex(),
        baseShader(context)
    {
        context.setOpenGLVersionRequired(OpenGLContext::OpenGLVersion::openGL3_2);
        context.setRenderer(this);
        context.attachTo(*this);
        
        auto& props = audioProcessor.props;
        auto& user = *props.getUserSettings();

        auto& constrainer = *getConstrainer();
        constrainer.setSizeLimits(69, 69, 10000, 10000);
        constrainer.setFixedAspectRatio(AspectRatio);

        {
            const auto scale = user.getDoubleValue("EditorScale", 420.);
            setOpaque(true);
            setResizable(true, true);
            setSize
            (
                static_cast<int>(scale * AspectRatio),
                static_cast<int>(scale)
            );
        }
    }

    Editor::~Editor()
    {
        context.detach();
    }

    void Editor::newOpenGLContextCreated()
	{
        context.extensions.glGenBuffers(1, &arraybuffer);
	}
    
    void Editor::renderOpenGL()
    {
        context.extensions.glBindBuffer(juce::gl::GL_ARRAY_BUFFER, arraybuffer);
        context.extensions.glBufferData(juce::gl::GL_ARRAY_BUFFER, sizeof(float) * 8, square, juce::gl::GL_DYNAMIC_DRAW);

        auto coord = context.extensions.glGetAttribLocation(baseShader.getProgramID(), "aPos");

        context.extensions.glEnableVertexAttribArray(coord);
        context.extensions.glVertexAttribPointer(coord, 2, juce::gl::GL_FLOAT, juce::gl::GL_FALSE, 0, 0);
    }
    
    void Editor::openGLContextClosing()
    {
        baseShader.release();
        baseTex.release();
    }
    
    void Editor::paint(Graphics&)
    {
        //g.fillAll(juce::Colours::black);
    }

    void Editor::resized()
    {
        auto& props = audioProcessor.props;
        auto& user = *props.getUserSettings();

		auto newScale = static_cast<double>(getWidth()) / AspectRatio;

        user.setValue("EditorScale", newScale);
    }
}