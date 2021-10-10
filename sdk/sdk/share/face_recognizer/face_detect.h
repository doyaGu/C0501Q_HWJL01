
#ifndef __FACE_DETECT__H__
#define __FACE_DETECT__H__

#include <cv.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EYE0_X	51   // 0.2 s23.8
#define EYE1_X	153  // 0.6 
#define EYE0_Y	78   // 0.3
#define EYE1_Y	78   // 0.3 
#define EYE_WIDTH	51 // 0.2
#define EYE_HEIGHT	51 // 0.2
#define REFLACTION_THRESHOLD		220
#define EYE_PUPIL_THRESHOLD		15
#define EYE_IRIS_THRESHOLD		150
#define EYE_THRESHOLD		2000

#define EYE_LENGTH		0.45 // compare to face size X

typedef struct FaceRange {
	CvRect	rect;
	int		min;
	int		max;
	float	scale;
	int		sizeX;
	int		sizeY;
	int		isDetectOnly;
} FaceRange;

/****************************************************************************************\
*                                  face detect                                         *
\****************************************************************************************/
IplImage* face_detect(IplImage* frameImg, FaceRange *range);
int face_detect_status(CvRect *faceRect, CvPoint	*eyePoint);

#ifdef __cplusplus
}
#endif

#endif

/* End of file. */
