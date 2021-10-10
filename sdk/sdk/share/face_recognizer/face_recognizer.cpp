//////////////////////////////////////////////////////////////////////////////////////
// OnlineFaceRec.cpp, by powei
//////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#if defined WIN32 || defined _WIN32
	#include <conio.h>		// For _kbhit() on Windows
	#include <direct.h>		// For mkdir(path) on Windows
	#include <Windows.h>
	#define snprintf sprintf_s	// Visual Studio on Windows comes with sprintf_s() instead of snprintf()
#else
	#include <stdio.h>		// For getchar() on Linux
#endif
#include <vector>
#include <string>
#include "cv.h"
#include "cvaux.h"
#include "highgui.h"
#include "face_recognizer.h"

extern FaceRecognizer PCARecognizer ;
extern FaceRecognizer LDARecognizer ;
extern FaceRecognizer LBPRecognizer ;

FaceRecognizer *createEigenFaceRecognizer(void)
{
	PCARecognizer.create();
	return &PCARecognizer;	
}

void releaseEigenFaceRecognizer(void)
{
	PCARecognizer.release();
}

FaceRecognizer *createFisherFaceRecognizer(void)
{
	LDARecognizer.create();
	return &LDARecognizer;
}

void releaseFisherFaceRecognizer(void)
{
	LDARecognizer.release();
}

FaceRecognizer *createLBPFaceRecognizer(void)
{
	LBPRecognizer.create();
	return &LBPRecognizer;
}

void releaseLBPFaceRecognizer(void)
{
	LBPRecognizer.release();
}
