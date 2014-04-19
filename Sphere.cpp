#include "Sphere.h"

Vector3d Sphere::getCenter(){
	return center;
}

double Sphere::getRadius(){
	return radius;
}

RayHit Sphere::hits(Vector3d ur, Vector3d base, Vector3d& point){

	RayHit hit;
	double tclose, d, a;
	Vector3d basetoc, xclose; 

	
	//Detect if there will be a hit at all
	basetoc = center - base;
	tclose = ur * (basetoc);
	xclose = base + (tclose * ur);
	d = (xclose - center).norm();
	
	if(d > radius)	//too far no hit
	{
		hit.hit = false;
		return hit;
	}

	//else, calculate hitpoint
	a = sqrt(Sqr(radius) - Sqr(d));

	if(tclose - a <= 0){
		hit.hit = false;
		return hit;
	}

	
	hit.point = base + ((tclose - a) * ur);
	hit.normal = (hit.point - center).normalize();
	hit.hit = true;
	hit.distance = tclose - a;

	return hit;
}

NewMaterial Sphere::getMaterial(RayHit r){
	return mat;
}
