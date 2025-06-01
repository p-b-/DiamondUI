#pragma once

#include <string>

class IOverlayView {
public:
	virtual ~IOverlayView() {}

	virtual int AddTextLabel(std::string sLabel, float fWidth = 0.0f) = 0;
	virtual int AddTextField(std::string sText, float fWidth = 0.0f) = 0;
};