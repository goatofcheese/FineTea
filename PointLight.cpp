#ifndef LIGHT_H
#define LIGHT_H
#include "PointLight.h"
#endif

Vector3d PointLight::getPos(){
	return xl;
}

Vector3d PointLight::getDirection(Vector3d hitpoint){
	return (hitpoint - xl).normalize();
}

