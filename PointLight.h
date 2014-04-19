#include "Vector.h"
#ifndef Light_H
#define Light_H
#include "Light.h"
#endif


class PointLight: public Light{

public:
	PointLight(Color color, Vector3d pos): Light(color), xl(pos){}

	Vector3d getPos();

	Vector3d getDirection(Vector3d hitpoint);

	void print(std::ostream& out) const { out << "Point: "<< xl; }

private:
	Vector3d xl;

};
