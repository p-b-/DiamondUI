#pragma once
#include <list>
#include <vector>

class Entity
{
public:
	Entity();
	virtual ~Entity() {}
	void IncrementRefCount();
	void DecrementRefCount();
	unsigned int GetRefCount() const { return m_unRefCount; }

	static void ClearUnreferencedEntities();

protected:
	unsigned int m_unRefCount;

	static std::vector<Entity* > s_vcUnreferencedEntities;
};

