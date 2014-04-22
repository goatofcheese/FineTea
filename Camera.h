/*
	Camera class definition
*/

#include "Vector.h"

class Camera {
public:
	Camera() : pinhole (0, 0, 0.5), focalDistance (-0.5) {}
	Camera(const Vector3d& pin, const Vector3d& face, const Vector3d& vup, double fd) : pinhole(pin), dir(face), up(vup), focalDistance(fd){}
	Camera(const Camera& c) : pinhole (c.pinhole), focalDistance(c.focalDistance){}
	
	friend std::ostream& operator <<(std::ostream& out, const Camera&);

	Vector3d getPinhole ();
	Vector3d getDir();
	Vector3d getUp();
	double getFocalDistance ();
	void setPinhole(const Vector3d p){ pinhole = p; }
	void setDir(const Vector3d d){ dir = d; }
	void setUp(const Vector3d u){ up = u; }
	void setFocalDistance(const double fd){ focalDistance = fd; }

private:
	Vector3d pinhole;
	Vector3d dir;
	Vector3d up;
	double focalDistance;

};
