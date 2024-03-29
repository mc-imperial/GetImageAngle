//            Based on Hello_Triangle.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

//
// This file is used by the template to render a basic scene using GL.
//

#include "stdafx.h"
#include "HelloTriangleRenderer.h"
#include "lodepng.h"

// These are used by the shader compilation methods.
#include <vector>
#include <iostream>
#include <fstream>

using namespace GetImageAngle;

#define STRING(s) #s

GLuint CompileShader(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);

    const char *sourceArray[1] = { source.c_str() };
    glShaderSource(shader, 1, sourceArray, NULL);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

    if (compileResult == 0)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, infoLog.size(), NULL, infoLog.data());

        std::wstring errorMessage = std::wstring(L"Shader compilation failed: ");
        errorMessage += std::wstring(infoLog.begin(), infoLog.end());
        OutputDebugStringW(errorMessage.c_str());

        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

GLuint CompileProgram(const std::string &vsSource, const std::string &fsSource)
{
    GLuint program = glCreateProgram();

    if (program == 0)
    {
        OutputDebugStringW(L"Program creation failed");
        return 0;
    }

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    if (vs == 0 || fs == 0)
    {
        glDeleteShader(fs);
        glDeleteShader(vs);
        glDeleteProgram(program);
        exit(COMPILE_ERROR_EXIT_CODE);
        return 0;
    }

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == 0)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetProgramInfoLog(program, infoLog.size(), NULL, infoLog.data());

        std::wstring errorMessage = std::wstring(L"Shader linking failed: ");
        errorMessage += std::wstring(infoLog.begin(), infoLog.end());
        OutputDebugStringW(errorMessage.c_str());

        glDeleteProgram(program);
        exit(LINK_ERROR_EXIT_CODE);
        return 0;
    }

    return program;
}

std::string LoadFile(const std::string& filePath)
{
    std::ifstream shdf0(filePath);
    std::stringstream shds0;
    shds0 << shdf0.rdbuf();
    return shds0.str();
}


bool HelloTriangleRenderer::Initialize() 
{
    std::string vs;
    if (args.vertex_shader == "")
    {
        vs = "attribute vec2 coord2d;\nvarying vec2 surfacePosition;\nvoid main(void) {\ngl_Position = vec4(coord2d, 0.0, 1.0);\nsurfacePosition = coord2d;\n}";
    }
    else
    {
        vs = LoadFile(args.vertex_shader);
    }

    const std::string fs(LoadFile(args.fragment_shader));

    mProgram = CompileProgram(vs, fs);
    if (!mProgram)
    {
        return false;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, width, height);

    return true;
}

HelloTriangleRenderer::~HelloTriangleRenderer()
{
    if (mProgram != 0)
    {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
}

// Draws a basic triangle
void HelloTriangleRenderer::Draw()
{
    GLfloat vertices1[] =
    {
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,
    };

    GLfloat vertices2[] =
    {
        -1.0f,  1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f,
    };

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(mProgram);

    const char* uniform_name;
    GLint uniform_loc;
    uniform_name = "resolution";
    uniform_loc = glGetUniformLocation(mProgram, uniform_name);
    if (uniform_loc != -1)
        glUniform2f(uniform_loc, width, height);
    uniform_name = "mouse";
    uniform_loc = glGetUniformLocation(mProgram, uniform_name);
    if (uniform_loc != -1)
        glUniform2f(uniform_loc, 0.0f, 0.0f);
    uniform_name = "injectionSwitch";
    uniform_loc = glGetUniformLocation(mProgram, uniform_name);
    if (uniform_loc != -1)
        glUniform2f(uniform_loc, 0.0f, 1.0f);
    uniform_name = "time";
    uniform_loc = glGetUniformLocation(mProgram, uniform_name);
    if (uniform_loc != -1)
        glUniform1f(uniform_loc, 0.0f);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices1);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

}

#define OUTPUT_IMAGE_MAX_PIXEL_VAL 255


void HelloTriangleRenderer::Step(double deltaTime, double elapsedTime)
{
    if (stepCountdown > 0) --stepCountdown;

    if (!saved && stepCountdown == 0)
    {

        std::vector<std::uint8_t> data(width * height * channels);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
        std::vector<std::uint8_t> flipped_data(width * height * channels);
        for (int h = 0; h < height; h++)
            for (int col = 0; col < width * channels; col++)
                flipped_data[h * width * channels + col] =
                data[(height - h - 1) * width * channels + col];
        unsigned png_error = lodepng::encode(args.output, flipped_data, width, height);
        if (png_error)
        {
            printf("Error producing PNG file: %s", lodepng_error_text(png_error));
            exit(PNG_ERROR_EXIT_CODE);
        }
        if (args.persist)
            printf("Press any key to exit...\n");
        else
            exit(0);
        
    }
}

void HelloTriangleRenderer::OnWindowSizeChanged(GLsizei width, GLsizei height)
{
    
}
