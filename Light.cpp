#include "Light.h"

std::ostream& operator<<(std::ostream& out, const Light& l){
	l.print(out);
}

Color Light::getColor(){
	return color;
}
