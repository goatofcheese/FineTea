#include "Vector.h"
#ifndef Light_H
#define Light_H
#include "Light.h"
#endif

class ParallelLight: public Light {

public:
	ParallelLight(){}
	ParallelLight(Color color, Vector3d dir): Light(color), ul(dir){}

	Vector3d getPos(Vector3d hitpoint);

	Vector3d getDirection(Vector3d hitpoint);

private:
	Vector3d ul;

};
