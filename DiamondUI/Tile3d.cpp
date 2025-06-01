#include "Tile3d.h"


#include "Diamond.h"
#include "Textures.h"
#include "Camera.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

Tile3d::Tile3d() {
    m_unEBO = 0;
    m_unVAO = 0;
    m_unVBO = 0;

    m_unEntityId = 0;
}

Tile3d::~Tile3d() {
    Deinitialise();
}

void Tile3d::Initialise(unsigned int unEntityId) {
    Initialise(unEntityId, nullptr);
}

void Tile3d::Initialise(unsigned int unEntityId, Textures* pTextureCtrl) {
    m_unEntityId = unEntityId;

    // Create source vertices, that are just the points of the polygon
    CreateSourceVertices();
    CalculateBoundingSphereFromSourceVertices();
    // These vertices are then replicated and given normals, to construct the vertices passed to gl
    // Note, this method called gives ownership to verticesForGL, which must be freed at the end of this method
    float* pfVerticesForGL_Owned = CreateGLTriVerticesFromSourceData();

    glGenVertexArrays(1, &m_unVAO);
    glGenBuffers(1, &m_unVBO);

    // The Vertex Array Object stores the state associated with the VBO, so the following code is stored, ensuring
    //  the binding of the vbo, and the setup of the vertex data, does not need to be repeated
    glBindVertexArray(m_unVAO);

    // The Vertex Buffer Object is of type GL_ARRAY_BUFFER. This binds the VBO identified by m_unVBO so that
    //  future operations on GL_ARRAY_BUFFER are performed on this VBO.
    glBindBuffer(GL_ARRAY_BUFFER, m_unVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 3 * m_unTriCount, pfVerticesForGL_Owned, GL_STATIC_DRAW);

    unsigned int unStride = 6 * sizeof(float);
    unsigned long long unOffset = 0 * sizeof(float);
    unsigned int unPosSize = 3;
    unsigned int unNormalSize = 3;
    unsigned int unTextureSize = 2;

    bool bHasTexture = false;
    int attribArrayIndex = 0;
    // position attribute
    glVertexAttribPointer(attribArrayIndex, unPosSize, GL_FLOAT, GL_FALSE, unStride, (void*)unOffset);
    glEnableVertexAttribArray(attribArrayIndex++);
    unOffset += unPosSize * sizeof(float);

    // normal attribute
    glVertexAttribPointer(attribArrayIndex, unNormalSize, GL_FLOAT, GL_FALSE, unStride, (void*)unOffset);
    glEnableVertexAttribArray(attribArrayIndex++);
    unOffset += unNormalSize * sizeof(float);

    glBindVertexArray(0);
    if (bHasTexture) {
        // texture coord attribute
        glVertexAttribPointer(attribArrayIndex, unTextureSize, GL_FLOAT, GL_FALSE, unStride, (void*)unOffset);
        glEnableVertexAttribArray(attribArrayIndex++);
        unOffset += unTextureSize * sizeof(float);
    }

    m_shader.InitialiseShaders("polyhedral_vert.glsl", "polyhedral_frag.glsl");
    m_shaderShadow.InitialiseShaders("polyhedral_shadow_vert.glsl", "polyhedral_shadow_frag.glsl");

    m_shader.Use();
    m_shader.SetInt("shadowMap", 0);
    delete[] pfVerticesForGL_Owned;
    DeleteSrcData();
}

void Tile3d::Deinitialise() {
    glDeleteVertexArrays(1, &m_unVAO);
    m_unVAO = 0;
    glDeleteBuffers(1, &m_unVBO);
    m_unVBO = 0;
}

void Tile3d::CreateSourceVertices() {
    InitialiseSrcData();
    SetNextVertex(3.0f, -1.0f, 3.0f);
    SetNextVertex(3.0f, -1.0f, -3.0f);
    SetNextVertex(-3.0f, -1.0f, -3.0f);
    SetNextVertex(-3.0f, -1.0f, 3.0f);

    SetNextVertex(3.0f, 0.1f, 3.0f);
    SetNextVertex(3.0f, 0.1f, -3.0f);
    SetNextVertex(-3.0f, 0.1f, -3.0f);
    SetNextVertex(-3.0f, 0.1f, 3.0f);

    // Bottom
    CreateNextSrcTriangle(0, 2, 1);
    CreateNextSrcTriangle(0, 3, 2);
    // Top
    CreateNextSrcTriangle(4, 5, 6);
    CreateNextSrcTriangle(4, 6, 7);
    // Rear
    CreateNextSrcTriangle(0, 4, 7);
    CreateNextSrcTriangle(0, 7, 3);
    // Right side
    CreateNextSrcTriangle(0, 5, 4);
    CreateNextSrcTriangle(0, 1, 5);
    // Left side
    CreateNextSrcTriangle(3, 7, 6);
    CreateNextSrcTriangle(3, 6, 2);
    // Front
    CreateNextSrcTriangle(2, 6, 5);
    CreateNextSrcTriangle(2, 5, 1);
}
