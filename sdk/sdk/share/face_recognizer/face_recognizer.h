
#ifndef __FACE_RECOGNIZER__H__
#define __FACE_RECOGNIZER__H__

#include <cv.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FACE_WIDTH					   212//100//212
#define FACE_HEIGHT							244//120//244

typedef struct
{
    const char name[64];  // name
    void (*create)(void);
    void (*release)(void);
	int (*load)(const char *filename);
	int (*save)(const char *filename);
	int (*train)(int nTrainFaces, IplImage** faceImgArr);
	int (*predict)(IplImage* faceImg);
	int (*status)(void);
} FaceRecognizer;

/****************************************************************************************\
*                                  Face recognizer                                      *
\****************************************************************************************/
FaceRecognizer *createEigenFaceRecognizer(void);
void releaseEigenFaceRecognizer(void);

FaceRecognizer *createFisherFaceRecognizer(void);
void releaseFisherFaceRecognizer(void);

FaceRecognizer *createLBPFaceRecognizer(void);
void releaseLBPFaceRecognizer(void);

/****************************************************************************************\
*                                  PCA Face recognizer                                      *
\****************************************************************************************/
#define PCA_RECOGNIZED_HIGH			60000
#define PCA_RECOGNIZED_LOW			30000
#define PCA_MAX_COMPONENTS			50

/****************************************************************************************\
*                                  LDA Face recognizer                                      *
\****************************************************************************************/
#define LDA_RECOGNIZED_HIGH			60000
#define LDA_RECOGNIZED_LOW			30000
#define LDA_MAX_COMPONENTS			50

/****************************************************************************************\
*                                  LBP Face recognizer                                      *
\****************************************************************************************/
#define LBP_BLOCK_WIDTH		30
#define LBP_BLOCK_HEIGHT		30
#define LBP_GRID_X						(FACE_WIDTH/LBP_BLOCK_WIDTH)
#define LBP_GRID_Y						(FACE_HEIGHT/LBP_BLOCK_HEIGHT)
#define LBP_BINS							58
#define LBP_DATA_SIZE				(LBP_GRID_X*LBP_GRID_Y*LBP_BINS)
#define LBP_RECOGNIZED_THRESHOLD			20000

int getFaceInfo(IplImage* faceImg, short* faceInfo);
int updateFaceDB(int nTrainFaces, short *faceInfoArr);
int predictFaceInfo(short *testFaceInfo, int index, int threshold);
int getFaceMatchResult(int *score);
int mergeFaceInfo(short *dstFaceInfo, int nFaces, short *srcFaceInfoArr);

#ifdef __cplusplus
}
#endif

#endif

/* End of file. */
