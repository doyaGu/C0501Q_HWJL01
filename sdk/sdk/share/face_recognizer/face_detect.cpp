// face_rec.cpp : 定義主控台應用程式的進入點。
//
#include "cv.h"
//#include "opencv/ml.h"
#include <stdio.h>
#include <math.h>
#include "haarcascade_frontalface_alt.h"
#include "highgui.h"
#include "face_detect.h"

//#define DEBUG
/* 
11x11
0   0   0   0   0   1   0   0   0   0   0
0   0   0   1   0   0   0   1   0   0   0
0   0   0   0   0   0   0   0   0   0   0  
0   1   0   0   0   0   0   0   0   1   0
0   0   0   0   0   x   0   0   0   0   0
1   0   0   0   x   x   x   0   0   0   1
0   0   0   0   0   x   0   0   0   0   0
0   1   0   0   0   0   0   0   0   1   0
0   0   0   0   0   0   0   0   0   0   0  
0   0   0   1   0   0   0   1   0   0   0
0   0   0   0   0   1   0   0   0   0   0
*/
int Filter11x11[12][2] = {
	{0,-5}, {-2,-4}, {2,-4}, {-4,-2}, {4,-2}, {-5,0}, {5,0}, {-4,2}, {4,2}, {-2,4}, {2,4}, {0,5}
};

/*
13x13
0   0   0   0   0   0   1   0   0   0   0   0   0
0   0   0   1   0   0   0   0   0   1   0   0   0
0   0   0   0   0   0   0   0   0   0   0   0   0
0   1   0   0   0   0   0   0   0   0   0   1   0
0   0   0   0   0   0   0   0   0   0   0   0   0
0   0   0   0   0   0   x   0   0   0   0   0   0
1   0   0   0   0   x   x   x   0   0   0   0   1
0   0   0   0   0   0   x   0   0   0   0   0   0
0   0   0   0   0   0   0   0   0   0   0   0   0
0   1   0   0   0   0   0   0   0   0   0   1   0
0   0   0   0   0   0   0   0   0   0   0   0   0
0   0   0   1   0   0   0   0   0   1   0   0   0
0   0   0   0   0   0   1   0   0   0   0   0   0
*/
int Filter13x13[12][2] = {
	{0,-6}, {-3,-5}, {3,-5}, {-5,-3}, {5,-3}, {-6,0}, {6,0}, {-5,3}, {5,3}, {-3,5}, {3,5}, {0,6}
};

/*
15x15
0   0   0   0   0   0   0   1   0   0   0   0   0   0   0
0   0   0   0   1   0   0   0   0   0   1   0   0   0   0
0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
0   1   0   0   0   0   0   0   0   0   0   0   0   1   0
0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
0   0   0   0   0   0   0   x   0   0   0   0   0   0   0
1   0   0   0   0   0   x   x   x   0   0   0   0   0   1
0   0   0   0   0   0   0   x   0   0   0   0   0   0   0
0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
0   1   0   0   0   0   0   0   0   0   0   0   0   1   0
0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
0   0   0   0   0   0   0   0   0   0   0   0   0   0   0
0   0   0   0   1   0   0   0   0   0   1   0   0   0   0
0   0   0   0   0   0   0   1   0   0   0   0   0   0   0
*/
int Filter15x15[12][2] = {
	{0,-7}, {-3,-6}, {3,-6}, {-6,-3}, {6,-3}, {-7,0}, {7,0}, {-6,3}, {6,3}, {-3,6}, {3,6}, {0,7}
};

int FilterCenter[5][2] = {
	{0,-1}, {-1,0}, {0,0}, {1,0}, {0,1}
};

//using namespace std;

static CvRect	face_detect_rect;
static CvPoint	face_detect_eye[2];
static int face_detected;

//const char cascadeName[] = "opencv\\data\\haarcascades\\haarcascade_frontalface_alt.xml";

static int detectfaces( IplImage *img, int search_min, int search_max, float scale, CvRect *face)
{
    int64 t = 0;//偵測速度
	CvSeq* rects;
	CvRect *fRect;
	CvMemStorage* storage;
	CvHaarClassifierCascade* cascade;
	const int flags = CV_HAAR_SCALE_IMAGE;
	const CvScalar redcolor = CV_RGB(255,0,0);
	int		nfaces;

	// Load the HaarCascade classifier for face detection.
	cascade = &haarcascade_frontalface_alt;
	/*cascade = (CvHaarClassifierCascade*)cvLoad(cascadeName, 0, 0, 0 );
	if( !cascade ) {
		printf("ERROR in recognizeFromCam(): Could not load Haar cascade Face detection classifier in '%s'.\n", cascadeName);
		return 0;
	}*/

    //cvEqualizeHist(smallImg, smallImg);//直方圖均衡化

	storage = cvCreateMemStorage(0);
	cvClearMemStorage( storage );

    //t = (int64)cvGetTickCount();
	rects = cvHaarDetectObjects( img, (CvHaarClassifierCascade*)cascade, storage,
				(int)(scale*256), 3, flags, 
				cvSize(search_min,search_min), 
				cvSize(search_max,search_max) );
    //t = (int64)cvGetTickCount() - t;
    //printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) ); 
    //printf("FPS = %f\n",1000/(t/((double)cvGetTickFrequency()*1000.)));

	nfaces = rects->total;
	if (nfaces > 0) {
        fRect = (CvRect*)cvGetSeqElem( rects, 0 );
		face->x = fRect->x;
		face->y = fRect->y;
		face->width = fRect->width;
		face->height = fRect->height;
		//printf("face size = %d\n", fRect->width);
    }
	//cvReleaseHaarClassifierCascade( &cascade );
	cvReleaseMemStorage(&storage);

	return nfaces;
}

static int detecteyes(IplImage *img, CvRect face_rect, CvPoint *eyes)
{
		int i,j,k;
		int min0 = 65535;
		int min1 = 65535;
		CvPoint loc0 = {0}, loc1 = {0};
		CvRect	org_roi;
		int ret = 0;
		int eye_width = (face_rect.width*EYE_WIDTH)>>8;
		int eye_height = (face_rect.width*EYE_HEIGHT)>>8;
		CvRect eye0rect = {face_rect.x+((face_rect.width*EYE0_X)>>8), face_rect.y+((face_rect.height*EYE0_Y)>>8), eye_width, eye_height};
		CvRect eye1rect = {face_rect.x+((face_rect.width*EYE1_X)>>8), face_rect.y+((face_rect.height*EYE1_Y)>>8), eye_width, eye_height};
		CvMat *eye0sum = cvCreateMat(eye_height, eye_width, CV_16UC1);
		CvMat *eye1sum = cvCreateMat(eye_height, eye_width, CV_16UC1);
		int  (*filter)[2];

		org_roi.x = img->roi->xOffset;
		org_roi.y = img->roi->yOffset;
		org_roi.width = img->roi->width;
		org_roi.height = img->roi->height;

		eye0rect.x += org_roi.x;
		eye0rect.y += org_roi.y;
		eye1rect.x += org_roi.x;
		eye1rect.y += org_roi.y;

		//chose filter
		if (face_rect.width<300)
			filter = Filter11x11;
		else if (face_rect.width<350)
			filter = Filter13x13;
		else
			filter = Filter15x15;

		// filter to emphasize the eye part and find minimum
		for(j=0;j<eye_height;j++) {	
			for(i=0;i<eye_width;i++) {
				int max = 0, min = 255;
				int tmp = 0;
				int center = (unsigned char)img->imageData[(eye0rect.y+j)*img->width+eye0rect.x+i];
				eye0sum->data.s[j*eye0sum->cols+i]  = 65535;
				if (center>REFLACTION_THRESHOLD) // glass reflaction
					return -1;
				//iris upper part
				for (k=0;k<5;k++)  {
					int fx = filter[k][0];
					int fy =  filter[k][1];
					tmp += (unsigned char)img->imageData[(eye0rect.y+j+fy)*img->width+eye0rect.x+i+ fx];
				}
				//iris lower part
				for (k=5;k<12;k++)  {
					int fx = filter[k][0];
					int fy =  filter[k][1];
					int data = (unsigned char)img->imageData[(eye0rect.y+j+fy)*img->width+eye0rect.x+i+ fx];
					if (data<EYE_IRIS_THRESHOLD && center>data+EYE_PUPIL_THRESHOLD) {
						if (data<min) min = data;
						if (data>max) max = data;
						tmp += data*2;
					} else {
						tmp = 0xff*12;
						break;
					}
				}
				if (max>min+EYE_PUPIL_THRESHOLD) tmp = 0xff*12;					

				for (k=0;k<5;k++) {
					int fx = FilterCenter[k][0];
					int fy =  FilterCenter[k][1];
					tmp -= (unsigned char)img->imageData[(eye0rect.y+j+fy)*img->width+eye0rect.x+i+ fx];
				}
				eye0sum->data.s[j*eye0sum->cols+i] = (tmp<0) ? 0 : tmp;
				if(tmp<min0) {
					loc0.y = j;
					loc0.x = i;
					min0 = tmp;
				}
			}
		}

		for(j=0;j<eye_height;j++) {	
			for(i=0;i<eye_width;i++) {
				int max = 0, min = 255;
				int tmp = 0;
				int center = (unsigned char)img->imageData[(eye1rect.y+j)*img->width+eye1rect.x+i];
				eye1sum->data.s[j*eye1sum->cols+i]  = 65535;
				if (center>REFLACTION_THRESHOLD)  // glass reflaction
					return -1;
				for (k=0;k<5;k++) 
					tmp += (unsigned char)img->imageData[(eye1rect.y+j+filter[k][1])*img->width+eye1rect.x+i+filter[k][0]];
				for (k=5;k<12;k++)  {
					int data = (unsigned char)img->imageData[(eye1rect.y+j+filter[k][1])*img->width+eye1rect.x+i+filter[k][0]];
					if (data<EYE_IRIS_THRESHOLD && center>data+EYE_PUPIL_THRESHOLD) {
						if (data<min) min = data;
						if (data>max) max = data;
						tmp += data*2;
					} else {
						tmp = 0xff*12;
						break;
					}
				}
				if (max>min+EYE_PUPIL_THRESHOLD) tmp = 0xff*12;					

				for (k=0;k<5;k++) 
					tmp -= (unsigned char)img->imageData[(eye1rect.y+j+FilterCenter[k][1])*img->width+eye1rect.x+i+FilterCenter[k][0]];
				eye1sum->data.s[j*eye1sum->cols+i] = (tmp<0) ? 0 : tmp;
				if(tmp<min1) {
					loc1.y = j;
					loc1.x = i;
					min1 = tmp;
				}
			}
		}
#if 0
		{ 
			printf("min0 = %d, min1 = %d\n", min0, min1);
			IplImage *dstImg = cvCreateImage(cvSize(eye_width, eye_height), 8, 1);
			cvConvertScale(eye1sum, dstImg, 1./12);
			//cvShowImage( "lbp", dstImg );
			cvSaveImage("result/eye.pgm", dstImg);
			cvReleaseImage(&dstImg);			
		}
#endif
		//printf("eye0: (%d,%d) min= %d, size=%d,%d\n",loc0.x,loc0.y,min0, eye_width,eye_height);
		//printf("eye1: (%d,%d) min= %d\n",loc1.x,loc1.y,min1);

		if( min0<EYE_THRESHOLD) {
			eyes[0] = cvPoint(eye0rect.x+loc0.x-org_roi.x, eye0rect.y+loc0.y-org_roi.y);
			ret++;
		} else
			eyes[0] = cvPoint(eye0rect.x+eye0rect.width/2-org_roi.x, eye0rect.y+eye0rect.height/2-org_roi.y);

		if( min1<EYE_THRESHOLD) {
			eyes[1] = cvPoint(eye1rect.x+loc1.x-org_roi.x, eye1rect.y+loc1.y-org_roi.y);
			ret++;
		} else
			eyes[1] = cvPoint(eye1rect.x+eye0rect.width/2-org_roi.x, eye1rect.y+eye0rect.height/2-org_roi.y);

		cvReleaseMat(&eye0sum);
		cvReleaseMat(&eye1sum);
	
	return ret;
}

IplImage* face_detect(IplImage* frameImg, FaceRange *range)
{
    IplImage *faceGray = 0, *dstGray = 0;
	CvPoint eyes[2];//記錄臉的兩個眼睛位置
	CvRect	rect;
	const CvScalar redcolor = CV_RGB(255,0,0);
	const CvScalar bluecolor = CV_RGB(0,0,255);
	int cx, cy, dx, dy, cnt, i = 0;
	float angle, eye_len, scale;
    double m[6];
    // [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]
    // [ m3  m4  m5 ]       [ A21  A22   b2 ]
    CvMat M = cvMat(2, 3, CV_64FC1, m);

	face_detected = 0;

	cvSetImageROI(frameImg, cvRect(range->rect.x, range->rect.y, range->rect.width, range->rect.height));
    cnt = detectfaces(frameImg, range->min, range->max, range->scale, &rect);

	if (!cnt) {
		printf("face not found!\n");
		goto end;
	} else if (cnt>1) {
		printf("multi-faces found!\n");
		goto end;
	}	

	face_detect_rect.x = rect.x + range->rect.x;
	face_detect_rect.y = rect.y + range->rect.y;
	face_detect_rect.width = rect.width;
	face_detect_rect.height = rect.height;
	printf("detect width = %d, height=%d\n", rect.width, rect.height);

	cnt = detecteyes(frameImg, rect, eyes);
	if (cnt < 0) {		
		printf("Glasses is reflected!\n");		
		goto end;
	} else if (!cnt) {
		printf("No eyes found!\n");
		goto end;
	} else if (cnt==1) {
		printf("Only one eye found!\n");1;
		goto end;
	} 

	face_detect_eye[0].x  = eyes[0].x +  range->rect.x;
	face_detect_eye[0].y  = eyes[0].y +  range->rect.y;
	face_detect_eye[1].x  = eyes[1].x +  range->rect.x;
	face_detect_eye[1].y  = eyes[1].y +  range->rect.y;

	face_detected = 1;

	if (range->isDetectOnly)
		goto end;

    faceGray = cvCreateImage (cvSize(range->rect.width, range->rect.height), IPL_DEPTH_8U, 1);
	dx = eyes[1].x-eyes[0].x;
	dy = eyes[1].y-eyes[0].y;
	angle = atan((float)dy/dx)*180.0/CV_PI;
	eye_len = sqrt((float)dx*dx + dy*dy);
	scale = (float)EYE_LENGTH*range->sizeX/eye_len;
	cx = (eyes[0].x+eyes[1].x)/2;
	cy = (eyes[0].y+eyes[1].y)/2;
	cv2DRotationMatrix(cvPoint2D32f(cx, cy),angle,scale, &M);
	cvWarpAffine(frameImg, faceGray, &M);
	cvSetImageROI(faceGray, cvRect(cx-range->sizeX/2, cy-range->sizeY*5/16,  range->sizeX, range->sizeY));
    dstGray = cvCreateImage (cvSize(range->sizeX, range->sizeY), IPL_DEPTH_8U, 1);
	cvCopy(faceGray, dstGray);
	cvReleaseImage(&faceGray);			

#ifdef DEBUG
	{
		CvRect *r = &rect;
		int eye_width = (r->width*EYE_WIDTH)>>8;
		int eye_height = (r->width*EYE_HEIGHT)>>8;
		CvRect eye0rect = {r->x+((r->width*EYE0_X)>>8), r->y+((r->height*EYE0_Y)>>8), eye_width, eye_height};
		CvRect eye1rect = {r->x+((r->width*EYE1_X)>>8), r->y+((r->height*EYE1_Y)>>8), eye_width, eye_height};

	    cvSetImageROI(frameImg, cvRect(range->rect.x, range->rect.y, range->rect.width, range->rect.height));
		cvRectangle(frameImg, cvPoint(r->x, r->y), cvPoint(r->x+r->width, r->y+r->height*range->sizeY/range->sizeX), redcolor);

		cvRectangle(frameImg, cvPoint(eye0rect.x, eye0rect.y), cvPoint(eye0rect.x+eye0rect.width, eye0rect.y+eye0rect.height), redcolor);
		cvRectangle(frameImg, cvPoint(eye1rect.x, eye1rect.y), cvPoint(eye1rect.x+eye1rect.width, eye1rect.y+eye1rect.height), redcolor);

		for( i=0;i<2;i++) {
			cvRectangle(frameImg, cvPoint(eyes[i].x-1, eyes[i].y-1), cvPoint(eyes[i].x+1, eyes[i].y+1), bluecolor);
		}		
		//cvShowImage( "source", frameImg );
	    cvSaveImage("result/source.pgm", frameImg);
	}
#endif

end:

	return dstGray;
}

int face_detect_status(CvRect *faceRect, CvPoint	*eyePoint)
{
	faceRect->x = face_detect_rect.x;
	faceRect->y = face_detect_rect.y;
	faceRect->width = face_detect_rect.width;
	faceRect->height = face_detect_rect.height;

	eyePoint[0].x = face_detect_eye[0].x ;
	eyePoint[0].y = face_detect_eye[0].y;
	eyePoint[1].x = face_detect_eye[1].x;
	eyePoint[1].y = face_detect_eye[1].y;

	return face_detected;
}
