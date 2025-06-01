#pragma once
#include "SelfCreatingPolyhedral.h"
class Diamond : public SelfCreatingPolyhedral
{
public:
	Diamond();
	virtual ~Diamond();
	virtual void Initialise(unsigned int unEntityId);
	virtual void Initialise(unsigned int unEntityId, Textures* pTextureCtrl);
	virtual void Deinitialise();

private:
	void CreateSourceVertices();

private:
	unsigned int m_unVBO;
	unsigned int m_unEBO;
};

