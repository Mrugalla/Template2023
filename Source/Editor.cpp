#include "Editor.h"

namespace gui
{
    Editor::Editor(AudioProcessor& p) :
        AudioProcessorEditor(&p),
        audioProcessor(p),
        // OPENGL
        context(),
        baseTex(),
        baseShader()
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
	}
    
    void Editor::renderOpenGL()
    {
    }
    
    void Editor::openGLContextClosing()
    {
    }
    
    void Editor::paint(juce::Graphics& g)
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