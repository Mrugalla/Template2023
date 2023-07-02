#include "Editor.h"
#include "BinaryData.h"

#if JUCE_DEBUG
#define GLCall(x) glClearError();\
    x;\
	jassert(glLogCall(#x, __FILE__, __LINE__))
#else
#define GLCall(x) x
#endif

namespace gui
{
    extern void glClearError()
    {
		while (glGetError() != GL_NO_ERROR);
    }

    extern bool glLogCall(const char* function, const char* file, int line)
    {
        function = function;
        file = file;
        line = line;
        while (auto error = glGetError())
        {
            DBG("[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line);
            return false;
        }
        return true;
    }

    extern bool createShaders(GLShaderProgram& shader, const String& vertexShader, const String& fragmentShader)
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

	extern void setRenderer(GLContext& context, GLRenderer& renderer, juce::Component& comp)
	{
        context.setRenderer(&renderer);
        context.attachTo(comp);
	}

    extern void genBuffer(GLuint* buffer)
    {
        GLCall(glGenBuffers(1, buffer));
    }

	extern void bindBuffer(GLenum target, GLuint buffer)
	{
		GLCall(glBindBuffer(target, buffer));
	}

	extern void bufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
	{
		GLCall(glBufferData(target, size, data, usage));
	}

	extern void vertexAttribPointer(GLuint idx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
	{
		GLCall(glVertexAttribPointer(idx, size, type, normalized, stride, pointer));
        GLCall(glEnableVertexAttribArray(idx));
	}

	extern void drawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices = nullptr)
	{
		GLCall(glDrawElements(mode, count, type, indices));
	}

    extern void disableVertexAttribArray(GLuint idx)
    {
		GLCall(glDisableVertexAttribArray(idx));
    }

    Editor::Editor(AudioProcessor& p) :
        AudioProcessorEditor(&p),
        Timer(),
        audioProcessor(p),
        // OPENGL
        vertexBuffer(),
        indexBuffer(),
        context(),
        shaderProgram(context),
        vbo(0), ibo(0)
    {
        setRenderer(context, *this, *this);
        
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
            startTimerHz(FPS);
        }
    }

    Editor::~Editor()
    {
        context.detach();
    }

    void Editor::timerCallback()
    {
        
    }
    
    void Editor::newOpenGLContextCreated()
	{
        glDisable(GL_DEBUG_OUTPUT);
        
        genBuffer(&vbo);
		genBuffer(&ibo);

        bindBuffer(GL_ARRAY_BUFFER, vbo);
        bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        
        vertexBuffer =
        {
            Vertex(-.5f, -.5f, 0.f, 1.f, 1.f, 1.f),
            Vertex(.5f, -.5f, 0.f, 1.f, 1.f, 1.f),
			Vertex(.5f, .5f, 1.f, 0.f, 1.f, 1.f),
            Vertex(-.5f, .5f, 1.f, 0.f, 1.f, 1.f),
        };

        indexBuffer =
        {
			0, 1, 2,
            2, 3, 0
        };
        
        bufferData
        (
            GL_ARRAY_BUFFER,
            sizeof(Vertex) * vertexBuffer.size(),
            vertexBuffer.data(),
            GL_STATIC_DRAW
        );

        bufferData
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

		auto uColourID = shaderProgram.getUniformIDFromName("uColour");
        glUniform4f(uColourID, 1.f, 0.f, 0.f, 1.f);
        
	}
    
    void Editor::renderOpenGL()
    {
        GLHelpers::clear(juce::Colours::black);
        shaderProgram.use();
        
        bindBuffer(GL_ARRAY_BUFFER, vbo);
        bindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        
        vertexAttribPointer
        (
            0,                        // Attribute Index
            Vertex::NumPosDimensions, // Num Values of this Attribute
            juce::gl::GL_FLOAT,       // Attribute Type
            juce::gl::GL_FALSE,       // Normalize
			sizeof(Vertex), 		  // Stride
			nullptr				      // Offset
        );
        
        vertexAttribPointer
        (
            1,
            Vertex::NumColourDimensions,
            juce::gl::GL_FLOAT,
            juce::gl::GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<GLvoid*>(sizeof(float) * Vertex::NumPosDimensions)
        );

        drawElements
        (
            GL_TRIANGLES,
            static_cast<GLsizei>(indexBuffer.size()),
            GL_UNSIGNED_INT
        );

        disableVertexAttribArray(0);
        disableVertexAttribArray(1);
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