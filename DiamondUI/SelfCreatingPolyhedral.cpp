#include "SelfCreatingPolyhedral.h"
#include "Camera.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

SelfCreatingPolyhedral::SelfCreatingPolyhedral() {
	m_nNextSrcVertex = 0;
	m_nNextSrcIndex = 0;
	// This would be 5 for vertices with textures, would need mechanisms to set that.
	m_nSrcIndexMultiplier = 3;
	m_nSrcVerticesStride = 3;

	m_fRed = 1.0f;
	m_fGreen = 0.5f;
	m_fBlue = 0.31f;
}

SelfCreatingPolyhedral::~SelfCreatingPolyhedral() {
	DeleteSrcData();
	glDeleteVertexArrays(1, &m_unVAO);
}

void SelfCreatingPolyhedral::SetVertex(float* pVertices, int nIndex, float fX, float fY, float fZ) {
	pVertices += nIndex * 3;
	pVertices[0] = fX;
	pVertices[1] = fY;
	pVertices[2] = fZ;
}

void SelfCreatingPolyhedral::SetVertex(float* pVertices, int nIndex, float fX, float fY, float fZ, float fTX, float fTY) {
	pVertices += nIndex * 5;
	pVertices[0] = fX;
	pVertices[1] = fY;
	pVertices[2] = fZ;
	pVertices[3] = fTX;
	pVertices[4] = fTY;
}

void SelfCreatingPolyhedral::SetSpecificVertex(int nIndex, float fX, float fY, float fZ) {
	for (int n = m_nNextSrcVertex; n <= nIndex; ++n) {
		SetNextVertex(0.0f, 0.0f, 0.0f);
	}
	int offset = nIndex * m_nSrcVerticesStride;
	m_vcSrcVertices[offset + 0] = fX;
	m_vcSrcVertices[offset + 1] = fY;
	m_vcSrcVertices[offset + 2] = fZ;
	if (nIndex >= m_nNextSrcVertex) {
		m_nNextSrcVertex = nIndex + 1;
	}
}

void SelfCreatingPolyhedral::SetSpecificVertex(int nIndex, float fX, float fY, float fZ, float fTX, float fTY) {
	for (int n = m_nNextSrcVertex; n <= nIndex; ++n) {
		SetNextVertex(0.0f, 0.0f, 0.0f, 0.0f,0.0f);
	}
	int offset = nIndex * m_nSrcVerticesStride;
	m_vcSrcVertices[offset + 0] = fX;
	m_vcSrcVertices[offset + 1] = fY;
	m_vcSrcVertices[offset + 2] = fZ;
	m_vcSrcVertices[offset + 3] = fTX;
	m_vcSrcVertices[offset + 4] = fTY;
	if (nIndex >= m_nNextSrcVertex) {
		m_nNextSrcVertex = nIndex + 1;
	}
}

void SelfCreatingPolyhedral::SetNextVertex(float fX, float fY, float fZ) {
	m_vcSrcVertices.push_back(fX);
	m_vcSrcVertices.push_back(fY);
	m_vcSrcVertices.push_back(fZ);
	if (m_nSrcVerticesStride > 3) {
		for (int index = 3; index < m_nSrcVerticesStride; ++index) {
			m_vcSrcVertices.push_back(0.0f);
		}
	}
	++m_nNextSrcVertex;
}

void SelfCreatingPolyhedral::SetNextVertex(float fX, float fY, float fZ, float fTX, float fTY) {
	m_vcSrcVertices.push_back(fX);
	m_vcSrcVertices.push_back(fY);
	m_vcSrcVertices.push_back(fZ);
	m_vcSrcVertices.push_back(fTX);
	m_vcSrcVertices.push_back(fTY);
	if (m_nSrcVerticesStride > 5) {
		for (int index = 5; index < m_nSrcVerticesStride; ++index) {
			m_vcSrcVertices.push_back(0.0f);
		}
	}
	++m_nNextSrcVertex;
}

void SelfCreatingPolyhedral::SetGLVertexFromVertices(float* pGLVertices, const std::vector<float>& vcSrcVertices, int nGLOffset, int nSrcVertex) {
	pGLVertices += nGLOffset;
	int offset = nSrcVertex * m_nSrcVerticesStride;
	pGLVertices[0] = vcSrcVertices[offset + 0];
	pGLVertices[1] = vcSrcVertices[offset + 1];
	pGLVertices[2] = vcSrcVertices[offset + 2];
}

void SelfCreatingPolyhedral::SetGLVertexFromVertices(float* pGLVertices, const std::vector<float>& vcSrcVertices, int nGLOffset, int nSrcVertex, glm::vec3 vecNormal) {
	pGLVertices += nGLOffset;
	int offset = nSrcVertex * m_nSrcVerticesStride;
	pGLVertices[0] = vcSrcVertices[offset + 0];
	pGLVertices[1] = vcSrcVertices[offset + 1];
	pGLVertices[2] = vcSrcVertices[offset + 2];
	pGLVertices[3] = vecNormal.x;
	pGLVertices[4] = vecNormal.y;
	pGLVertices[5] = vecNormal.z;
}

glm::vec3 SelfCreatingPolyhedral::NormalFromSrcVertices(unsigned int index1, unsigned int index2, unsigned int index3) {
	int offset1 = index1 * m_nSrcIndexMultiplier;
	int offset2 = index2 * m_nSrcIndexMultiplier;
	int offset3 = index3 * m_nSrcIndexMultiplier;
	glm::vec3 vecPos1(m_vcSrcVertices[offset1 + 0], m_vcSrcVertices[offset1 + 1], m_vcSrcVertices[offset1 + 2]);
	glm::vec3 vecPos2(m_vcSrcVertices[offset2 + 0], m_vcSrcVertices[offset2 + 1], m_vcSrcVertices[offset2 + 2]);
	glm::vec3 vecPos3(m_vcSrcVertices[offset3 + 0], m_vcSrcVertices[offset3 + 1], m_vcSrcVertices[offset3 + 2]);
	return NormalFromVertices(vecPos1, vecPos2, vecPos3);
}

int SelfCreatingPolyhedral::NextIndexWithinCircularRange(int rangeSt, int rangeEndExcl, int nextAfter) {
    nextAfter++;
    if (nextAfter >= rangeEndExcl) {
        int stepsAfterRange = nextAfter - rangeEndExcl;
        return rangeSt + stepsAfterRange;
    }
    return nextAfter;
}

void SelfCreatingPolyhedral::SetNextSrcIndex(unsigned int& punTriCount, unsigned int* punSrcIndices, int& nIndexOffset, unsigned int unVertex1, unsigned int unVertex2, unsigned int unVertex3) {
    punSrcIndices[nIndexOffset + 0] = unVertex1;
    punSrcIndices[nIndexOffset + 1] = unVertex2;
    punSrcIndices[nIndexOffset + 2] = unVertex3;
    ++punTriCount;
    nIndexOffset += 3;
}

void SelfCreatingPolyhedral::CreateNextSrcTriangle(unsigned int unVertex1, unsigned int unVertex2, unsigned int unVertex3) {
	m_vcunSrcIndices.push_back(unVertex1);
	m_vcunSrcIndices.push_back(unVertex2);
	m_vcunSrcIndices.push_back(unVertex3);
	++m_unTriCount;
	m_nNextSrcIndex += 3;
}

float* SelfCreatingPolyhedral::CreateGLTriVerticesFromSourceData() {
	// per triangle, 3 vertices per side, 6 floats for vertex (pos, normal)
	float* verticesForGL = new float[m_unTriCount * 3 * 6];
	for (unsigned int nTri = 0; nTri < m_unTriCount; ++nTri) {
		glm::vec3 normal = NormalFromSrcVertices(m_vcunSrcIndices[nTri * 3 + 0], m_vcunSrcIndices[nTri * 3 + 1], m_vcunSrcIndices[nTri * 3 + 2]);
		int destIndex = nTri * 3 * 3 * 2;
		for (int nVertex = 0; nVertex < 3; ++nVertex) {

			int srcIndex = m_vcunSrcIndices[nTri * 3 + nVertex];
			SetGLVertexFromVertices(verticesForGL, m_vcSrcVertices, destIndex, srcIndex, normal);
			destIndex += 3 * 2;
		}
	}
	return verticesForGL;
}


void SelfCreatingPolyhedral::InitialiseSrcData() {
	m_nNextSrcVertex = 0;
	m_nNextSrcIndex= 0;
	m_unTriCount = 0;
}

void SelfCreatingPolyhedral::DeleteSrcData() {
	m_vcunSrcIndices.clear();
}

glm::vec3 SelfCreatingPolyhedral::GetSrcVector(unsigned int unIndex) {
	int offset = unIndex * m_nSrcVerticesStride;
	return glm::vec3(m_vcSrcVertices[offset + 0], m_vcSrcVertices[offset + 1], m_vcSrcVertices[offset + 2]);
}


void SelfCreatingPolyhedral::CalculateBoundingSphereFromSourceVertices() {
	int nVertexCount = m_nNextSrcVertex;
	float xLeftmost;
	float xRightmost;
	float yBottom;
	float yTop;
	float zFurthest;
	float zNearest;
	float* pfSrcVertices = &m_vcSrcVertices[0];
	CalcMinMaxFromVertexElement(xLeftmost, xRightmost, nVertexCount, pfSrcVertices + 0, m_nSrcIndexMultiplier);
	CalcMinMaxFromVertexElement(yBottom, yTop, nVertexCount, pfSrcVertices + 1, m_nSrcIndexMultiplier);
	CalcMinMaxFromVertexElement(zFurthest, zNearest, nVertexCount, pfSrcVertices + 2, m_nSrcIndexMultiplier);

	glm::vec3 vecCentre = glm::vec3((xLeftmost + xRightmost) / 2, (yBottom + yTop) / 2, (zFurthest + zNearest) / 2);

	float radius = 0.0f;

	for (int i = 0; i < m_nNextSrcVertex; ++i) {
		glm::vec3 v = GetSrcVector(i);
		glm::vec3 centreToV = v - vecCentre;
		float dist = glm::length(centreToV);
		if (dist > radius) {
			radius = dist;
		}
	}
	m_boundingSphere = BoundingSphere(vecCentre, radius);
}

void SelfCreatingPolyhedral::CalcMinMaxFromVertexElement(float& rfMin, float& rfMax, int nVertexCount, float* pfFirstVertexElement, int nStride) {
	float* pfVertexElement = pfFirstVertexElement;
	rfMax = pfFirstVertexElement[0];
	rfMin = rfMax;
	for (int i = 1; i < nVertexCount; ++i) {
		pfVertexElement += nStride;
		if (pfVertexElement[0] > rfMax) {
			rfMax = pfVertexElement[0];
		} else if (pfVertexElement[0] < rfMin) {
			rfMin = pfVertexElement[0];
		}
	}
}

void SelfCreatingPolyhedral::PrepareToDrawShadows(const glm::vec3& vecLightPos, const glm::mat4& matLightSpace) {
	glBindVertexArray(m_unVAO);
	m_shaderShadow.Use();
	m_shaderShadow.SetMat4("lightSpaceTransform", matLightSpace);
}

void SelfCreatingPolyhedral::PrepareToDraw(Camera* pCamera, const glm::vec3& vecLightPos, const glm::mat4& matLightSpace, unsigned int unDepthMapTexture) {
	glBindVertexArray(m_unVAO);
	m_shader.Use();
	m_shader.SetMat4("lightSpaceTransform", matLightSpace);
	m_shader.SetVec3("objectColor", m_fRed, m_fGreen, m_fBlue);
	m_shader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
	m_shader.SetVec3("lightPos", vecLightPos);
	m_shader.SetVec3("viewPos", pCamera->GetPosition());

	m_shader.SetMat4("projection", pCamera->GetProjectionMatrix());
	m_shader.SetMat4("view", pCamera->GetViewMatrix());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, unDepthMapTexture);
}

void SelfCreatingPolyhedral::Draw(glm::mat4 matModel, bool bForShadowMap) {
	Shader* pShader = &m_shader;
	if (bForShadowMap) {
		pShader = &m_shaderShadow;
	}
	pShader->SetMat4("model", matModel);

	glm::mat3 matNormal = glm::transpose(glm::inverse(matModel));
	pShader->SetMat3("normalMatrix", matNormal);

	glDrawArrays(GL_TRIANGLES, 0, m_unTriCount * 3);
}

void SelfCreatingPolyhedral::SetColour(float fR, float fG, float fB) {
	m_fRed = fR;
	m_fGreen = fG;
	m_fBlue = fB;
}