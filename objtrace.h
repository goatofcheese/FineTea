//objtrace.h
#include "Matrix.h"
#include "Camera.h"
#include "ViewScreen.h"
#include "PointLight.h"
#include "ParallelLight.h"
#include "ImageFile.h"
#include "Color.h"
#include "Material.h"
#include "PolySurf.h"
#include "OBJFile.h"
#include <vector>
#include <GL/glut.h>
#include <iostream>
#include <ostream>
#include <fstream>
#include <string>
#include <string.h>
#include <math.h>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif


void raytrace(char**, std::string, int, bool, PolySurf *, ImageFile *, Camera *, double, double, bool, GLfloat *);
