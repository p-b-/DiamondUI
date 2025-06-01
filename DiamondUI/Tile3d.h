#pragma once
#include "SelfCreatingPolyhedral.h"
class Tile3d :
    public SelfCreatingPolyhedral
{
public:
	Tile3d();
	~Tile3d();
	virtual void Initialise(unsigned int unEntityId);
	virtual void Initialise(unsigned int unEntityId, Textures* pTextureCtrl);

	virtual void Deinitialise();

	virtual bool IsDirectional() const { return false; }
private:
	void CreateSourceVertices();

private:
	unsigned int m_unVBO;
	unsigned int m_unEBO;
};


