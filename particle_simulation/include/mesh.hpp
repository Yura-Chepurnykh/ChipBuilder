#ifndef MESH_HPP
#define MESH_HPP

#include <glad/glad.h>
#include <cstddef>

class Mesh 
{
public:
    Mesh(float*, size_t);
    ~Mesh();

    void draw();

private:
    float* m_vertices;
    size_t m_size;
    GLuint m_vbo, m_vao;
};

#endif // MESH_HPP