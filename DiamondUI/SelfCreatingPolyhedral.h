#pragma once
#include "Entity3d.h"
#include "Shader.h"

class SelfCreatingPolyhedral : public Entity3d
{
public:
	SelfCreatingPolyhedral();
	virtual  ~SelfCreatingPolyhedral();

protected:
	float* CreateGLTriVerticesFromSourceData();
	int NextIndexWithinCircularRange(int rangeSt, int rangeEndExcl, int nextAfter);
	virtual bool IsDirectional() const { return false; }
	virtual bool IsDrawable() const { return true; }
	virtual void PrepareToDrawShadows(const glm::vec3& vecLightPos, const glm::mat4& matLightSpace);
	virtual void PrepareToDraw(Camera* pCamera, const glm::vec3& vecLightPos, const glm::mat4& matLightSpace, unsigned int unDepthMapTexture);
	virtual void Draw(glm::mat4 matModel, bool bForShadowMap);
	virtual void SetColour(float fR, float fG, float fB);
	
	void SetVertex(float* pVertices, int nIndex, float fX, float fY, float fZ);
	void SetVertex(float* pVertices, int nIndex, float fX, float fY, float fZ, float fTX, float fTY);
	void SetSpecificVertex(int nIndex, float fX, float fY, float fZ);
	void SetSpecificVertex(int nIndex, float fX, float fY, float fZ, float fTX, float fTY);
	void SetNextVertex(float fX, float fY, float fZ);
	void SetNextVertex(float fX, float fY, float fZ, float fTX, float fTY);
	void SetNextSrcIndex(unsigned int& punTriCount, unsigned int* punSrcIndices, int& nIndexOffset, unsigned int unVertex1, unsigned int unVertex2, unsigned int unVertex3);
	void CreateNextSrcTriangle(unsigned int unVertex1, unsigned int unVertex2, unsigned int unVertex3);

	void SetGLVertexFromVertices(float* pGLVertices, const std::vector<float>& vcSrcVertices, int nGLOffset, int nSrcVertex);
	void SetGLVertexFromVertices(float* pGLVertices, const std::vector<float>& vcSrcVertices, int nGLOffset, int nSrcVertex, glm::vec3 vecNormal);

	glm::vec3 NormalFromSrcVertices(unsigned int index1, unsigned int index2, unsigned int index3);

	void InitialiseSrcData();
	void DeleteSrcData();
	
	void CalculateBoundingSphereFromSourceVertices();
	glm::vec3 GetSrcVector(unsigned int unIndex);

private:
	void CalcMinMaxFromVertexElement(float& rfMin, float& rfMax, int nVertexCount, float* pfFirstVertexElement, int nStride);

protected:
	int m_nSrcVerticesStride;
	std::vector<float> m_vcSrcVertices;
	int m_nNextSrcVertex;
	std::vector<unsigned int> m_vcunSrcIndices;
	int m_nNextSrcIndex;
	int m_nSrcIndexMultiplier;

	unsigned int m_unVAO;
	Shader m_shader;
	Shader m_shaderShadow;

	float m_fRed;
	float m_fBlue;
	float m_fGreen;
};
