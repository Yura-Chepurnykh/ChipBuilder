#include "mesh.hpp"

Mesh::Mesh(float* vertices, size_t size) : m_vertices(vertices), m_size(size)
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void Mesh::draw()
{
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_size);
}