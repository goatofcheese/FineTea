#ifndef RGBApixel_H
#define RGBApixel_H
#include "RGBApixel.h"
#endif

#include <iostream>

class Material{

public:
	Material(): color(), diffuse(1.0), specular(1.0){}
	Material(const Material& m): color(m.color), diffuse(m.diffuse), specular(m.specular) {}
	Material(RGBApixel col, double d, double s): color(col), diffuse(d), specular(s){}

	friend std::ostream& operator <<(std::ostream& out, const Material&);

	RGBApixel getColor();
	double getDiffuse();
	double getSpecular();

private:
	RGBApixel color;
	double diffuse;
	double specular;
};
