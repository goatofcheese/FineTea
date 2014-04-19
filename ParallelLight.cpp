#include "ParallelLight.h"

Vector3d ParallelLight::getDirection(Vector3d hitpoint){
	return (hitpoint - ul).normalize();
}
