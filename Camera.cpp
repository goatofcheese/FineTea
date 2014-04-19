/*
	Implementation of the Camera class
*/

#include "Camera.h"

std::ostream& operator<<(std::ostream& out, const Camera& c){
	return out << "Pinhole: " << c.pinhole 
		<< "Direction: " << c.dir
		<< "Up Vector: " << c.up
		<< "Focal Distance: " << c.focalDistance
		<<std::endl;

}

Vector3d Camera::getPinhole(){
	return pinhole;
}

Vector3d Camera::getDir(){
	return dir;
}

Vector3d Camera::getUp(){
	return up;
}

double Camera::getFocalDistance(){
	return focalDistance;
}
