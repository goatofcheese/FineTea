CC      = g++
C	= cpp
H	= h

CFLAGS		= -g `Magick++-config --cppflags`
LFLAGS		= -g `Magick++-config --ldflags`

ifeq ("$(shell uname)", "Darwin")
  LDFLAGS     = -framework Foundation -framework GLUT -framework OpenGL -lm
else
  ifeq ("$(shell uname)", "Linux")
    LDFLAGS     = -lglut -lGL -lGLU -lMagick++ -lm
  endif
endif

HFILES = File.${H} OBJFile.${H} MTLFile.${H} ImageFile.${H} PolySurf.${H} Face.${H} Line.${H} Group.${H} NewMaterial.${H} Color.${H} Pixmap.${H} Vector.${H} Utility.${H} MakeSpace.${H} Camera.${H} ViewScreen.${H} Sphere.${H} Light.${H} PointLight.${H} Material.${H} ParallelLight.${H} Matrix.${H}
OFILES = File.o OBJFile.o MTLFile.o ImageFile.o PolySurf.o Face.o Line.o Group.o NewMaterial.o Pixmap.o Color.o Vector.o Utility.o Camera.o ViewScreen.o Sphere.o Light.o PointLight.o Material.o ParallelLight.o Matrix.o

PROJECT = objtrace

${PROJECT}:	${PROJECT}.o ${OFILES}
	${CC} ${LFLAGS} -o ${PROJECT} ${PROJECT}.o ${OFILES} ${LDFLAGS}

${PROJECT}.o: ${PROJECT}.${C} ${HFILES}
	${CC} ${CFLAGS} -c ${PROJECT}.${C}

File.o:  File.${C} File.${H}
	${CC} ${CFLAGS} -c File.${C}

OBJFile.o:  OBJFile.${C} File.${H} OBJFile.${H} MTLFile.${H} Vector.${H} Utility.${H}
	${CC} ${CFLAGS} -c OBJFile.${C}

MTLFile.o:  MTLFile.${C} File.${H} MTLFile.${H} ImageFile.${H} Color.${H} PolySurf.${H} Pixmap.${H}
	${CC} ${CFLAGS} -c MTLFile.${C}

ImageFile.o:  ImageFile.${C} File.${H} ImageFile.${H} Pixmap.${H} 
	${CC} ${CFLAGS} -c ImageFile.${C}

PolySurf.o:  PolySurf.${C} PolySurf.${H} Pixmap.${H} Vector.${H} Utility.${H}  MakeSpace.${H}
	${CC} ${CFLAGS} -c PolySurf.${C}

Face.o:  Face.${C} Face.${H} Color.${H} Pixmap.${H} Vector.${H} Utility.${H}
	${CC} ${CFLAGS} -c Face.${C}

Line.o:  Line.${C} Line.${H} Vector.${H} Utility.${H} MakeSpace.${H}
	${CC} ${CFLAGS} -c Line.${C}

Group.o:  Group.${C} Group.${H} MakeSpace.${H}
	${CC} ${CFLAGS} -c Group.${C}

NewMaterial.o:  NewMaterial.${C} NewMaterial.${H} Color.${H} Pixmap.${H} Vector.${H} Utility.${H}
	${CC} ${CFLAGS} -c NewMaterial.${C}

Pixmap.o:  Pixmap.${C} Pixmap.${H} 
	${CC} ${CFLAGS} -c Pixmap.${C}

Color.o:  Color.${C} Color.${H} 
	${CC} ${CFLAGS} -c Color.${C}
	
Vector.o:  Vector.${C} Vector.${H} Utility.${H} 
	${CC} ${CFLAGS} -c Vector.${C}

Matrix.o: Matrix.${C} Vector.${H} Utility.${H}
	${CC} $(CFLAGS) -c Matrix.${C}

Utility.o:  Utility.${C} Utility.${H}
	${CC} ${CFLAGS} -c Utility.${C}

MakeSpace.o:  MakeSpace.${C} MakeSpace.${H}
	${CC} ${CFLAGS} -c MakeSpace.${C}

Camera.o: Camera.${C} Camera.${H}
	${CC} $(CFLAGS) -c Camera.${C}

ViewScreen.o: ViewScreen.${C} ViewScreen.${H}
	${CC} $(CFLAGS) -c ViewScreen.${C}

Sphere.o: Sphere.${C} Sphere.${H}
	${CC} $(CFLAGS) -c Sphere.${C}

Light.o: Light.${C} Light.${H}
	${CC} $(CFLAGS) -c Light.${C}

PointLight.o: PointLight.${C} PointLight.${H}
	${CC} $(CFLAGS) -c PointLight.${C}

Material.o: Material.${C} Material.${H}
	${CC} $(CFLAGS) -c Material.${C}

ParallelLight.o: ParallelLight.${C} ParallelLight.${H}
	${CC} $(CFLAGS) -c ParallelLight.${C}

clean:
	rm -f *.o *~ core.* ${PROJECT}
