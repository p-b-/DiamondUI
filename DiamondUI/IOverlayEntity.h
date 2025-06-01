#pragma once
#include <string>

class IOverlayEntity {
public:
	virtual ~IOverlayEntity() {}
	virtual void SetText(std::string sText) = 0;
};