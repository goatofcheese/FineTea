/*
James Wells
CpSc 405 HW 4
Main Driver

Usage:
	raycast [l|v] [pixwidth] [filename.ext]
*/

#include "Matrix.h"
#include "Camera.h"
#include "ViewScreen.h"
#include "Sphere.h"
#include "PointLight.h"
#include "ParallelLight.h"
#include <vector>
#include <Magick++.h>
#include <GL/glut.h>
#include <iostream>
#include <ostream>
#include <fstream>
#include <string>
#include <string.h>
#include <math.h>

using namespace Magick;


//globals
Image _image;
int Width = 500, Height = 400;
RGBApixel *magicmap;
bool orthographic = false;
std::string saveName;
Vector3d black;
RGBApixel pblack;
int MAX_RECURSION_LEVEL = 8;
double SMALL_NUMBER = 0.001;


//since the origins of the image differ between opengl and imagemagick, we've gotta move stuff around
void rearrange(){
	int i, j;
	const unsigned char *buffer;
	buffer = (unsigned char *) malloc(Width * sizeof(RGBApixel));
	for(i = 0; i < Height / 2; i++){
		memcpy((void *)buffer, (void *)&magicmap[Width * i], Width * sizeof(RGBApixel));
		memcpy((void *)&magicmap[Width * i], 
			(void *)&magicmap[Width * (Height - i - 1)], Width * sizeof(RGBApixel));
		memcpy((void *)&magicmap[Width * (Height -i - 1)], (void *)buffer, Width * sizeof(RGBApixel));
	}
	
}

void drawScreen(){
	
	//clear to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glRasterPos2i(0, 0);
	
	glDrawPixels(Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, magicmap);
/*fixings
	pixmap.Draw();
*/

	glutSwapBuffers();
}

void writeImage(const char* name){
	rearrange();
	_image.read(Width, Height,"RGBA", CharPixel, (void *) magicmap);
	_image.write(name);
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
			writeImage(saveName.c_str());
			return;
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
RayHit shoot(Vector3d ur, Vector3d base, vector<Sphere> scene){

	RayHit rh, closest;
	Vector3d hit, lasthit = NULL;
	closest.oindex = -1;
	closest.distance = INFINITY;
	closest.hit = false;
	for(long l = 0; l < scene.size(); l++){
		rh = scene.at(l).hits(ur, base, hit);			
		if(rh.hit)
		{	
			if(lasthit == NULL || ((rh.point - base).norm() < (lasthit - base).norm())){
				closest = rh;
				closest.oindex = l;
				lasthit = closest.point;
			}
		}
	}
	
	return closest;	
} 

Vector3d directShade(Vector3d eye, RayHit rh, Vector3d lcolor, Vector3d ul, vector<Sphere> scene){

	int co = 100;
	double id, is, kd = 1.0, ks = 1.0;
	Vector3d ret(0.,0.,0.), h, cd, cs, ambient(.2,.2,.2), fcolor;
	Material mat = scene.at(rh.oindex).getMaterial();

	RGBApixel cm = mat.getColor();
	fcolor[0] = (double)cm.r / 255.0;
	fcolor[1] = (double)cm.g / 255.0;
	fcolor[2] = (double)cm.b / 255.0;

	h = ((rh.point- eye).normalize() + ul).normalize();
	id = -1 * (ul * rh.normal);
	is = (h * rh.normal);
	is = pow(is, co);
	cd = id * mat.getDiffuse() * (lcolor ^ fcolor);
	cs = is * mat.getSpecular() * (lcolor ^ fcolor); 
	
	cd = cd * kd;
	cs = cs * ks;		
	
	ret = cd + cs + (ambient ^ fcolor);

	for(int i = 0; i < 3; i++){
		if(ret[i] >= 1.0)
			ret[i] = 1.0;
		if(ret[i] < 0.0)
			ret[i] = 0.0;
	}

	return ret;
}

Vector3d recShade(Vector3d eye, RayHit rh, int level, vector<Sphere> scene, vector<Light*> lights){
	Vector3d color, ul, r;
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
	}


	for(int i = 0; i < 3; i++){
		if(color[i] >= 1.0)
			color[i] = 1.0;
		if(color[i] < 0.0)
			color[i] = 0.0;
	}

	return color;
}

vector<Sphere> buildScene(){
   RGBApixel dakulur;
   dakulur.r = 70;
   dakulur.g = 0;
   dakulur.b = 0;
   dakulur.a = 255;
	Material *mat;
	mat = new Material(dakulur, 1.0, 1.0);

   std::vector<Sphere> scene(5);
   Vector3d *center;
   center = new Vector3d(-0.3, 0.1, -0.5);
   scene.at(0) = Sphere(*center, 0.05, *mat); //red


   dakulur.g = 255;
	dakulur.r = 0;
	mat = new Material(dakulur, 1.0, 1.0);
	center = new Vector3d(0.0, -0.2, -0.8);
   scene.at(1) = Sphere(*center, 0.15, *mat); //green


   dakulur.b = 255;
	dakulur.g = 0;
	mat = new Material(dakulur, 1.0, 1.0);
   center = new Vector3d(0.3, 0.3, -1.1);
   scene.at(2) = Sphere(*center, 0.3, *mat); //blue


   dakulur.r = 255;
	dakulur.g = 155;
	dakulur.b = 0;
	mat = new Material(dakulur, 1.0, 1.0);
   center = new Vector3d(0.1, 0.2, -0.3);
   scene.at(3) = Sphere(*center, 0.075, *mat); //orange


   dakulur.r = 70;
	dakulur.g = 0;
	dakulur.b = 120;
	mat = new Material(dakulur, 1.0, 1.0);
   center = new Vector3d(-0.2, -0.25, -0.4);
   scene.at(4) = Sphere(*center, 0.225, *mat); //purple

	return scene;
}


int main(int argc, char* argv[]){

	//parse command line arguments
	switch(argc){
		case 4:
			/*fixings
				ImageFile image(argv[3]);
			*/
			saveName = argv[3];
		case 3:
			Width = atoi(argv[2]);
			Height = Width * 4. / 5.;
			if(Width <=0){
				std::cerr<< "invalid pixel width " << argv[2] << std::endl;
				exit(-1);
			}
		case 2:
			if(*argv[1] == 'l')
				orthographic = true;
			else if(*argv[1] == 'v')
				orthographic = false;
			else{
				std::cerr<< "invalid cl argument " << *argv[1] << std::endl;
				exit(-1);
			}
			break;
		case 1:
			break;
		default:
			std::cerr<< "invalid number of arguments "<< std::endl;
			exit(-1);
			break;
	}

	/* read in camera attributes */
	double d1, d2, d3;
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
	attributes >> d1 >> d2 >> d3;	
	up = new Vector3d(d1,d2,d3);
	attributes >> d1 >> d2 >> d3;	

	//Adjust pixel height accordinglly
	Height = Width/d2;
	
	Camera *cam = new Camera(*viewpoint, *face, *up, d1);
	ViewScreen *vs = new ViewScreen(Width, Height, d3, d3/d2);

	// initialize default colors
	black = Vector3d(0.0, 0.0, 0.0);
	pblack.r = 0;
	pblack.g = 0;
	pblack.b = 0;
	pblack.a = 255;

	//start up display
	InitializeMagick(*argv);
	glutInit(&argc, argv);
	startgraphics(Width, Height);

	//construct scene
	std::vector<Sphere> scene = buildScene();
	
	//raycast
	int i, rows, j, cols, count=0;
	double ph, pw, rx, ry, rz;
	Vector3d center, pin, ux, uy, uz, ur, hit;
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
	Vector3d lightpos(2, 0.5, 0);
	Vector3d lightcol(0.8, 0.8, 0.2);
	std::vector<Light*> lights(3);
	lights.at(0) = new PointLight(1.0, lightcol, lightpos);
	lights.at(1) = new PointLight(1.0, lightcol, Vector3d(1, 0.5, -5));
	lights.at(2) = new PointLight(1.0, Vector3d(0.4, 0.4, 0.2), Vector3d(-1, 1, 5));

	//shoot a ray through each pixel	
	center = (pin + (cam->getDir() * cam->getFocalDistance()));
	for(i =0; i < rows; i++){
		ry = (vs->getWheight() / -2.) + (ph * (i + 0.5));
		for(j = 0; j < cols; j++){
			rx = (vs->getWwidth() / -2.) + (pw * (j + 0.5));
			ur = center + (rx * ux) + (ry * uy);
			//if orthographic, shoot ray perpendicular to screen
			if(orthographic)
				pin = ur + uz;	
			ur = (ur - pin);
			ur = ur.normalize();

			closest = shoot(ur, pin, scene);
			if(closest.oindex != -1){
				Vector3d vcol = recShade(pin, closest, 0, scene, lights);
				color.r = vcol[0] * 255;
				color.g = vcol[1] * 255;
				color.b = vcol[2] * 255;
				color.a = 255;
				vs->setPixelAt(i,j, color);
				/* fixings
					pixmap[i][j].r = color.r;
				*/
			}
			else{
				vs->setPixelAt(i, j,  pblack);
			}
		}
	}

	magicmap = vs->pixmap[0];

	glutDisplayFunc(drawScreen);
	glutKeyboardFunc(handleKey);
	glutMainLoop();

 
  return 0; 
}
