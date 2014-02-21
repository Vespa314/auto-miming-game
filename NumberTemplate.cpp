#include "NumberTemplate.h"

CNumberTemplate::CNumberTemplate(void)
{
}

void CNumberTemplate::Init( IplImage* img,int n )
{
	if (!img)
		return;
	num = n;
	size.height = img->height;
	size.width = img->width;
	pic = cvCloneImage(img);
	for(int i = 0;i < size.height;i++)
	{
		for(int j = 0;j < size.width;j++)
		{
			if (CV_IMAGE_ELEM(img,uchar,i,3*j) != BACKGROUND_COLOR.val[0] ||
				CV_IMAGE_ELEM(img,uchar,i,3*j+1) != BACKGROUND_COLOR.val[1] ||
				CV_IMAGE_ELEM(img,uchar,i,3*j+2) != BACKGROUND_COLOR.val[2])
			{
				FirstColorPixel2Center = cvPoint(j,i);
				CenterColor = cvScalar(CV_IMAGE_ELEM(img,uchar,i,3*j),
					                   CV_IMAGE_ELEM(img,uchar,i,3*j+1),
									   CV_IMAGE_ELEM(img,uchar,i,3*j+2));
			}
		}
	}
}

CNumberTemplate::~CNumberTemplate(void)
{
}
