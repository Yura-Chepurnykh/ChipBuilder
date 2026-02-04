#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>
#include "errors.hpp"

inline const char* vertexShader = R"(

#version 330 core
layout(location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1);
}

)";

inline const char* fragmentShader = R"(

#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.4f, 0.7f, 0.4f, 0.4f);
}

)";

GLuint createShaderProgram(const char*, const char*);
GLuint compileShader(GLenum, const char*);

#endif // SHADER_HPP