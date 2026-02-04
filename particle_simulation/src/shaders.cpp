#include "shaders.hpp"

GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource)
{
    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint program = glCreateProgram();

    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    GLint success;

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
        throw ShaderLinkFailedError("Shader linking failed");

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compile error:" << infoLog << std::endl;
        throw std::runtime_error("Shader compile error");
    }

    return shader;
}