#include "Editor.h"
#include "BinaryData.h"

namespace gui
{
    static bool createShaders(OpenGLShaderProgram& shader, const String& vertexShader, const String& fragmentShader)
    {
        if (!shader.addVertexShader(vertexShader)
            || !shader.addFragmentShader(fragmentShader)
            || !shader.link())
        {
            jassertfalse;
            return false;
        }
        return true;
    }

    Editor::Editor(AudioProcessor& p) :
        AudioProcessorEditor(&p),
        audioProcessor(p),
        // OPENGL
        vertexBuffer(),
        indexBuffer(),
        context(),
        shaderProgram(context),
        vbo(0), ibo(0)
    {
        // OPENGL
        //context.setOpenGLVersionRequired(OpenGLContext::OpenGLVersion::openGL3_2);
        context.setRenderer(this);
        context.attachTo(*this);
        //context.setContinuousRepainting(true);
        
        // COMPONENT CONSTRAINER
        auto& thisConstrainer = *getConstrainer();
        thisConstrainer.setSizeLimits(69, 69, 10000, 10000);
        thisConstrainer.setFixedAspectRatio(AspectRatio);

        // APPLICATION PROPERTIES
        auto& props = audioProcessor.state.props;
        auto& user = *props.getUserSettings();
        
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
        glDisable(GL_DEBUG_OUTPUT);
        
        // Generate buffers (vertex and index) and store id in vbo and ibo
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);

        // Bind the Buffers
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        
        // Create 3 vertices to form a triangle
        vertexBuffer =
        {
            Vertex(-.5f, -.5f, 1.f, 0.f, 0.f, 1.f),
            Vertex(.5f, -.5f, 1.f, 0.f, 0.f, 1.f),
			Vertex(0.f, .5f, 1.f, 0.f, 1.f, 1.f),
        };

        // An indice for each corner of the triangle.
        indexBuffer =
        {
            0, 1, 2
        };
        
        // Send the vertices data.
        glBufferData
        (
            GL_ARRAY_BUFFER,
            sizeof(Vertex) * vertexBuffer.size(),
            vertexBuffer.data(),
            GL_STATIC_DRAW
        );

        // Send the indices data.
        glBufferData
        (
            GL_ELEMENT_ARRAY_BUFFER,
            sizeof(GLuint) * indexBuffer.size(),
            indexBuffer.data(),
            GL_STATIC_DRAW
        );
        
        createShaders
		(
			shaderProgram,
			String(BinaryData::vertex_cxx, BinaryData::vertex_cxxSize),
			String(BinaryData::fragment_cxx, BinaryData::fragment_cxxSize)
		);
	}
    
    void Editor::renderOpenGL()
    {
        OpenGLHelpers::clear(juce::Colours::black);
        shaderProgram.use();
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        
        glVertexAttribPointer
        (
            0,                        // Attribute Index
            Vertex::NumPosDimensions, // Num Values of this Attribute
            juce::gl::GL_FLOAT,       // Attribute Type
            juce::gl::GL_FALSE,       // Normalize
			sizeof(Vertex), 		  // Stride
			nullptr				      // Offset
        );
        glEnableVertexAttribArray(0);

        // Enable to colour attribute.
        glVertexAttribPointer
        (
            1,
            Vertex::NumColourDimensions,
            juce::gl::GL_FLOAT,
            juce::gl::GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<GLvoid*>(sizeof(float) * Vertex::NumPosDimensions)
        );
        glEnableVertexAttribArray(1);

        glDrawElements
        (
            GL_TRIANGLES,
            static_cast<GLsizei>(indexBuffer.size()), // How many indices
            GL_UNSIGNED_INT,    // type of indices
            nullptr
        );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
    
    void Editor::openGLContextClosing()
    {
        
    }
    
    void Editor::paint(Graphics&)
    {
    }

    void Editor::resized()
    {
        auto& props = audioProcessor.state.props;
        auto& user = *props.getUserSettings();

		auto newScale = static_cast<double>(getWidth()) / AspectRatio;

        user.setValue("EditorScale", newScale);
    }
}