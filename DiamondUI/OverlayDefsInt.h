#pragma once

// TODO Add 'l' to end of this filename

class IOverlayIntl;
class IOverlayEnvIntl;

class ScrollTarget {
public:
	virtual ~ScrollTarget() {}

	virtual void SetVerticalScrollOffset(float yOffset) = 0;
};

class IElementCollectionContainer {
public:
	virtual ~IElementCollectionContainer() {}

	virtual float GetDisplayableHeight() = 0;
	virtual float GetDisplayableWidth() = 0;
	virtual bool GetCentreELementsHorizontally() const = 0;
	virtual IOverlayIntl* GetOverlay() = 0;
	virtual const IOverlayEnvIntl* GetEnvironment() const = 0;
	virtual bool GetContainerIsTabNotView() = 0;
	virtual bool GetContainerAspectIsVertical() = 0;
};