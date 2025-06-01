#include "Entity.h"

// TODO Lock this if app goes multi-threaded
std::vector<Entity* >Entity::s_vcUnreferencedEntities;
// TODO Occassionally clear out unreferenced entities

Entity::Entity() {
	m_unRefCount = 0;
}

void Entity::IncrementRefCount() {
	++m_unRefCount;
}

void Entity::DecrementRefCount() {
	--m_unRefCount;
	if (m_unRefCount == 0) {
		s_vcUnreferencedEntities.push_back(this);
	}
}

void Entity::ClearUnreferencedEntities() {
	for (auto e : s_vcUnreferencedEntities) {
		delete e;
	}
	s_vcUnreferencedEntities.clear();
}