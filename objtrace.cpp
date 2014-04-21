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
	float alpha;

	illum_model = mat.illum_model;

	if(mat.amap){
		ui = (int)(mat.amap[0].NCols() * col.uv[0]) % mat.amap[0].NCols();
		vi = (int)(mat.amap[0].NRows() * col.uv[1]) % mat.amap[0].NRows();
		alpha = ((float)mat.amap[0][vi][ui][3] / 255.0);
		mapcolor = Color( (double)mat.amap[0][vi][ui][0] / 255.,
								(double)mat.amap[0][vi][ui][1] / 255.,	
								(double)mat.amap[0][vi][ui][2] / 255.,
								1.0 );
		ambient = ((1 - alpha) * ambient + (alpha * mapcolor));
	}
	if(mat.dmap){
		ui = (int)(mat.dmap[0].NCols() * col.uv[0]) % mat.dmap[0].NRows();
		vi = (int)(mat.dmap[0].NRows() * col.uv[1]) % mat.dmap[0].NRows();
		alpha = ((float)mat.dmap[0][vi][ui][3] / 255.0);
		mapcolor = Color( (double)mat.dmap[0][vi][ui][0] / 255.,
								(double)mat.dmap[0][vi][ui][1] / 255.,	
								(double)mat.dmap[0][vi][ui][2] / 255.,
								1.0 );
		diffuse = ((1 - alpha) * diffuse + (alpha * mapcolor));
	}
	if(mat.smap){
		ui = (int)(mat.smap[0].NCols() * col.uv[0]) % mat.smap[0].NRows();
		vi = (int)(mat.smap[0].NRows() * col.uv[1]) % mat.smap[0].NRows();
		alpha = ((float)mat.smap[0][vi][ui][3] / 255.0);
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
	float alpha;

	illum_model = mat.illum_model;

	if(mat.amap){
		ui = (int)(mat.amap[0].NCols() * col.uv[0]) % mat.amap[0].NCols();
		vi = (int)(mat.amap[0].NRows() * col.uv[1]) % mat.amap[0].NRows();
		alpha = ((float)mat.amap[0][vi][ui][3] / 255.0);
		mapcolor = Color( (double)mat.amap[0][vi][ui][0] / 255.,
								(double)mat.amap[0][vi][ui][1] / 255.,	
								(double)mat.amap[0][vi][ui][2] / 255.,
								1.0 );
		ambient = ((1 - alpha) * ambient + (alpha * mapcolor));
	}
	if(mat.dmap){
		ui = (int)(mat.dmap[0].NCols() * col.uv[0]) % mat.dmap[0].NCols();
		vi = (int)(mat.dmap[0].NRows() * col.uv[1]) % mat.dmap[0].NRows();
		alpha = ((float)mat.dmap[0][vi][ui][3] / 255.0);
		mapcolor = Color( (double)mat.dmap[0][vi][ui][0] / 255.,
								(double)mat.dmap[0][vi][ui][1] / 255.,	
								(double)mat.dmap[0][vi][ui][2] / 255.,
								1.0 );
		diffuse = ((1 - alpha) * diffuse + (alpha * mapcolor));
	}
	if(mat.smap){
		ui = (int)(mat.smap[0].NCols() * col.uv[0]) % mat.smap[0].NRows();
		vi = (int)(mat.smap[0].NRows() * col.uv[1]) % mat.smap[0].NRows();
		alpha = ((float)mat.smap[0][vi][ui][3] / 255.0);
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

bool gluInvertMatrix(const float m[16], float invOut[16]){
    float inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}

Vector3d ExtractCameraPos_NoScale(const Matrix4x4 &a_modelView)
{
/*  Matrix3x3 rotMat(a_modelView[0],a_modelView[1],a_modelView[2],
			a_modelView[4], a_modelView[5], a_modelView[6],
			a_modelView[8], a_modelView[9], a_modelView[10]);
*/
  Matrix3x3 rotMat(a_modelView[0][0], a_modelView[0][1], a_modelView[0][2],
		a_modelView[1][0], a_modelView[1][1], a_modelView[1][2],
		a_modelView[2][0], a_modelView[2][1], a_modelView[2][2]);

  Vector3d d(a_modelView[0][3], a_modelView[1][3], a_modelView[2][3]);
 
  Vector3d retVec = -d * rotMat;
  return retVec;
}

void raytrace(char* argv[], std::string svn, int Nrays, bool wFileExists, PolySurf *p, ImageFile *imageFile, Camera* c, double worldwidth, bool ortho, float *transform){

	sn = svn;
	wfe = wFileExists;

	img = imageFile;

	Matrix4x4 matTrans;
	matTrans = Matrix4x4(transform[0], transform[4], transform[8], transform[12],
					 transform[1], transform[5], transform[9], transform[13],
					 transform[2], transform[6], transform[10], transform[14],
					 transform[3], transform[7], transform[11], transform[15]);
/*
	for(int f = 0; f < p->NVertices(); f++){
		Vector3d *vert = &(p->Vertices()[f]);
		Vector4d modvert = Vector4d((*vert)[0], (*vert)[1], (*vert)[2], 1.);
		modvert = (matTrans * modvert);
		(*vert)[0] = modvert[0];
		(*vert)[1] = modvert[1];
		(*vert)[2] = modvert[2];
	}
*/
	float invT[16];
	gluInvertMatrix(transform, invT);

	Matrix4x4 invTransT;
	invTransT = Matrix4x4(invT[0], invT[1], invT[2], invT[3],
				 invT[4], invT[5], invT[6], invT[7],
				 invT[8], invT[9], invT[10], invT[11],
				 invT[12], invT[13], invT[14], invT[15]);
/*
	for(int n = 0; n < p->NNormals(); n++){
		Vector3d *norm = &(p->Normals()[n]);
		Vector4d modnorm = Vector4d((*norm)[0],(*norm)[1],(*norm)[2],1.);
		modnorm = (invTransT * modnorm);
		(*norm)[0] = modnorm[0];
		(*norm)[1] = modnorm[1];
		(*norm)[2] = modnorm[2];
	}
*/
std::cerr << "matTrans:\n" << matTrans << std::endl;
std::cerr << "invTransT:\n" << invTransT << std::endl;
std::cerr << "camera trans dir: " << (matTrans * c->getDir()) << std::endl;
std::cerr << "fuckityfuckfuck: " << ExtractCameraPos_NoScale(matTrans) << std::endl;
std::cerr << std::endl;

p->BuildBIHTree();	

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

	Vector4d burble(matTrans * c->getDir());
std::cerr << "herro?" << std::endl;
	Vector3d gurbleburble(burble[0],burble[1],burble[2]);
	cam = new Camera(ExtractCameraPos_NoScale(matTrans),gurbleburble,Vector3d(0,1,0),0.5);

	cam = new Camera(Vector3d(-9.45, 0.246,17.6),Vector3d(-0.473,0.014,-0.881),Vector3d(0,1,0),0.5);


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
	Vector3d lightpos(1, 1, 30);
	Color lightcol(0.8, 0.8, 0.8, 1.);
/*	std::vector<Light*> lights(5);
	lights.at(0) = new PointLight(lightcol, lightpos);
	lights.at(1) = new PointLight(lightcol, Vector3d(-20.0, 0.9, -5));
	lights.at(2) = new PointLight(Color(0.4, 0.4, 0.2, 1), Vector3d(0, 20, 0));
	lights.at(3) = new PointLight(lightcol, Vector3d(-20, -20, 20));
	lights.at(4) = new PointLight(lightcol, Vector3d(20, -22, 18));
*/
	std::vector<Light*> lights(1);
	lights.at(0) = new PointLight(lightcol, lightpos);

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
					//shades[n] = recShade(r, closest, 0, scene, lights);
					shades[n] = nonRecShade(r, closest, lights);
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
