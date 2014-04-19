/*
 *  NewMaterial.h
 *  
 *
 *  Created by House on 9/8/08.
 *  Copyright 2008 Clemson University. All rights reserved.
 *
 */

#ifndef _NewMaterial_H_
#define _NewMaterial_H_

#include "Color.h"
#include "Pixmap.h"

//
// Basic structure for storing the shading information about a surface
//
struct NewMaterial{
  char *name;
  
  Color a;
  Color d;
  Color s;
  double exp;
  double alpha;

  Color t;
  double n;

  int illum_model;

  Pixmap *amap;
  Pixmap *dmap;
  Pixmap *smap;
  
  unsigned int textureid;
  
  NewMaterial(char *mname = NULL);
  NewMaterial(const Color &ambient, const Color &diffuse, const Color &specular, 
	   double spexp);

  ~NewMaterial(){};
  
  void setName(char *mname);
  bool isNamed(char *mname);

  void setProperties(const Color &ambient, const Color &diffuse, const Color &specular, 
		     double spexp);
  void setProperties(double ambient, double diffuse, double specular, 
		     double spexp);
  
  void setK(int ktype, const Color &c);
  void setTransmission(const Color &c);
  void setExp(double spexp);
  void setAlpha(double alfa);
  void setIOR(double ior);
  void setIllum(int i);
  void setMap(int mtype, Pixmap *p);

  void createTexture();

  friend ostream& operator<< (ostream& os, const NewMaterial& m);
};

#endif
