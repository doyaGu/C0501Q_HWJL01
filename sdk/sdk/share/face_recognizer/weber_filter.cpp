// face_rec.cpp : 定義主控台應用程式的進入點。
//
#include "cv.h"
#include <stdio.h>
#include <math.h>
#include "highgui.h"

#define WEBER_ALPHA		2
// Get the octant a coordinate pair is in.
#define OCTANTIFY(_x, _y, _o)   do {                            \
    int _t; _o= 0;                                              \
    if(_y<  0)  {            _x= -_x;   _y= -_y; _o += 4; }     \
    if(_x<= 0)  { _t= _x;    _x=  _y;   _y= -_t; _o += 2; }     \
    if(_x<=_y)  { _t= _y-_x; _x= _x+_y; _y=  _t; _o += 1; }     \
} while(0);

static const unsigned int BRAD_PI_SHIFT=14,   BRAD_PI = 1<<BRAD_PI_SHIFT; 
static const unsigned int BRAD_HPI= BRAD_PI/2, BRAD_2PI= BRAD_PI*2; 
static const unsigned int ATAN_ONE = 0x1000, ATAN_FP= 12; 

// atan via CORDIC (coordinate rotations). 
// Returns [0,2pi], where pi ~ 0x4000. 
static unsigned int atan2Cordic(int x, int y) 
{     
	if(y==0) return (x>=0 ? 0 : BRAD_PI);
	int phi;     
	OCTANTIFY(x, y, phi);     
	phi *= BRAD_PI/4;     
	// Scale up a bit for greater accuracy.     
	if(x < 0x10000) {
		x *= 0x1000;         
		y *= 0x1000;     
	}     
	// atan(2^-i) terms using PI=0x10000 for accuracy     
	const unsigned short list[]= {0x4000, 0x25C8, 0x13F6, 0x0A22, 0x0516, 0x028C, 0x0146, 0x00A3,         
		                          0x0051, 0x0029, 0x0014, 0x000A, 0x0005, 0x0003, 0x0001, 0x0001};     
	int i, tmp, dphi=0;     
	for(i=1; i<12; i++) {         
		if(y>=0) {             
			tmp= x + (y>>i);             
			y = y - (x>>i);             
			x = tmp;             
			dphi += list[i];         
		} else {             
			tmp = x - (y>>i);             
			y = y + (x>>i);             
			x = tmp;             
			dphi -= list[i];         
		}     
	}     
	return phi + (dphi>>2); 
}

void weber_filter(IplImage *srcImg, IplImage *dstImg)
{
	uchar *sd, *dd;
	int i,j;
	//uchar k[] = { 1, 2, 1, 2, 4, 2, 1, 2, 1 }; 
	uchar k[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 }; 
	CvMat kernel = cvMat(3, 3, CV_8UC1, k);
	CvMat *smooth = cvCreateMat(srcImg->height, srcImg->width, CV_16SC1);
	uchar *data;
	int	  step = 0;
	CvSize size;


	// Gaussian filter
	cvConvert(srcImg, smooth);
	cvFilter2D(smooth, smooth, &kernel);		//s11.4 format
	cvGetImageRawData( dstImg, &data, &step, &size );

	// weber filter
	for (i=0;i<smooth->cols;i++) {
		data[i] = 127;
		data[(smooth->rows-1)*step+i] = 127;
	}

	for(j=1;j<smooth->rows-1;j++) {
		data[j*step] = 127;
		for(i=1;i<smooth->cols-1;i++) {
			int tmp = 8 * smooth->data.s[j*smooth->cols+i]
						- smooth->data.s[(j-1)*smooth->cols+i-1]
						- smooth->data.s[(j-1)*smooth->cols+i]
						- smooth->data.s[(j-1)*smooth->cols+i+1]
						- smooth->data.s[j*smooth->cols+i-1]
						- smooth->data.s[j*smooth->cols+i+1]
						- smooth->data.s[(j+1)*smooth->cols+i-1]
						- smooth->data.s[(j+1)*smooth->cols+i]
						- smooth->data.s[(j+1)*smooth->cols+i+1];
			int arctan = atan2Cordic(smooth->data.s[j*smooth->cols+i], WEBER_ALPHA * abs(tmp));
			arctan = (tmp>0) ? arctan : -arctan;
			data[j*step+i] = (arctan + 0x2000)>>6;			
			printf("");
		}
		data[j*step+i] = 127;
	}

}