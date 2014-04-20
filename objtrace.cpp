/*
James Wells
CpSc 405 HW 5
Main Driver

Usage:
	objtrace objfile.obj nrays imagefile.ext
*/

#include "objtrace.h"

//globals
int Width = 800, Height = 600;
ImageFile *img;
Pixmap *pixmap = NULL;
std::string sn;
bool wfe;
bool orthographic = false;
Color black;
int MAX_RECURSION_LEVEL = 8;

void drawScreen2(){
	
	//clear to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glRasterPos2i(0, 0);
	
	
	pixmap->Draw();


	glutSwapBuffers();
}

void handleKey2(unsigned char key, int x, int y){
	string prompt;
	switch(key){
		case 'Q':
		case 'q':
			exit(0);
			break;
		case 'W':
		case 'w':
			if(wfe)
				img->write(sn.c_str());
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
Collision shoot(Ray r, vector<Object*> scene){

	//RayHit rh, closest;
	Collision col, closest;
	closest.objectid = -1;
	closest.t = INFINITY;
	for(long l = 0; l < scene.size(); l++){
		
		col = scene.at(l)->RayCollide(r);			
		if(col.t != INFINITY)
		{
			if(closest.objectid == -1 || ((col.x - r.p).norm() < (closest.x - r.p).norm())){
				closest = col;
				closest.objectid = l;
			}
		}
	}

	return closest;	
} 

Color directShade(Ray ray, Collision col, Color lcolor, Vector3d ul, vector<Object*> scene){

	double id, is, kd = 1.0, ks = 1.0;
	Vector3d h;
	Color cd, cs, ret, ambient, diffuse, specular;

	Material mat = *(col.m);
	ambient = mat.a;
	diffuse = mat.d;
	specular = mat.s;
	int ui, vi;

	if(mat.amap){
		ui = (int)(mat.amap[0].NCols() * col.uv[0]) % mat.amap[0].NCols();
		vi = (int)(mat.amap[0].NRows() * col.uv[1]) % mat.amap[0].NRows();
		ambient[0] = (double)mat.amap[0][vi][ui][0] / 255;
		ambient[1] = (double)mat.amap[0][vi][ui][1] / 255;
		ambient[2] = (double)mat.amap[0][vi][ui][2] / 255;
	}
	if(mat.dmap){
		ui = (int)(mat.dmap[0].NCols() * col.uv[0]) % mat.dmap[0].NRows();
		vi = (int)(mat.dmap[0].NRows() * col.uv[1]) % mat.dmap[0].NRows();
		diffuse[0] = (double)mat.dmap[0][vi][ui][0] / 255;
		diffuse[1] = (double)mat.dmap[0][vi][ui][1] / 255;
		diffuse[2] = (double)mat.dmap[0][vi][ui][2] / 255;
	}

	h = ((col.x- ray.p).normalize() + ul).normalize();
	id = max(-1 * (ul * col.n), 0.);
	is = (h * col.n);
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

Color recShade(Ray r, Collision col, int level, vector<Object*> scene, vector<Light*> lights){
	Vector3d ul;
	Color color, recCol;
	Collision visCheck;
	int occluded = 0;
	double id;
	color = black;

	if(level > MAX_RECURSION_LEVEL)
		return color;

	for(long l = 0; l < lights.size(); l++){
		Light *light = lights.at(l);
		if(light == NULL)
			continue;

		ul = light->getDirection(col.x);	

		//check for visibility
		Ray sray(col.x, (light->getPos() - col.x).normalize());
		visCheck = shoot(sray, scene);

		double euclidDist = (light->getPos() - col.x).norm();
		if(visCheck.t > abs(euclidDist) || visCheck.t < SMALLNUMBER){
			color = color + directShade(r, col, light->getColor(), ul,  scene);
		}

	}


	//calc reflection vector
	Vector3d refVec;
	refVec = (col.x - r.p) - (2 * (col.n * (col.x - r.p)) * col.n);
	refVec = refVec.normalize();

	//shoot reflection vector
	Ray refRay(col.x, refVec);
	Collision refcol = shoot(refRay, scene);


	if(refcol.t != INFINITY){
		recCol = recShade(refRay, refcol, level + 1, scene, lights);
		color = color + recCol;
		recCol = directShade(r, col, color, refcol.x - col.x, scene);
		color = color + recCol;
	}

	for(int i = 0; i < 3; i++){
		if(color[i] >= 1.0)
			color[i] = 1.0;
		if(color[i] < 0.0)
			color[i] = 0.0;
	}

	return color;
}

vector<Object*> buildScene(PolySurf *p){

	std::vector<Object*> scene(1);
	scene.at(0) = (Object*) p;
	return scene;
}


void raytrace(char* argv[], std::string svn, int Nrays, bool wFileExists, PolySurf *p, ImageFile *imageFile, Camera* c, double worldwidth, bool ortho){

	sn = svn;
	wfe = wFileExists;

	img = imageFile;
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


	orthographic = ortho;


	//Adjust pixel height accordinglly
	Height = Width/d2;
	
	Camera *cam = new Camera(*viewpoint, *face, *up, d1);
	std::cout<< *cam << std::endl;
	cam = c;
	std::cout<< *cam << std::endl;
	//ViewScreen *vs = new ViewScreen(Width, Height, d3, d3/d2);
	pixmap = new Pixmap(Height, Width);
	if(wFileExists)
		img->setpixmap(pixmap);

	// initialize default colors
	black = Color(0.0, 0.0, 0.0, 1.);

	//start up display
	Magick::InitializeMagick(*argv);
	startgraphics(Width, Height);

	//construct scene
	PolySurf *object;
	std::vector<Object*> scene = buildScene(p);
	
	//raycast
	int i, rows, j, cols, count=0;
	double ph, pw, rx, ry, rz, pz, py, px, ww, wh;
	Vector3d center, pin, ux, uy, uz, ur, hit, base;
	Collision closest;
	unsigned char r, g, b, a;
	RGBApixel color;

	pin = cam->getPinhole();
	ww = worldwidth;
	wh = ww/d2;
	rows = Height;
	cols = Width;
	ph = wh / double(rows);
	pw = ww / double(cols);

	//Orient camera	
	//uz = -d from c
	//ux = dir x up normalized
	uz = -1 * cam->getDir();
	ux = (cam->getDir() % cam->getUp()).normalize();
	uy = uz % ux;

	//Arrange Lights
	//cube Vector3d lightpos(3, 0, 0);
	Vector3d lightpos(20, 0, 0);
	Color lightcol(0.8, 0.8, 0.2, 1.);
	std::vector<Light*> lights(5);
	lights.at(0) = new PointLight(lightcol, lightpos);
	lights.at(1) = new PointLight(lightcol, Vector3d(-20.0, 0.9, -5));
	lights.at(2) = new PointLight(Color(0.4, 0.4, 0.2, 1), Vector3d(0, 20, 0));
	lights.at(3) = new PointLight(lightcol, Vector3d(-20, -20, 20));
	lights.at(4) = new PointLight(lightcol, Vector3d(20, -22, 18));

	//shoot a ray through each pixel	
	center = (pin + (cam->getDir() * cam->getFocalDistance()));

	Color shades[Nrays];

	pz = -1. * cam->getFocalDistance();
	for(i =0; i < rows; i++){
		py = (wh / -2.) + (ph * (i + 0.5));
		for(j = 0; j < cols; j++){
			px = (ww / -2.) + (pw * (j + 0.5));
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
					ur = cam->getDir().normalize();
					pin = p;
				}
				else{
					ur = (p - pin).normalize();
				}
				Ray r(pin, ur);
				closest = shoot(r, scene);
				if(closest.objectid != -1){
					count++;
					shades[n] = recShade(r, closest, 0, scene, lights);
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
	
	std::cout<< "count: " << count << std::endl;
	glutDisplayFunc(drawScreen2);
	glutKeyboardFunc(handleKey2);
	glutMainLoop();

 
  return;
}
