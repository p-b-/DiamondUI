#include "Sphere3d.h"
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
#include <sstream>
#include <algorithm>

Sphere3d::Sphere3d(int nDetailLevel) {
    m_nDetailLevel = nDetailLevel;
    m_unEBO = 0;
    m_unVAO = 0;
    m_unVBO = 0;

    m_unEntityId = 0;
}

Sphere3d::~Sphere3d() {
    Deinitialise();
}

void Sphere3d::Initialise(unsigned int unEntityId) {
    Initialise(unEntityId, nullptr);
}

void Sphere3d::Initialise(unsigned int unEntityId, Textures* pTextureCtrl) {
    m_unEntityId = unEntityId;

    // Create source vertices, that are just the points of the polygon
    CreateSourceVertices();
    CalculateBoundingSphereFromSourceVertices();
    // These vertices are then replicated and given normals, to construct the vertices passed to gl

    glGenVertexArrays(1, &m_unVAO);
    glGenBuffers(1, &m_unVBO);

    // The Vertex Array Object stores the state associated with the VBO, so the following code is stored, ensuring
    //  the binding of the vbo, and the setup of the vertex data, does not need to be repeated
    glBindVertexArray(m_unVAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_unEBO);
    unsigned int* pIndices = &m_vcunSrcIndices[0];
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3 * m_unTriCount, pIndices, GL_STATIC_DRAW);

    // The Vertex Buffer Object is of type GL_ARRAY_BUFFER. This binds the VBO identified by m_unVBO so that
    //  future operations on GL_ARRAY_BUFFER are performed on this VBO.
    glBindBuffer(GL_ARRAY_BUFFER, m_unVBO);
    float* pSrcVertices = &m_vcSrcVertices[0];
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_nSrcVerticesStride * m_nNextSrcVertex, pSrcVertices, GL_STATIC_DRAW);

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

    if (m_bReceivesLight) {
        m_shader.InitialiseShaders("polyhedral_vert.glsl", "polyhedral_frag.glsl");
        m_shaderShadow.InitialiseShaders("polyhedral_shadow_vert.glsl", "polyhedral_shadow_frag.glsl");
        m_shader.Use();
        m_shader.SetInt("shadowMap", 0);

    }
    else {
        m_shader.InitialiseShaders("light_vert.glsl", "light_frag.glsl");
    }

    m_vcunSrcIndices.clear();
    DeleteSrcData();
}

void Sphere3d::Deinitialise() {
    glDeleteVertexArrays(1, &m_unVAO);
    m_unVAO = 0;
    glDeleteBuffers(1, &m_unVBO);
    m_unVBO = 0;

    glDeleteBuffers(1, &m_unEBO);
    m_unEBO = 0;
}

void Sphere3d::InitialiseIndexData(int triangleCount) {
    glGenBuffers(1, &m_unEBO);
}

void Sphere3d::CreateSourceVertices() {
    m_nSrcVerticesStride = 6;
    InitialiseSrcData();
    InitialiseIndexData(20);

    // Create the 12 vertices of a 20-sided icosahedron.
    SetNextVertex(0, -0.5, 0);
    float radians36Deg = (float)(2.0f * M_PI / 10.0f);
    float yOne = 0.5f * -sinf(radians36Deg);
    float yTwo = 0.5f * sinf(radians36Deg);

    float radians1 = (float)(2 * M_PI / 5);
    float radians2 = (float)(2 * M_PI / 10);
    float midHeight = 0.4f;
    float radius = 0.5f;
    for (int n = 0; n < 5; ++n) {
        float c1 = std::cosf(n * radians1);
        float s1 = std::sinf(n * radians1);
        float c2 = std::cosf(n * radians1 + radians2);
        float s2 = std::sinf(n * radians1 + radians2);

        std::cout << "Vertex " << (n + 1) << " (" << (radius * c1) << ", " << yOne << ", " << (radius * s1) << ")" << std::endl;;
        std::cout << "Vertex " << (n + 1+5) << " (" << (radius * c2) << ", " << yTwo << ", " << (radius * s2) << ")" << std::endl;;
        SetSpecificVertex(n + 1, radius * c1, yOne, radius * s1);
        SetSpecificVertex(n + 1 + 5, radius * c2, yTwo, radius * s2);
    }

    // Vertex 11 (12th vertex)
    SetNextVertex(0, 0.5f, 0);

    // Create the 20 triangles of the icosahedron

    // Bottom: vertex 0
    // Next ring: vertex 1 -> 5 (incl)
    // Next ring: vertex 6 -> 10 (incl)
    // Top: vertex 11
    int vertex1 = 0;
    int vertex2 = 1;
    int vertex3 = 2;
    for (int n = 0; n < 5; ++n) {
        CreateNextSrcTriangle(vertex1, vertex2, vertex3);
        vertex2 = NextIndexWithinCircularRange(1, 6, vertex2);
        vertex3 = NextIndexWithinCircularRange(1, 6, vertex3);
    }
    vertex1 = 1;
    vertex2 = 6;
    vertex3 = 10;
    std::cout << std::endl;
    for (int n = 0; n < 5; ++n) {
        CreateNextSrcTriangle(vertex1, vertex3, vertex2);
        vertex1 = NextIndexWithinCircularRange(1, 6, vertex1);
        vertex2 = NextIndexWithinCircularRange(6, 11, vertex2);
        vertex3 = NextIndexWithinCircularRange(6, 11, vertex3);
    }

    vertex1 = 1;
    vertex2 = 6;
    vertex3 = 2;
    std::cout << std::endl;
    for (int n = 0; n < 5; ++n) {
        CreateNextSrcTriangle(vertex1, vertex2, vertex3);
        vertex1 = NextIndexWithinCircularRange(1, 6, vertex1);
        vertex2 = NextIndexWithinCircularRange(6, 11, vertex2);
        vertex3 = NextIndexWithinCircularRange(1, 6, vertex3);
    }

    vertex1 = 11;
    vertex2 = 6;
    vertex3 = 10;
    std::cout << std::endl;
    for (int n = 0; n < 5; ++n) {
        CreateNextSrcTriangle(vertex1, vertex2, vertex3);
        vertex2 = NextIndexWithinCircularRange(6, 11, vertex2);
        vertex3 = NextIndexWithinCircularRange(6, 11, vertex3);
    }

    // Subdivide each triangle 4 times, repeat 3 times (20*4 = 80, 80*4=320, 320*4 = 1280 triangles)
    Subdivide(m_nDetailLevel);

    // Expand the vertices so they all touch a radius of the sphere
    ExpandVerticesToRadius(0.5f);
    SetVertexNormals();
}

void Sphere3d::Subdivide(int detailLevel) {
    for (int nDetailIndex = 0; nDetailIndex < detailLevel; ++nDetailIndex) {
        std::map<std::string, unsigned int> midpointIndices;
        std::vector<unsigned int> srcTriangles = m_vcunSrcIndices;
        int indexCount = static_cast<int>(srcTriangles.size());
        m_vcunSrcIndices.clear();
        m_nNextSrcIndex = 0;
        m_unTriCount = 0;
        for (int i = 0; i < indexCount - 2; i += 3) {
            unsigned int index0 = srcTriangles[i];
            unsigned int index1 = srcTriangles[i + 1];
            unsigned int index2 = srcTriangles[i + 2];

            unsigned int m01 = GetMidpointIndex(midpointIndices, index0, index1);
            unsigned int m12 = GetMidpointIndex(midpointIndices, index1, index2);
            unsigned int m02 = GetMidpointIndex(midpointIndices, index2, index0);

            CreateNextSrcTriangle(index0, m01, m02);
            CreateNextSrcTriangle(index1, m12, m01);
            CreateNextSrcTriangle(index2, m02, m12);
            CreateNextSrcTriangle(m02, m01, m12);
        }
    }
}

bool Sphere3d::GetIndexIfExistsFromMap(std::map<std::string, unsigned int>& mapMidpoints, std::string sKey, unsigned int& index) {
    auto iter = mapMidpoints.find(sKey);
    if (iter == mapMidpoints.end()) {
        return false;
    }
    index = iter->second;
    return true;
}

bool Sphere3d::GetVec3IndexIfExists(glm::vec3 vecFind, unsigned int& index) {
    for (int n = 0; n < m_nNextSrcVertex; ++n) {
        glm::vec3 vecCompareTo = GetSrcVector(n);

        glm::vec3 vecDiff = vecFind - vecCompareTo;
        float length = glm::length(vecDiff);
        if (length < 0.0001) {
            index = n;
            return true;
        }
    }

    return false;
}

unsigned int Sphere3d::GetMidpointIndex(std::map<std::string, unsigned int>& mapMidpoints, unsigned int index0, unsigned int index1) {
    std::ostringstream strStream;
    strStream << std::min(index0, index1);
    strStream << "_";
    strStream << std::max(index0, index1);
    std::string edgeKey = strStream.str();

    unsigned int midpointIndex;


    if (!GetIndexIfExistsFromMap(mapMidpoints, edgeKey, midpointIndex))
    {
        glm::vec3 v0 = GetSrcVector(index0);
        glm::vec3 v1 = GetSrcVector(index1);

        glm::vec3 midpoint = (v0 + v1) / 2.0f;

        if (!GetVec3IndexIfExists(midpoint, midpointIndex)) {
            midpointIndex = m_nNextSrcVertex;
            SetNextVertex(midpoint.x, midpoint.y, midpoint.z);
            mapMidpoints[edgeKey.c_str()] = midpointIndex;
        }
    }

    return midpointIndex;
}

void Sphere3d::SetVertexNormals() {
    for (int n = 0; n < m_nNextSrcVertex; ++n) {
        int offset = n * m_nSrcVerticesStride;
        float x = m_vcSrcVertices[offset + 0];
        float y = m_vcSrcVertices[offset + 1];
        float z = m_vcSrcVertices[offset + 2];
        glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
        m_vcSrcVertices[offset + 3] = normal.x;
        m_vcSrcVertices[offset + 4] = normal.y;
        m_vcSrcVertices[offset + 5] = normal.z;
    }
}

void Sphere3d::ExpandVerticesToRadius(float fRadius) {
    for (int n = 0; n < m_nNextSrcVertex; ++n) {
        glm::vec3 v = glm::normalize(GetSrcVector(n)) * fRadius;
        int offset = n * m_nSrcVerticesStride;
        m_vcSrcVertices[offset + 0] = v.x;
        m_vcSrcVertices[offset + 1] = v.y;
        m_vcSrcVertices[offset + 2] = v.z;
    }
}

void Sphere3d::PrepareToDrawShadows(const glm::vec3& vecLightPos, const glm::mat4& matLightSpace) {
    glBindVertexArray(m_unVAO);
    if (m_bReceivesLight) {
        m_shaderShadow.Use();
        m_shaderShadow.SetMat4("lightSpaceTransform", matLightSpace);
    }
}

void Sphere3d::PrepareToDraw(Camera* pCamera, const glm::vec3& vecLightPos, const glm::mat4& matLightSpace, unsigned int unDepthMapTexture) {
    glBindVertexArray(m_unVAO);
    m_shader.Use();

    if (m_bReceivesLight) {
        m_shader.SetMat4("lightSpaceTransform", matLightSpace);
        m_shader.SetVec3("objectColor", 1.0f, 0.5f, 0.31f);
        m_shader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
        m_shader.SetVec3("lightPos", vecLightPos);
        m_shader.SetVec3("viewPos", pCamera->GetPosition());
    }
    else {
        m_shader.SetVec3("objectColor", 1.0f, 1.0f, 1.0f);
    }

    m_shader.SetMat4("projection", pCamera->GetProjectionMatrix());
    m_shader.SetMat4("view", pCamera->GetViewMatrix());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, unDepthMapTexture);
}

void Sphere3d::Draw(glm::mat4 matModel, bool bForShadowMap) {
    Shader* pShader = &m_shader;
    if (bForShadowMap) {
        pShader = &m_shaderShadow;
    }
    pShader->SetMat4("model", matModel);

    if (m_bReceivesLight) {
        glm::mat3 matNormal = glm::transpose(glm::inverse(matModel));
        pShader->SetMat3("normalMatrix", matNormal);
    }

    glDrawElements(GL_TRIANGLES, m_unTriCount * 3, GL_UNSIGNED_INT, 0);
}
