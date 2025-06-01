#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

#include "Textures.h"

Textures::Textures() {
}

Textures::~Textures() {
    for (auto d : m_mpTextureAtlases) {
        glDeleteTextures(1, &d.second->unTextureId);
        delete d.second;
    }
    m_mpTextureAtlases.clear();
}

unsigned int Textures::CreateTexture(const char* pzFilepath, const std::string& name, bool useAlpha, bool flipVertically) {
    unsigned int textureId = GetTextureIdFromName(name);
    if (textureId != 0) {
        return textureId;
    }
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    if (flipVertically) {
        stbi_set_flip_vertically_on_load(true); 
    }
    unsigned char* data = stbi_load(pzFilepath, &width, &height, &nrChannels, 0);
    if (flipVertically) {
        stbi_set_flip_vertically_on_load(false);
    }
    if (data)
    {
        unsigned int colourFormat = GL_RGB;
        if (useAlpha) {
            colourFormat = GL_RGBA;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, colourFormat, width, height, 0, colourFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    m_mapTextureNameToId[name] = textureId;

    return textureId;
}

unsigned int Textures::GetTextureIdFromName(const std::string& name) {
    auto iter = m_mapTextureNameToId.find(name);
    if (iter == m_mapTextureNameToId.end()) {
        return 0;
    }
    return iter->second;
}

unsigned int Textures::CreateTextureAtlas(int nSubtextureWidth, int nSubTextureHeight, int nHorizCount, int nVertCount, unsigned int unColourFormat) {
    unsigned int id = m_unNextAtlasId++;

    unsigned int unAtlasTextureId = 0;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &unAtlasTextureId);
    glBindTexture(GL_TEXTURE_2D, unAtlasTextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int sz = nSubtextureWidth * nHorizCount * 4 * nSubTextureHeight * nVertCount * 4;
    unsigned char* pixels = new unsigned char[sz];
    memset(pixels, 0, sz);


    glTexImage2D(GL_TEXTURE_2D, 0, unColourFormat, nSubtextureWidth*nHorizCount, nSubTextureHeight*nVertCount, 0, unColourFormat, GL_UNSIGNED_BYTE, pixels);
    Atlas* a = new Atlas{ nSubtextureWidth, nSubTextureHeight, nHorizCount, nVertCount,unColourFormat, unAtlasTextureId, 0 };
    m_mpTextureAtlases[id] = a;
    delete[] pixels;

    return id;
}

unsigned int Textures::GetTextureIdForTextureAtlas(unsigned int unTextureAtlasId) {
    Atlas* pAtlas = GetAtlas(unTextureAtlasId);
    return pAtlas->unTextureId;
}

bool Textures::GetTexelForTextureInAtlas(unsigned int unAtlasId, unsigned int unSubTextureId, float* pxOrigin, float* pyOrigin, float* pnWidth, float* pnHeight) {
    Atlas* pAtlas = GetAtlas(unAtlasId);
    if (pAtlas == nullptr) {
        return false;
    }
    auto find = pAtlas->mpIdToFlattenedCoords.find(unSubTextureId);
    if (find == pAtlas->mpIdToFlattenedCoords.end()) {
        return false;
    }
    int nFlattenedCoord = find->second;
    float x = static_cast<float>((nFlattenedCoord % pAtlas->nHorizCount) * pAtlas->nSubTextureWidth);
    float y = static_cast<float>((nFlattenedCoord / pAtlas->nHorizCount) * pAtlas->nSubTextureHeight);
    *pxOrigin = x / (pAtlas->nSubTextureWidth * pAtlas->nHorizCount);
    *pyOrigin = y / (pAtlas->nSubTextureHeight * pAtlas->nVertCount);
    *pnWidth = 1.0f / pAtlas->nHorizCount;
    *pnHeight = 1.0f / pAtlas->nVertCount;
    return true;
}

void Textures::DeleteTextureAtlas(unsigned int unTextureAtlasId) {
    Atlas* pAtlas = GetAtlas(unTextureAtlasId);
    if (pAtlas != nullptr) {
        DeleteAtlas(unTextureAtlasId);
    }
}

bool Textures::AddTextureToAtlas(unsigned int unAtlasId, const char* pzFilepath, unsigned int unId) {
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* pData = stbi_load(pzFilepath, &width, &height, &nrChannels, 0);

    if (!pData)
    {
        std::cerr << "Failed to load texture for tetxure atlas" << std::endl;
    }

    Atlas* pAtlas = GetAtlas(unAtlasId);
    if (pAtlas == nullptr) {
        return false;
    }
    int x = (pAtlas->nNextFlattenedCoord % pAtlas->nHorizCount) * pAtlas->nSubTextureWidth;
    int y = (pAtlas->nNextFlattenedCoord / pAtlas->nHorizCount) * pAtlas->nSubTextureHeight;
    pAtlas->mpIdToFlattenedCoords[unId] = pAtlas->nNextFlattenedCoord;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pAtlas->unTextureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, pAtlas->nSubTextureWidth, pAtlas->nSubTextureHeight, pAtlas->unColourFormat, GL_UNSIGNED_BYTE, pData);

    pAtlas->nNextFlattenedCoord++;
    stbi_image_free(pData);

    return true;
}

Textures::Atlas* Textures::GetAtlas(unsigned int unAtlasId) {
    auto findAtlas = m_mpTextureAtlases.find(unAtlasId);
    if (findAtlas == m_mpTextureAtlases.end()) {
        return nullptr;
    }
    return findAtlas->second;
}

void Textures::DeleteAtlas(unsigned int unAtlasId) {
    auto findAtlas = m_mpTextureAtlases.find(unAtlasId);
    if (findAtlas != m_mpTextureAtlases.end()) {
        glDeleteTextures(1, &findAtlas->second->unTextureId);
        delete findAtlas->second;
        m_mpTextureAtlases.erase(findAtlas);
    }
}
