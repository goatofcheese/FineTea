/*
James Wells
Stephen Wells
CpSc 405 HW 6
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

void handleReshape2( int w, int h ) {
	int basex = 0;
	int basey = 0;
	double lowerscale = 1.;

	//Base viewport
	glViewport(basex, basey, w, h);

	//if the pixmap for the display isn't init'd carry on
	if ( pixmap ) {
		//if screen too small
		if ((w < pixmap->NCols() ) || (h < pixmap->NRows())) {
			double xscale = (double)w / (double)pixmap->NCols();
			double yscale = (double)h / (double)pixmap->NRows();
			
			//scale by the one which needs to most change
			lowerscale = (xscale < yscale) ? xscale : yscale;
		}

		//center image 
		if( w > (lowerscale * pixmap->NCols()) ) {
			basex = (w - (pixmap->NCols() * lowerscale))/2;
		}
		if( h > (lowerscale * pixmap->NRows()) ) {
			basey = (h - (pixmap->NRows() * lowerscale))/2;
		}
		//set viewport at new coords
		glViewport(basex, basey, (pixmap->NCols() * lowerscale), (pixmap->NRows() * lowerscale));
	}
  
	glPixelZoom(lowerscale, lowerscale);
  
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);
	return;
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

Color nonRecShade(Ray ray, Collision col, vector<Light*> lights){

	double id, is, kd = 1.0, ks = 1.0;
	Vector3d h;
	Color cd, cs, ambient, diffuse, specular, mapcolor;
	Color	ret = Color(0.,0.,0.,1.);

	Material mat = *(col.m);
	ambient = mat.a;
	diffuse = mat.d;
	specular = mat.s;
	int ui, vi, row, column, illum_model;
	double alpha;

	illum_model = mat.illum_model;
	if(mat.amap){
		ui = (int)(mat.amap[0].NCols() * col.uv[0]) % mat.amap[0].NCols();
		vi = (int)(mat.amap[0].NRows() * col.uv[1]) % mat.amap[0].NRows();
		alpha = ((double)mat.amap[0][vi][ui][3] / 255.0);
		mapcolor = Color( (double)mat.amap[0][vi][ui][0] / 255.,
								(double)mat.amap[0][vi][ui][1] / 255.,	
								(double)mat.amap[0][vi][ui][2] / 255.,
								1.0 );
		ambient = ((1 - alpha) * ambient + (alpha * mapcolor));
	}
	if(mat.dmap){
		ui = (int)(mat.dmap[0].NCols() * col.uv[0]) % mat.dmap[0].NRows();
		vi = (int)(mat.dmap[0].NRows() * col.uv[1]) % mat.dmap[0].NRows();
		alpha = ((double)mat.dmap[0][vi][ui][3] / 255.0);
		mapcolor = Color( (double)mat.dmap[0][vi][ui][0] / 255.,
								(double)mat.dmap[0][vi][ui][1] / 255.,	
								(double)mat.dmap[0][vi][ui][2] / 255.,
								1.0 );
		diffuse = ((1 - alpha) * diffuse + (alpha * mapcolor));
	}
	if(mat.smap){
		ui = (int)(mat.smap[0].NCols() * col.uv[0]) % mat.smap[0].NRows();
		vi = (int)(mat.smap[0].NRows() * col.uv[1]) % mat.smap[0].NRows();
		alpha = ((double)mat.smap[0][vi][ui][3] / 255.0);
		mapcolor = Color( (double)mat.smap[0][vi][ui][0] / 255.,
								(double)mat.smap[0][vi][ui][1] / 255.,	
								(double)mat.smap[0][vi][ui][2] / 255.,
								1.0 );
		specular = ((1 - alpha) * diffuse + (alpha * mapcolor));
	}

	for(long l = 0; l < lights.size(); l++){
		Light *light = lights.at(l);
		if(light == NULL)
			continue;
		Color lcolor;
		Vector3d ul;
		lcolor = light->getColor();
		ul = light->getDirection(col.x);



		h = ((col.x- ray.p).normalize() + ul).normalize();
		id = max(-1 * (ul * col.n), 0.);
		is = (h * col.n);
		is = pow(is, mat.exp);
	
		cd = id * (diffuse * lcolor);
		cs = is * (specular * lcolor);
		cd = cd * kd;
		cs = cs * ks;		

		if(illum_model == 0)
			ret = ret + cd;
		else if(illum_model == 1)
			ret = ret + cd + (ambient * 0.2);
		else if(illum_model == 2)
			ret = ret + cd + cs + (ambient * 0.2);	

		for(int i = 0; i < 3; i++){
			if(ret[i] >= 1.0)
				ret[i] = 1.0;
			if(ret[i] < 0.0)
				ret[i] = 0.0;
		}
	}
	return ret;
}

Color directShade(Ray ray, Collision col, Color lcolor, Vector3d ul, vector<Object*> scene){

	double id, is, kd = 1.0, ks = 1.0;
	Vector3d h;
	Color cd, cs, ret, ambient, diffuse, specular, mapcolor;

	Material mat = *(col.m);
	ambient = mat.a;
	diffuse = mat.d;
	specular = mat.s;
	int ui, vi, row, column, illum_model;
	double alpha;

	illum_model = mat.illum_model;

	if(mat.amap){
		ui = (int)(mat.amap[0].NCols() * col.uv[0]) % mat.amap[0].NCols();
		vi = (int)(mat.amap[0].NRows() * col.uv[1]) % mat.amap[0].NRows();
		alpha = ((double)mat.amap[0][vi][ui][3] / 255.0);
		mapcolor = Color( (double)mat.amap[0][vi][ui][0] / 255.,
								(double)mat.amap[0][vi][ui][1] / 255.,	
								(double)mat.amap[0][vi][ui][2] / 255.,
								1.0 );
		ambient = ((1 - alpha) * ambient + (alpha * mapcolor));
	}
	if(mat.dmap){
		ui = (int)(mat.dmap[0].NCols() * col.uv[0]) % mat.dmap[0].NCols();
		vi = (int)(mat.dmap[0].NRows() * col.uv[1]) % mat.dmap[0].NRows();
		alpha = ((double)mat.dmap[0][vi][ui][3] / 255.0);
		mapcolor = Color( (double)mat.dmap[0][vi][ui][0] / 255.,
								(double)mat.dmap[0][vi][ui][1] / 255.,	
								(double)mat.dmap[0][vi][ui][2] / 255.,
								1.0 );
		diffuse = ((1 - alpha) * diffuse + (alpha * mapcolor));
	}
	if(mat.smap){
		ui = (int)(mat.smap[0].NCols() * col.uv[0]) % mat.smap[0].NRows();
		vi = (int)(mat.smap[0].NRows() * col.uv[1]) % mat.smap[0].NRows();
		alpha = ((double)mat.smap[0][vi][ui][3] / 255.0);
		mapcolor = Color( (double)mat.smap[0][vi][ui][0] / 255.,
								(double)mat.smap[0][vi][ui][1] / 255.,	
								(double)mat.smap[0][vi][ui][2] / 255.,
								1.0 );
		specular = ((1 - alpha) * diffuse + (alpha * mapcolor));
	}

	h = ((col.x- ray.p).normalize() + ul).normalize();
	id = max(-1 * (ul * col.n), 0.);
	is = (h * col.n);
	is = pow(is, mat.exp);

	cd = id * (diffuse * lcolor);
	cs = is * (specular * lcolor);
	
	cd = cd * kd;
	cs = cs * ks;		

	if(illum_model == 0)
		ret = cd;
	else if(illum_model == 1)
		ret = cd + (ambient * 0.2);
	else if(illum_model == 2)
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

//	if((col.n * (r.p - col.x) > 0)){
		//shoot reflection vector 
		Ray refRay(col.x, refVec);
		Collision refcol = shoot(refRay, scene);
	

		if(refcol.t != INFINITY){
			recCol = recShade(refRay, refcol, level + 1, scene, lights);
			color = color + recCol;
			recCol = directShade(r, col, color, refcol.x - col.x, scene);
			color = color + recCol;
		}

//	}

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

void raytrace(char* argv[], std::string svn, int Nrays, bool wFileExists, PolySurf *p, ImageFile *imageFile, Camera* c, double worldwidth, double worldheight, bool ortho, float *transform, std::vector<Vector3d> lightpos, double glWidth){

	sn = svn;
	wfe = wFileExists;
	img = imageFile;

	Matrix4x4 matTrans;
	matTrans = Matrix4x4(transform[0], transform[4], transform[8], transform[12],
				 transform[1], transform[5], transform[9], transform[13],
				 transform[2], transform[6], transform[10], transform[14],
				 transform[3], transform[7], transform[11], transform[15]);


	orthographic = ortho;

	//Adjust pixel height accordinglly
	Width = glWidth;
	Height = Width*(worldheight/worldwidth);
	
	//Camera *cam = new Camera(*viewpoint, *face, *up, d1);
	Camera *cam = c;
	std::cout<< "Camera: " << *cam<< std::endl;

	pixmap = new Pixmap(Height, Width);
	if(wFileExists)
		img->setpixmap(pixmap);

	// initialize default colors
	black = Color(0.0, 0.0, 0.0, 1.);

	//start up display
	Magick::InitializeMagick(*argv);
	startgraphics(Width, Height);

	//construct scene
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
	wh = worldheight;
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
	//Vector3d lightpos(100, 100, 100);
	Color lightcol(0.9, 0.9, 0.9, 1.);
/*	std::vector<Light*> lights(5);
	lights.at(0) = new PointLight(lightcol, lightpos);
	lights.at(1) = new PointLight(lightcol, Vector3d(-20.0, 0.9, -5));
	lights.at(2) = new PointLight(Color(0.4, 0.4, 0.2, 1), Vector3d(0, 20, 0));
	lights.at(3) = new PointLight(lightcol, Vector3d(-20, -20, 20));
	lights.at(4) = new PointLight(lightcol, Vector3d(20, -22, 18));
*/
	std::vector<Light*> lights;
	lights.reserve(lightpos.size());
	for(std::vector<Vector3d>::iterator iter = lightpos.begin(); iter != lightpos.end(); ++iter){
	    std::cout << "and over here lightpos: " << (*iter) << std::endl;
	    lights.push_back(new PointLight(lightcol,(*iter)));
//	    lights.push_back(new PointLight(lightcol,Vector3d(100,100,100)));
	}

	//shoot a ray through each pixel	
	center = (pin + (cam->getDir() * cam->getFocalDistance()));

	Color shades[Nrays];

	pz = -1. * cam->getFocalDistance();
	for(i = 0; i < rows; i++){
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
				Vector3d pcenter = center + ((px + rx) * ux) + ((py + ry) * uy);
				//if orthographic, shoot ray perpendicular to screen
				if(orthographic){
					ur = cam->getDir().normalize();
					pin = pcenter;
				}
				else{
					ur = (pcenter - pin).normalize();
				}
				//Vector3d nohit = Vector3d(-0.42363566508318828, -0.31759419697725882, -0.8483317448474359);
				Ray r(pin, ur);
				closest = shoot(r, scene);
				if(closest.objectid != -1){
					count++;
					shades[n] = recShade(r, closest, 0, scene, lights);
					//shades[n] = nonRecShade(r, closest, lights);

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
	glutReshapeFunc(handleReshape2);
	glutKeyboardFunc(handleKey2);
	glutMainLoop();

 
  return;
}
