#pragma once
#include "Plane.h"
class Camera;

class Frustum
{
	enum PlaneId {
		ePlaneId_Near,
		ePlaneId_Far,
		ePlaneId_Left,
		ePlaneId_Right,
		ePlaneId_Top,
		ePlaneId_Bottom
	};

public:
	Frustum();
	void CreateFromCamera(Camera* pCam);
	bool ContainsSphere(const BoundingSphere& s) const;

protected:
	Plane m_aPlanes[6];
};

