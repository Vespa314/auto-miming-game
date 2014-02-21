#pragma once

#include "cv.h"
#include "highgui.h"

#define BACKGROUND_COLOR (cvScalar(192,192,192))

class CNumberTemplate
{
public:
	CNumberTemplate(void);
	~CNumberTemplate(void);

	void Init(IplImage* img,int n);
	IplImage* pic;
	int num;
	CvScalar CenterColor;
	CvPoint FirstColorPixel2Center;
	CvSize size;
};
