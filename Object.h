#include "Vector.h"

#ifndef RGBApixel_H
#define RGBApixel_H
#include "RGBApixel.h"
#endif

#include "NewMaterial.h"

#ifndef RayHit_H
#define RayHit_H
#include "RayHit.h"
#endif

class Object{

public:
	virtual RayHit hits(Vector3d ur, Vector3d base, Vector3d& point) = 0;
	virtual NewMaterial getMaterial(RayHit r) = 0;

};
