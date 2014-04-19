#include "Material.h"

RGBApixel Material::getColor(){
	return color;
}

double Material::getDiffuse(){
	return diffuse;
}

double Material::getSpecular(){
	return specular;
}


std::ostream& operator<<(std::ostream& out, const Material& m){
	return out << (int)m.color.r << " " << (int)m.color.g
					<< " " << (int)m.color.b<< " " << (int)m.color.a;

}
