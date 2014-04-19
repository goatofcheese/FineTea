#include "ViewScreen.h"
#include <iostream>
#include <cstring>

int ViewScreen::getPwidth(){
	return pwidth;
}

int ViewScreen::getPheight(){
	return pheight;
}

double ViewScreen::getWwidth(){
	return wwidth;
}

double ViewScreen::getWheight(){
	return wheight;
}

void ViewScreen::initPixmap(){
	pixmap = new RGBApixel*[pheight];
	pixmap[0] = new RGBApixel[pwidth * pheight];
	for(int i = 1; i < pheight; i++)
		pixmap[i] = pixmap[i-1] + pwidth;

}

void ViewScreen::setPixelAt(int row, int col, RGBApixel color){
	if(row > pheight || col > pwidth || row < 0 || col < 0 ){
		std::cout<< "don't do that" << std::endl;
		return;
	}
	pixmap[row][col].r = color.r;
	pixmap[row][col].g = color.g;
	pixmap[row][col].b = color.b;
	pixmap[row][col].a = color.a;
}
