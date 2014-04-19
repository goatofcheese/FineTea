/*
James Wells
CpSc 405 HW 5
Main Driver

Usage:
	objtrace objfile.obj nrays imagefile.ext
*/

#include "Matrix.h"
#include "Camera.h"
#include "ViewScreen.h"
#include "Sphere.h"
#include "PointLight.h"
#include "ParallelLight.h"
#include "ImageFile.h"
#include "Color.h"
#include "NewMaterial.h"
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

//globals
int Width = 500, Height = 400;
ImageFile *image = NULL;
Pixmap *pixmap = NULL;
std::string saveName;
bool wFileExists = false;
bool orthographic = false;
int Nrays = 1;
OBJFile objfile;
Color black;
int MAX_RECURSION_LEVEL = 8;
double SMALL_NUMBER = 0.00000001;

void drawScreen(){
	
	//clear to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glRasterPos2i(0, 0);
	
	
	pixmap->Draw();


	glutSwapBuffers();
}

void handleKey(unsigned char key, int x, int y){
	string prompt;
	switch(key){
		case 'Q':
		case 'q':
			exit(0);
			break;
		case 'W':
		case 'w':
			if(wFileExists)
				image->write(saveName.c_str());
			break;
		default:
			return;
	}
}

void startgraphics(int w, int h){
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(w, h);
	glutCreateWindow("ImgView");
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
}



//returns index to closest hit object
RayHit shoot(Vector3d ur, Vector3d base, vector<Object*> scene){

	RayHit rh, closest;
	Vector3d hit;
	closest.oindex = -1;
	closest.distance = INFINITY;
	closest.hit = false;
	for(long l = 0; l < scene.size(); l++){
		rh = scene.at(l)->hits(ur, base, hit);			
		if(rh.hit)
		{	
			if(closest.oindex == -1 || ((rh.point - base).norm() < (closest.point - base).norm())){
				closest = rh;
				closest.oindex = l;
			}
		}
	}

	return closest;	
} 

Color directShade(Vector3d eye, RayHit rh, Color lcolor, Vector3d ul, vector<Object*> scene){

	int co = 100;	//specular coefficient
	double id, is, kd = 1.0, ks = 1.0;
	Vector3d h;
	Color cd, cs, ret, ambient, diffuse, specular;

	NewMaterial mat = scene.at(rh.oindex)->getMaterial(rh);
	ambient = mat.a;
	diffuse = mat.d;
	specular = mat.s;
	int ui, vi;

	if(mat.amap){
		ui = mat.amap[0].NCols() * rh.uv[0];
		vi = mat.amap[0].NRows() * rh.uv[1];
		if(ui >= mat.amap[0].NCols()) ui--;
		if(vi >= mat.amap[0].NCols()) vi--;
		ambient[0] = (double)mat.amap[0][vi][ui][0] / 255;
		ambient[1] = (double)mat.amap[0][vi][ui][1] / 255;
		ambient[2] = (double)mat.amap[0][vi][ui][2] / 255;
	}
	if(mat.dmap){
		ui = mat.dmap[0].NCols() * rh.uv[0];
		vi = mat.dmap[0].NRows() * rh.uv[1];
		if(ui >= mat.dmap[0].NCols()) ui--;
		if(vi >= mat.dmap[0].NCols()) vi--;
		diffuse[0] = (double)mat.dmap[0][vi][ui][0] / 255;
		diffuse[1] = (double)mat.dmap[0][vi][ui][1] / 255;
		diffuse[2] = (double)mat.dmap[0][vi][ui][2] / 255;
	}

	h = ((rh.point- eye).normalize() + ul).normalize();
	id = max(-1 * (ul * rh.normal), 0.);
	is = (h * rh.normal);
	is = pow(is, mat.exp);

	cd = id * (diffuse * lcolor);
	cs = is * (specular * lcolor);
	
	cd = cd * kd;
	cs = cs * ks;		

	ret = cd + cs + (ambient * 0.2);

	for(int i = 0; i < 3; i++){
		if(ret[i] >= 1.0)
			ret[i] = 1.0;
		if(ret[i] < 0.0)
			ret[i] = 0.0;
	}

	return ret;
}

Color recShade(Vector3d eye, RayHit rh, int level, vector<Object*> scene, vector<Light*> lights){
	Vector3d ul;
	Color color, r;
	RayHit visCheck;
	int occluded = 0;
	double id;
	color = black;

	if(level > MAX_RECURSION_LEVEL)
		return color;

	for(long l = 0; l < lights.size(); l++){
		Light *light = lights.at(l);
		if(light == NULL)
			continue;

		ul = light->getDirection(rh.point);	

		//check for visibility
		visCheck = shoot( (light->getPos() - rh.point).normalize(), rh.point , scene);

		double euclidDist = (light->getPos() - rh.point).norm();
		if(visCheck.distance > abs(euclidDist) || visCheck.distance < SMALL_NUMBER){
			color = color + directShade(eye, rh, light->getColor(), ul,  scene);
		}

	}


	//calc reflection vector
	Vector3d refVec;
	refVec = (rh.point - eye) - (2 * (rh.normal * (rh.point - eye)) * rh.normal);
	refVec = refVec.normalize();

	//shoot reflection vector
	RayHit refhit = shoot(refVec, rh.point, scene);


	if(refhit.distance != INFINITY){
		r = recShade(rh.point, refhit, level + 1, scene, lights);
		color = color + r;
		r = directShade(eye, rh, color, refhit.point - rh.point, scene);
		color = color + r;
	}

	for(int i = 0; i < 3; i++){
		if(color[i] >= 1.0)
			color[i] = 1.0;
		if(color[i] < 0.0)
			color[i] = 0.0;
	}

	return color;
}

vector<Object*> buildScene(){

/*	NewMaterial mat1 = NewMaterial(Color(.2, 0., 0., 1.), Color(0.7, 0.0, 0.0),
								Color(0.5, 0.5, 0.5), 100.);
	NewMaterial mat2 = NewMaterial(Color(0., .2, 0., 1.), Color(0.0, 0.7, 0.0),
								Color(0.2, 0.2, 0.2), 100.);
*/
	std::vector<Object*> scene(1);
/*   Vector3d *center;
   center = new Vector3d(-2., 0.0, 2.);
   scene.at(0) = new Sphere(*center, 0.5, mat1); //red

	center = new Vector3d(0.0, 4., 2.);
   scene.at(1) = new Sphere(*center, 1.0, mat2); //green
*/
	objfile.read();
	scene.at(0) = objfile.getscene();
	return scene;
}


int main(int argc, char* argv[]){

	int suffix;
	//parse command line arguments
	switch(argc){
		case 4:
			image = new ImageFile();
			saveName = argv[3];
			wFileExists = true;
		case 3:
			Nrays = atoi(argv[2]);
		case 2:
			suffix = strlen(argv[1]) - 4;
			if(strcmp(&(argv[1][suffix]), ".obj") != 0){
				std::cerr<< "Invalid file suffix: " << suffix << std::endl;
				exit(1);
			}
			objfile.setfilename(argv[1]);
			break;
		case 1:
		default:
			std::cerr<< "invalid number of arguments "<< std::endl;
			exit(-1);
			break;
	}

	/* read in camera attributes */
	double d1, d2, d3;
	char view;
	Vector3d *viewpoint, *face, *up;
	std::ifstream attributes("camera.txt");
	if(!attributes.is_open()){
		std::cerr<< "Could not open attributes file" <<std::endl;
		exit(-1);
	}
	attributes >> d1 >> d2 >> d3;	
	viewpoint = new Vector3d(d1,d2,d3);
	attributes >> d1 >> d2 >> d3;	
	face = new Vector3d(d1,d2,d3);
	*face = face->normalize();
	attributes >> d1 >> d2 >> d3;	
	up = new Vector3d(d1,d2,d3);
	attributes >> d1 >> d2 >> d3;	
	attributes >> view;
	if(view == 'l')
		orthographic = true;
	else if(view == 'v')
		orthographic = false;

	//Adjust pixel height accordinglly
	Height = Width/d2;
	
	Camera *cam = new Camera(*viewpoint, *face, *up, d1);
	ViewScreen *vs = new ViewScreen(Width, Height, d3, d3/d2);
	pixmap = new Pixmap(Height, Width);
	if(wFileExists)
		image->setpixmap(pixmap);

	// initialize default colors
	black = Color(0.0, 0.0, 0.0, 1.);

	//start up display
	Magick::InitializeMagick(*argv);
	glutInit(&argc, argv);
	startgraphics(Width, Height);

	//construct scene
	PolySurf *object;
	std::vector<Object*> scene = buildScene();
	
	//raycast
	int i, rows, j, cols, count=0;
	double ph, pw, rx, ry, rz, pz, py, px;
	Vector3d center, pin, ux, uy, uz, ur, hit, base;
	RayHit closest;
	unsigned char r, g, b, a;
	RGBApixel color;

	pin = cam->getPinhole();
	rows = vs->getPheight();
	cols = vs->getPwidth();
	ph = vs->getWheight() / double(rows);
	pw = vs->getWwidth() / double(cols);

	//Orient camera	
	//uz = -d from c
	//ux = dir x up normalized
	uz = -1 * cam->getDir();
	ux = (cam->getDir() % cam->getUp()).normalize();
	uy = uz % ux;

	//Arrange Lights
	Vector3d lightpos(3, 0, 0);
	Color lightcol(0.8, 0.8, 0.2, 1.);
	std::vector<Light*> lights(4);
	lights.at(0) = new PointLight(lightcol, lightpos);
	lights.at(1) = new PointLight(lightcol, Vector3d(-3.0, 0.9, -5));
	lights.at(2) = new PointLight(Color(0.4, 0.4, 0.2, 1), Vector3d(0, 3, 0));
	lights.at(3) = new PointLight(lightcol, Vector3d(-3, 0, 3));

	//shoot a ray through each pixel	
	center = (pin + (cam->getDir() * cam->getFocalDistance()));

	Color shades[Nrays];

	pz = -1. * cam->getFocalDistance();
	for(i =0; i < rows; i++){
		py = (vs->getWheight() / -2.) + (ph * (i + 0.5));
		for(j = 0; j < cols; j++){
			px = (vs->getWwidth() / -2.) + (pw * (j + 0.5));
			int n;
			//supersampling loop
			Color vcol(0.,0.,0.,1.);
			for(n = 0; n < Nrays; n++){
				double rx = 0, ry = 0;
				if(Nrays > 1){
					rx = (pw * -0.5) + (((double)rand() / RAND_MAX) * pw);
					ry = (ph * -0.5) + (((double)rand() / RAND_MAX) * ph);
				}
				Vector3d p = center + ((px + rx) * ux) + ((py + ry) * uy);
				//if orthographic, shoot ray perpendicular to screen
				if(orthographic){
					ur = cam->getDir();
				}
				else{
					ur = (p - pin).normalize();
				}
				closest = shoot(ur, pin, scene);
				if(closest.oindex != -1){
					count++;
					shades[n] = recShade(pin, closest, 0, scene, lights);
				}
				else{
					shades[n][0] = 0;
					shades[n][1] = 0;
					shades[n][2] = 0;
				}
			}
			for(n = 0; n < Nrays; n++){
				vcol[0] = vcol[0] + shades[n][0];
				vcol[1] = vcol[1] + shades[n][1];
				vcol[2] = vcol[2] + shades[n][2];
			}
			(*pixmap)[i][j].r = (vcol[0] / Nrays) * 255;
			(*pixmap)[i][j].g = (vcol[1] / Nrays) * 255;
			(*pixmap)[i][j].b = (vcol[2] / Nrays) * 255;
			(*pixmap)[i][j].a = 255;
		}
	}

	glutDisplayFunc(drawScreen);
	glutKeyboardFunc(handleKey);
	glutMainLoop();

 
  return 0; 
}
