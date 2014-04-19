/*
 *  NewMaterial.cpp
 *  
 *
 *  Created by Donald House on 9/8/08.
 *  Copyright 2008 Clemson University. All rights reserved.
 *
 */

#include "NewMaterial.h"

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <cstring>

NewMaterial::NewMaterial(char *mname){
  name = NULL;
  setName(mname);
  
  amap = dmap = smap = NULL;
  
  alpha = n = 1.0;
  illum_model = 1;
  
  textureid = -1;

  setProperties(0.2, 0.8, 0, 1);
}

NewMaterial::NewMaterial(const Color &ambient, const Color &diffuse, const Color &specular, 
		   double spexp){
  name = NULL;
  
  amap = dmap = smap = NULL;
  
  alpha = n = 1.0;
  illum_model = 1;
  setProperties(ambient, diffuse, specular, spexp);
}

void NewMaterial::setName(char *mname){
  delete name;
  if(mname == NULL)
    name = NULL;
  else{
    name = new char[strlen(mname) + 1];
    strcpy(name, mname);
  }
}

bool NewMaterial::isNamed(char *mname){
  if(name == NULL || mname == NULL)
    return false;
  return strcmp(mname, name) == 0;
}

void NewMaterial::setProperties(const Color &ambient, const Color &diffuse, const Color &specular,
			     double spexp){
  a = ambient; 
  d = diffuse; 
  s = specular; 
  exp = spexp;
}

void NewMaterial::setProperties(double ambient, double diffuse, double specular,
			     double spexp){
  Color ca, cd, cs;
  ca.set(ambient, ambient, ambient);
  cd.set(diffuse, diffuse, diffuse);
  cs.set(specular, specular, specular);
  setProperties(ca, cd, cs, spexp);
}

void NewMaterial::setK(int k, const Color &c){
  switch(k){
  case 0:
    a = c;
    break;
  case 1:
    d = c;
    break;
  case 2:
    s = c;
    break;
  }
}

void NewMaterial::setTransmission(const Color &c){
  t = c;
}

void NewMaterial::setExp(double spexp){
  exp = spexp;
}

void NewMaterial::setAlpha(double alfa){
  alpha = alfa;
}

void NewMaterial::setIOR(double ior){
  n = ior;
}

void NewMaterial::setIllum(int i){
  illum_model = i;
}

void NewMaterial::setMap(int mtype, Pixmap *p){
  switch(mtype){
    case 0:
      amap = p;
      break;
    case 1:
      dmap = p;
      break;
    case 2:
      smap = p;
      break;
  }
}

void NewMaterial::createTexture(){
  if(illum_model < 1 || dmap == NULL || textureid != -1)
    return;
  glGenTextures(1, &textureid); // OpenGL ID for this texture
  glBindTexture(GL_TEXTURE_2D, textureid);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, dmap->NCols(), dmap->NRows(),		    
		    GL_RGBA, GL_UNSIGNED_BYTE, dmap->Pixels());  
}

ostream& operator<< (ostream& os, const NewMaterial& m){
  os << "[NewMaterial " << m.name << ": a = " << m.a << ", d = " << m.d << 
	", s = " << m.s << ", exp = " << m.exp << "\n";
  os << "     alpha = " << m.alpha << ", t = " << m.t << 
	", illum model = " << m.illum_model << "\n";
  os << "     textures: (" << (m.amap? "ambient": "no ambient") << ", " <<  
			 (m.dmap? "diffuse": "no diffuse") << ", " <<
			 (m.smap? "specular": "no specular") << ")";
  os << "]";
  return os;
}
