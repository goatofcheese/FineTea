James Wells
HW5 -- Textured Object Viewer
	 -- punless :(

Overview
	raytracing program capable of polygonal models defined by .obj files

Files
	objtace.cpp 	- The main driver. Reads obj file, handles glut functions, casts and traces super sampling rays recursively and shades appropriately

	Camera.h 			- header file for Camera class
							- defines pinhole point for perspective ray fire and focal distance to ViewScreen
	Camera.cpp 			- implementation of Camera functions

	ViewScreen.h 		- header file for ViewScreen class
							- Defines pixel dimensions for the image as well as the world dimensions to be shot at
							- stores pixmap of image data
	ViewScreen.cpp 	- implementation of ViewScreen functions

	Sphere.h 			- header file for Sphere class
							- has center and radius to define it
							- can be tested for hits for a ray from a base
	Sphere.cpp 			- implementation of Sphere functions

	Light.h				- header file for Light class
							- sets up virtual functions for light classes
	ParallelLight.h	- header file for ParallelLight class
							- defined by a ray
	PointLight.h 		- header file for PointLight class
							- defined by point in space
	camera.txt			- camera configuration file
	
	brazil.png			- example of brazil model with a few spheres
	tetrahedron.png	- example of tetrahedron model with a few spheres
