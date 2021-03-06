/*
   mipmaps.cpp
   
   CP SC 405/605 Computer Graphics    Donald H. House     12/2/08
   
   OpenGL/GLUT Program to View a 3D cube or teapot using a mipmapped textures
   
   Keyboard keypresses have the following effects:
   a		- toggle drawing coordinate axes
   c		- toggle between cube and teapot models
   i       	- reinitialize (reset program to initial default state)
   p		- toggle between orthographic and perspective view
   q or Esc	- quit
   s		- toggle between flat shading and smooth shading
   t		- cycle through texturing modes (nearest, interpolation, mipmapped)
   w		- toggle between wireframe and shaded viewing

   Camera and model controls following the mouse:
   model yaw	- left-button, horizontal motion, rotation of the model around the y axis
   model tilt	- left-button, vertical motion, rotation of the model about the x axis
   camera yaw	- middle-button, horizontal motion, rotation of the camera about the y axis
   camera tilt	- middle-button, vertical motion, rotation of the camera about the x axis
   approach	- right-button, vertical or horizontal motion, translation of camera along z axis
   
   usage: mipmaps
 */

#include <cstdlib>
#include <cstdio>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include "ImageFile.h"
#include "PolySurf.h"
#include "OBJFile.h"
#include <map>
#include <malloc.h>
#include "objtrace.h"

using namespace std;

//*****************************************************************
//  Demo Code
//*****************************************************************

#define FALSE		0
#define TRUE		1

#define HORIZONTAL	0
#define VERTICAL	1

#define ESC		27	// numeric code for keyboard Esc key

#define WIDTH           800	// initial window dimensions
#define HEIGHT          600

#define ORTHO		0	// projection system codes
#define PERSPECTIVE	1

#define NONE		-1	// used to indicate no mouse button pressed

#define ROTFACTOR	0.2     // degrees rotation per pixel of mouse movement
#define XLATEFACTOR	0.1     // units of translation per pixel of mouse movement

#define TEXTUREWIDTH	64	// dimensions of texture map image
#define TEXTUREHEIGHT	64

#define AMBIENT_FRACTION 0.2
#define DIFFUSE_FRACTION 0.8
#define SPECULAR_FRACTION 0.3

// colors used for lights, and materials for coordinate axes
const float DIM_PALEBLUE[] = {0.1, 0.1, 0.3, 1};
const float BRIGHT_PALEBLUE[] = {0.5, 0.5, 1, 1};
const float GRAY[] = {0.3, 0.3, 0.3, 1};
const float WHITE[] = {1, 1, 1, 1};
const float DIM_WHITE[] = {0.8, 0.8, 0.8, 1};
const float DIM_RED[] = {0.3, 0, 0, 1};
const float RED[] = {1, 0, 0, 1};
const float DIM_GREEN[] = {0, 0.3, 0, 1};
const float GREEN[] = {0, 1, 0, 1};
const float DIM_BLUE[] = {0, 0, 0.3, 1};
const float BLUE[] = {0, 0, 1, 1};

enum TEXTUREMODE{NOTEXTURE, NEARTEXTURE, LINEARTEXTURE, MIPMAPTEXTURE};

// white color for wireframe drawing
float white[3] = {1, 1, 1};

//
// Global variables updated and shared by callback routines
//

// Dimensions
static double Width = WIDTH;
static double Height = HEIGHT;
static double DRAWWIDTH = 20;
static double DRAWHEIGHT = 15;
static double NEAR = 1;
static double FAR = 400;
static double DEPTH = -20;
static double WORLDWIDTH;

// Viewing parameters
static int Projection;

// Camera position and orientation
static double Pan;
static double Tilt;
static double Approach;

// model orientation
static double ThetaX;
static double ThetaY;

// global variables to track mouse and shift key
static int MouseX;
static int MouseY;
static int Button = NONE;

// global variables to track wireframe/shaded mode, material color, and textuing mode
static int Wireframe;
static int SmoothShading;
static int TextureMode;
static int ColorMode;

// light
const int NUMLIGHTS = 2;
static std::vector<Vector3d> gl_lightspos;
static std::vector<Vector3d> rt_lightspos;


// Texture map to be used by program
static GLuint TextureID;	    // texture ID from OpenGL
static unsigned char *TextureImage;   // pixmap for texture image
std::map<std::string,GLuint> texIDMap;

//command line attributes
ImageFile *image = NULL;
std::string saveName;
bool wFileExists = false;
int Nrays = 1;
OBJFile objfile;
PolySurf *psurf;
Camera *cam;
char **args;
Vector3d camPos3;
Vector3d camDir3;
Vector3d camUp3;

//ModelView matrix in Matrix4x4 form to change the rays of the raytracer
static Matrix4x4 idklel;
GLfloat mv[16];

//
// Routine to initialize the state of the program to start-up defaults
//
void setInitialState(){
  
	// initial camera viewing and shading model controls
	Projection = ORTHO;
  
	// model initially wireframe with white color, and flat shading
	Wireframe = TRUE;
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	TextureMode = NOTEXTURE;
	ColorMode = GL_MODULATE;
	SmoothShading = FALSE;
    
	// initial camera orientation and position
	Pan = 0;
	Tilt = 0;
	Approach = DEPTH;
  
	// initial model orientation
	ThetaX = 0;
	ThetaY = 0;
}

//
// Set up the projection matrix to be either orthographic or perspective
//
void updateProjection(){
  
	// initialize the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// determine the projection system and drawing coordinates
	if(Projection == ORTHO){
		glOrtho(-DRAWWIDTH/2, DRAWWIDTH/2, -DRAWHEIGHT/2, DRAWHEIGHT/2, NEAR, FAR);
		WORLDWIDTH = DRAWWIDTH;
	}
	else{
		// scale drawing coords so center of cube is same size as in ortho
		// if it is at its nominal location
		double scale = fabs((double)NEAR / (double)DEPTH);
		double xmax = scale * DRAWWIDTH / 2;
		double ymax = scale * DRAWHEIGHT / 2;
		glFrustum(-xmax, xmax, -ymax, ymax, NEAR, FAR);
		WORLDWIDTH = 2. * xmax;
	}
  
	// restore modelview matrix as the one being updated
	glMatrixMode(GL_MODELVIEW);
}

//
// routine to draw the obj file model
//
void drawModel(int wireframe){
	float ambient_color[4];
	float diffuse_color[4];
	float specular_color[4];
	int shininess;

	//Draw the PolySurf
	int i, j;
	bool hasVertexNormals = (psurf->Faces()[0].faceverts[0].n != -1);
	int op = (wireframe? GL_LINE_LOOP: GL_POLYGON);
	for(i = 0; i < psurf->NFaces(); i++){
		//ith face
		Face curFace = psurf->Faces()[i];
		// set up material color
		Material *curMat;
		if(curFace.material != -1)
			curMat = &psurf->Materials()[curFace.material];
		else{
			Color w(1.,1.,1.,1.);
			curMat = new Material(w, w, w, 100);
		}
		for(int k = 0; k < 3; k++){
			ambient_color[k] = AMBIENT_FRACTION * curMat->a[k];
			diffuse_color[k] = DIFFUSE_FRACTION * curMat->d[k];
			specular_color[k] = SPECULAR_FRACTION * curMat->s[k];
			shininess = (int)curMat->exp;
		}
		glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_color);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_color);
		glMaterialfv(GL_FRONT, GL_SPECULAR, specular_color);
		glMaterialf(GL_FRONT, GL_SHININESS, shininess);

		if(wireframe){	
			glDisable(GL_TEXTURE_2D);    
			// set drawing color to current hue, and draw with thicker wireframe lines
			glColor3f(white[0], white[1], white[2]);
			glLineWidth(2);
		}
		else{
			// set up material color to be white
			if(TextureMode != NOTEXTURE){
				curMat->createTexture();
				glEnable(GL_TEXTURE_2D);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, ColorMode);
				glBindTexture(GL_TEXTURE_2D, curMat->textureid);	    // set the active texture
				switch(TextureMode){
					case NEARTEXTURE:
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
						break;
					case LINEARTEXTURE:
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						break;
					case MIPMAPTEXTURE:
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		 				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						break;
				}
			}
		}
 
	 	//if no vertex normals defined, use face normal
		if(!hasVertexNormals)
			glNormal3f(curFace.normal[0], curFace.normal[1], curFace.normal[2]);
		//Draw the face
		glBegin(op);
			//loop through vertices
			for(j = 0; j < curFace.nverts; j++){
				//if vector normals defined, tell opengl
				if(hasVertexNormals){
					Vector3d vertNorm = psurf->Normals()[curFace.faceverts[j].n];
					glNormal3f(vertNorm[0], vertNorm[1], vertNorm[2]);
				}
				//texture coords
				bool hasTextureCoords = (psurf->Faces()[i].faceverts[j].u != -1);
				if(hasTextureCoords){
					Vector2d vertUV = psurf->UVs()[curFace.faceverts[j].u];
					glTexCoord2f(vertUV[0], vertUV[1]);
				}
				//draw the vertex
				Vector3d curVert = psurf->Vertices()[curFace.faceverts[j].v];
				glVertex3f(curVert[0], curVert[1], curVert[2]);
			}
		glEnd();	
	}

	glDisable(GL_TEXTURE_2D);
}

//
// Display callback
//
void doDisplay(){
	// distant light source, parallel rays coming from front upper right
	float light_position[4];// = {100, 100, 100, 0};
  
	// clear the window to the background color
	glClear(GL_COLOR_BUFFER_BIT);
  
	if(!Wireframe)
		glClear(GL_DEPTH_BUFFER_BIT);  // solid - clear depth buffer
	
	if(!Wireframe && SmoothShading)  // establish shading model, flat or smooth
		glShadeModel(GL_SMOOTH);
	else
		glShadeModel(GL_FLAT);
  
	// light is positioned in camera space so it does not move with object
	glLoadIdentity();

	int i = 0;
	for(std::vector<Vector3d>::iterator iter = gl_lightspos.begin(); iter != gl_lightspos.end(); ++iter){
	    light_position[0] = (*iter)[0];
	    light_position[1] = (*iter)[1];
	    light_position[2] = (*iter)[2];
	    light_position[3] = 0;
	    int n = 0;
	    switch(i){
		    case 0:
			    n = GL_LIGHT0;
			    break;
		    case 1:
			    n = GL_LIGHT1;
			    break;
		    case 2:
			    n = GL_LIGHT2;
			    break;
	    }
	    i++; 
	    glLightfv(n, GL_POSITION, light_position);
	    glLightfv(n, GL_AMBIENT, WHITE);
	    glLightfv(n, GL_DIFFUSE, WHITE);
	    glLightfv(n, GL_SPECULAR, WHITE);
	}

	// establish camera coordinates
	glRotatef(Tilt, 1, 0, 0);	    // tilt - rotate camera about x axis
	glRotatef(Pan, 0, 1, 0);	    // pan - rotate camera about y axis
	glTranslatef(0, 0, Approach);     // approach - translate camera along z axis

  
	// rotate the model
	glRotatef(ThetaY, 0, 1, 0);       // rotate model about x axis
	glRotatef(ThetaX, 1, 0, 0);       // rotate model about y axis
	
	glPushMatrix();
		glLoadIdentity();
		glRotatef(-ThetaX, 1, 0, 0);       // rotate model about y axis
		glRotatef(-ThetaY, 0, 1, 0);       // rotate model about x axis
		glTranslatef(0, 0, -Approach);     // approach - translate camera along z axis
		glRotatef(-Pan, 0, 1, 0);	    // pan - rotate camera about y axis
		glRotatef(-Tilt, 1, 0, 0);	    // tilt - rotate camera about x axis
		glGetFloatv (GL_MODELVIEW_MATRIX, mv);
	glPopMatrix();

	//opengl is column major, matrix library row major
	Matrix4x4 m4 = Matrix4x4(mv[0], mv[4], mv[8], mv[12],
				 mv[1], mv[5], mv[9], mv[13],
				 mv[2], mv[6], mv[10], mv[14],
				 mv[3], mv[7], mv[11], mv[15]);
//	std::cout<< "cam mod matrix: "<< std::endl<< m4<< std::endl;

	Vector4d camPos4 = Vector4d(0., 0., 0., 1.);
	camPos4 = m4 * camPos4;
	Vector4d camDir4 = Vector4d(0., 0, -1., 0.);
	camDir4 = m4 * camDir4;
	Vector4d camUp4 = Vector4d(0., 1, 0., 0.);
	camUp4 = m4 * camUp4;

	rt_lightspos.clear();
	for(std::vector<Vector3d>::iterator iter = gl_lightspos.begin(); iter != gl_lightspos.end(); ++iter){
	    Vector4d rt_lightpos4 = Vector4d((*iter)[0],(*iter)[1],(*iter)[2],1);
	    rt_lightpos4 = m4 * rt_lightpos4;
	    rt_lightspos.push_back(Vector3d(rt_lightpos4[0],rt_lightpos4[1],rt_lightpos4[2]));
	}

	
	camPos3 = Vector3d(camPos4[0], camPos4[1], camPos4[2]);
	camDir3 = Vector3d(camDir4[0], camDir4[1], camDir4[2]);
	camUp3 = Vector3d(camUp4[0], camUp4[1], camUp4[2]);

	cam->setDir(camDir3);
	cam->setPinhole(camPos3);
	cam->setUp(camUp3);

	drawModel(Wireframe);
    
	glutSwapBuffers();
}

//
// Reshape Callback
// Keep viewport the entire screen
//
void doReshape(int width, int height){
  
	glViewport(0, 0, width, height);
	Width = width;
	Height = height;
	
	updateProjection();
}

// Keyboard callback routine. 
// Set various modes or take actions based on key presses
//
void handleKey(unsigned char key, int x, int y){
  
	switch(key){

		case 'p':			// P -- toggle between ortho and perspective
		case 'P':
			Projection = !Projection;
			updateProjection();
			glutPostRedisplay();
			break;
            
		case 'i':			// I -- reinitialize 
		case 'I':
			setInitialState();
			updateProjection();
			glutPostRedisplay();
			break;
      
		case 'm':			// M -- cycle through color modes
		case 'M':
			ColorMode = (ColorMode == GL_MODULATE? GL_DECAL: GL_MODULATE);
			glutPostRedisplay();
			break;
      
		case 't':			// T -- cycle through texture modes
		case 'T':
			TextureMode = (TextureMode == MIPMAPTEXTURE? NOTEXTURE: TextureMode + 1);
			glutPostRedisplay();
			break;
  
		case 'q':			// Q or Esc -- exit program
		case 'Q':
		case ESC:
			exit(0);

		case 'r':
		case 'R':
			doReshape(Width, Height);
			updateProjection();
			raytrace(args, saveName, Nrays, wFileExists, psurf, image, cam, WORLDWIDTH, WORLDWIDTH * (Height/Width), !Projection, mv,rt_lightspos,Width);
			break;
      
		case 's':			// S -- toggle between flat and smooth shading
		case 'S':
			SmoothShading = !SmoothShading;
			glutPostRedisplay();
			break;
      
		case 'w':			// M -- toggle between wireframe and shaded viewing
		case 'W':
			Wireframe = !Wireframe;
			if(Wireframe){
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_LIGHTING);
			}
			else{
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_LIGHTING);
			}
			glutPostRedisplay();
			break;
	}
}

//
// Mouse Button Callback
// on button press, record mouse position and which button is pressed
//
void handleButtons(int button, int state, int x, int y){
  
	if(state == GLUT_UP)
		Button = NONE;		// no button pressed
	else{
		MouseY = -y;		// invert y window coordinate to correspond with OpenGL
		MouseX = x;
    
		Button = button;		// store which button pressed
	}
}

//
// Mouse Motion Callback
// when mouse moves with a button down, update appropriate camera parameter
//
void handleMotion(int x, int y){
	int delta;
  
	y = -y;
	int dy = y - MouseY;
	int dx = x - MouseX;
  
	switch(Button){
		case GLUT_LEFT_BUTTON:
			ThetaX -= ROTFACTOR * dy;
			ThetaY += ROTFACTOR * dx;
			glutPostRedisplay();
			break;
		case GLUT_MIDDLE_BUTTON:
			Pan -= ROTFACTOR * dx;
			Tilt += ROTFACTOR * dy;
			glutPostRedisplay();
			break;
		case GLUT_RIGHT_BUTTON:
			delta = (fabs(dx) > fabs(dy)? dx: dy);
			Approach += XLATEFACTOR * delta;
			glutPostRedisplay();
			break;
	}
  
	MouseX = x;
	MouseY = y;
}

//
// Initialize OpenGL to establish lighting and colors
// and initialize viewing and model parameter
//
void initialize(){
  
	// initialize modelview matrix to identity
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
  
	// specify window clear (background) color to be black
	glClearColor(0, 0, 0, 0);
  
	// position light and turn it on
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
  
	// initialize viewing and model parameters
	setInitialState();
	updateProjection();
  
	//read in OBJFile
	objfile.read(); 
 
	psurf = objfile.getscene();

	// This is texture map sent to texture memory without mipmapping:
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTUREWIDTH, TEXTUREHEIGHT,
	//	       0, GL_RGBA, GL_UNSIGNED_BYTE, TextureImage);  
}

//
// Main program to create window, setup callbacks, and initiate GLUT
//
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

	args = argv;

	//add to the lights vector
	gl_lightspos.push_back(Vector3d(100,100,100));
	gl_lightspos.push_back(Vector3d(-100,100,40));

	cam = new Camera(Vector3d(0., 0., 0.), Vector3d(0., 0., -1.), Vector3d(0., 1., 0.), 1.0);
	// start up the glut utilities
	glutInit(&argc, argv);
  
	// create the graphics window, giving width, height, and title text
	// and establish double buffering, RGBA color
	// Depth buffering must be available for drawing the shaded model
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("3D Texture Viewer with Mipmaps");
  
	// register callback to draw graphics when window needs updating
	glutDisplayFunc(doDisplay);
	glutReshapeFunc(doReshape);
	glutKeyboardFunc(handleKey);
	glutMouseFunc(handleButtons);
	glutMotionFunc(handleMotion);
  
	initialize();
  
	glutMainLoop();
}
