#include "OverlayEntity.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void OverlayEntity::SetBoundingRect(int nX, int nY, OverlayEntityRelativeTo eRelativeTo, int nWidth, int nHeight) {
    if (nWidth != 0) {
        m_nWidth = nWidth;
    }
    if (nHeight != 0) {
        m_nHeight = nHeight;
    } 
    if (eRelativeTo == eOERTFromTopLeft || eRelativeTo == eOERTFromBottomLeft) {
        m_nX = nX;
    }
    else {
        m_nX = s_nScreenWidth - m_nWidth;
    }

    if (eRelativeTo == eOERTFromBottomLeft || eRelativeTo == eOERTFromBottomRight) {
        m_nY = nY;
    } 
    else {
        m_nY = s_nScreenHeight - m_nHeight - nY;
    }
}

void OverlayEntity::SetBoundingRect(OverlayEntityRelativeTo eRelativeTo, OverlayEntity* pOtherEntity) {
    int xSpacing = 5;
    int ySpacing = 5;
    if (eRelativeTo == eOERTAbove || eRelativeTo == eOERTBelow) {
        m_nX = pOtherEntity->m_nX;
    }
    else if (eRelativeTo == eOERTLeftOf) {
        m_nX = pOtherEntity->m_nX - m_nWidth-xSpacing;
    }
    else {
        // Right
        m_nX = pOtherEntity->m_nX + pOtherEntity->m_nWidth + xSpacing;
    }

    if (eRelativeTo == eOERTLeftOf || eRelativeTo == eOERTRightOf) {
        m_nY = pOtherEntity->m_nY;
    }
    else if (eRelativeTo == eOERTAbove) {

        m_nY = pOtherEntity->m_nY + pOtherEntity->m_nHeight + ySpacing;
    }
    else {
        // Below
        m_nY = pOtherEntity->m_nY - m_nHeight - ySpacing;
    }
}

void OverlayEntity::Render() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    s_shader.Use();
    glBindVertexArray(s_unVAOQuad);
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(m_nX, m_nY, 0));
    transform = glm::scale(transform, glm::vec3((float)m_nWidth, (float)m_nHeight-1,0));
    s_shader.SetMat4("transform", transform);
    s_shader.SetVec4("overlayColour", m_rgbaOverlay);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}