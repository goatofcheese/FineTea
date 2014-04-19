#include <iostream>
#ifndef RGBApixel_H
#define RGBApixel_H
#include "RGBApixel.h"
#endif
#include "Vector.h"
#include "Color.h"

class Light {

public:
	Light(): color(){}
	Light(Color col): color(col){}
	Light(const Light &l): color(l.color){}

	friend std::ostream &operator <<(std::ostream& out, const Light&);

	virtual void print(std::ostream& out) const { out << "Color "<< color; }
	
	Color getColor();
	
	virtual Vector3d getDirection(Vector3d hitpoint) = 0;
	virtual Vector3d getPos() = 0;	

private:
	Color color;

};
