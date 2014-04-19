#ifndef RGBApixel_H
#define RGBApixel_H
#include "RGBApixel.h"
#endif

class ViewScreen{

public:
	ViewScreen() : pwidth(250), pheight(200), wwidth(0.5), wheight(0.4){
		initPixmap();
	}

	ViewScreen(int pw, int ph, double ww, double wh): pwidth(pw), pheight(ph), wwidth(ww), wheight(wh){
		initPixmap();
	}

	ViewScreen(const ViewScreen& vs): pwidth(vs.pwidth), pheight(vs.pheight), wwidth(vs.wwidth), wheight(vs.wheight){
		initPixmap();
	}

	int getPwidth();
	int getPheight();
	double getWwidth();
	double getWheight();
	void setPixelAt(int x, int y, RGBApixel color);

	RGBApixel **pixmap;

private:
	int pwidth, pheight;
	double wwidth, wheight;

	void initPixmap();

};
