#pragma once
#include "SelfCreatingPolyhedral.h"
#include <vector>
#include <map>

class Sphere3d :
    public SelfCreatingPolyhedral {
public:
	Sphere3d(int nDetailLevel=1);
	virtual ~Sphere3d();

	virtual void Initialise(unsigned int unEntityId);
	virtual void Initialise(unsigned int unEntityId, Textures* pTextureCtrl);

	virtual void Deinitialise();
	virtual void PrepareToDrawShadows(const glm::vec3& vecLightPos, const glm::mat4& matLightSpace);
	virtual void PrepareToDraw(Camera* pCamera, const glm::vec3& vecLightPos, const glm::mat4& matLightSpace, unsigned int unDepthMapTexture);
	virtual bool IsDirectional() const { return false; }


	virtual void Draw(glm::mat4 matModel, bool bForShadowMap);

private:
	void CreateSourceVertices();
	void InitialiseIndexData(int triangleCount);
	void SetVertexNormals();

	void Subdivide(int detailLevel);
	bool GetIndexIfExistsFromMap(std::map<std::string, unsigned int>& mapMidpoints, std::string sKey, unsigned int& index);

	unsigned int GetMidpointIndex(std::map<std::string, unsigned int>& mapMidpoints, unsigned int index0, unsigned int index1);
	bool GetVec3IndexIfExists(glm::vec3 vecFind, unsigned int& index);

	void ExpandVerticesToRadius(float fRadius);


private:
	unsigned int m_unVBO;
	unsigned int m_unEBO;
	std::vector<float> m_vcVectors;
	int m_nDetailLevel;
};

