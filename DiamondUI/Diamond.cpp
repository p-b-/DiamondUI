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

Diamond::Diamond() {
    m_unEBO = 0;
    m_unVAO = 0;
    m_unVBO = 0;

    m_unEntityId = 0;
}

Diamond::~Diamond() {
    Deinitialise();
}

void Diamond::Initialise(unsigned int unEntityId) {
    Initialise(unEntityId, nullptr);
}

void Diamond::Initialise(unsigned int unEntityId, Textures* pTextureCtrl) {
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

    // TODO Use a standard shader for all polyhedrals
    m_shader.InitialiseShaders("polyhedral_vert.glsl", "polyhedral_frag.glsl");
    m_shaderShadow.InitialiseShaders("polyhedral_shadow_vert.glsl", "polyhedral_shadow_frag.glsl");
    m_shader.Use();
    m_shader.SetInt("shadowMap", 0);

    delete[] pfVerticesForGL_Owned;
    DeleteSrcData();
}

void Diamond::Deinitialise() {
    glDeleteVertexArrays(1, &m_unVAO);
    m_unVAO = 0;
    glDeleteBuffers(1, &m_unVBO);
    m_unVBO = 0;
}

void Diamond::CreateSourceVertices() {
    // Bottom vertex, 3 layers of 8 vertices, top vertex
    InitialiseSrcData();
    SetNextVertex(0, -0.5, 0);
    float radians1 = (float)(2 * M_PI / 8);
    float radians2 = (float)(2 * M_PI / 16);
    float midHeight = 0.4f;

    float radius1 = 0.5f;
    float radius2 = 0.3f + 0.08f;
    float radius3 = 0.3f;
    for (int n = 0; n < 8; ++n) {
        float c1 = std::cosf(n * radians1);
        float s1 = std::sinf(n * radians1);
        float c2 = std::cosf(n * radians1 + radians2);
        float s2 = std::sinf(n * radians1 + radians2);

        SetSpecificVertex(n + 1, radius1 * c1, midHeight, radius1 * s1);
        SetSpecificVertex(n + 1 + 8, radius2 * c2, 0.5f, radius2 * s2);
        SetSpecificVertex(n + 1 + 8 + 8, radius3 * c1, 0.525f, radius3 * s1);
    }

    // Vertex 25
    SetNextVertex(0, 0.55f, 0);

    // Bottom: vertex 0
    // Next ring: vertex 1 -> 8 (incl)
    // Next ring: vertex 9 -> 16 (incl)
    // Next ring: vertex 17 -> 24 (incl)
    // Top: vertex 25
    for (int n = 0; n < 8; ++n) {
        CreateNextSrcTriangle(0, n + 1, NextIndexWithinCircularRange(1, 9, n + 1));

    }

    for (int n = 0; n < 8; ++n) {
        CreateNextSrcTriangle(n + 1,n + 1 + 8,NextIndexWithinCircularRange(1, 9, n + 1));
        CreateNextSrcTriangle(NextIndexWithinCircularRange(1, 9, n + 1), n + 1 + 8, NextIndexWithinCircularRange(17, 25, n + 1 + 16));
        CreateNextSrcTriangle(n + 1, n + 1 + 16,n + 1 + 8);
        CreateNextSrcTriangle(n + 1 + 16,NextIndexWithinCircularRange(17, 25, n + 1 + 16),n + 1 + 8);
    }
    for (int n = 0; n < 8; ++n) {
        CreateNextSrcTriangle(n + 17, 25, NextIndexWithinCircularRange(17, 25, n + 17));
    }
}
