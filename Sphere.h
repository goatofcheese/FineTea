#include "Vector.h"
#ifndef RGBApixel_H
#define RGBApixel_H
#include "RGBApixel.h"
#endif
#include "Material.h"
#ifndef Object_H
#define Object_H
#include "Object.h"
#endif

class Sphere: public Object{

public:
	
	Sphere(): center(0., 0., 1.), radius(1.), mat() {}
	Sphere(const Sphere& s): center(s.center), radius(s.radius), mat(s.mat){}
	Sphere(const Vector3d& c, const double r, NewMaterial m): center(c), radius(r), mat(m){}

	RayHit hits(Vector3d ur, Vector3d base, Vector3d& point);

	Vector3d getCenter();
	double getRadius();
	NewMaterial getMaterial(RayHit r);

private:
	Vector3d center;
	double radius;
	NewMaterial mat;

};
