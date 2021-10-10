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

using namespace std;

#define BINARY_FILESTORAGE

#define SAVE_EIGENFACE_IMAGES	1		// Set to 0 if you dont want images of the Eigenvectors saved to files (for debugging).
#define SAVE_TRAINFACE_IMAGES	1		// Set to 0 if you dont want images of the trained saved to files (for debugging).
//#define USE_MAHALANOBIS_DISTANCE	// You might get better recognition accuracy if you enable this.

typedef struct _PCARecognizer {
	int					nTrainFaces;			// the number of training images
	int					faceId;
	IplImage		*avgTrainImg;			// the average image // U8 8 format
	int					nEigens;				// the number of eigenvalues
	IplImage		**eigenVectArr;			// eigenvectors
	CvMat			*projectMat;
	//CvMat			*eigenValMat;
} PCA;

static PCA *pca_recognizer = 0;

// Get an 8-bit equivalent of the 32-bit Float image.
// Returns a new image, so remember to call 'cvReleaseImage()' on the result.
static IplImage* convertFloatImageToUcharImage(const IplImage *srcImg)
{
	IplImage *dstImg = 0;
	if ((srcImg) && (srcImg->width > 0 && srcImg->height > 0)) {

		// Spread the 32bit floating point pixels to fit within 8bit pixel range.
		double minVal, maxVal;
		cvMinMaxLoc(srcImg, &minVal, &maxVal);

		//cout << "FloatImage:(minV=" << minVal << ", maxV=" << maxVal << ")." << endl;

		// Deal with NaN and extreme values, since the DFT seems to give some NaN results.
		if (cvIsNaN(minVal) || minVal < -1e30)
			minVal = -1e30;
		if (cvIsNaN(maxVal) || maxVal > 1e30)
			maxVal = 1e30;
		if (maxVal-minVal == 0.0f)
			maxVal = minVal + 0.001;	// remove potential divide by zero errors.

		// Convert the format
		dstImg = cvCreateImage(cvSize(srcImg->width, srcImg->height), 8, 1);
		cvConvertScale(srcImg, dstImg, 255.0 / (maxVal - minVal), - minVal * 255.0 / (maxVal-minVal));
	}
	return dstImg;
}

// Save all the eigenvectors as images, so that they can be checked.
static void saveEigenfaceImages(int nEigens, IplImage** eigenVectArr, IplImage* avgTrainImg)
{
	// Store the average image to a file
	printf("Saving the image of the average face as 'out_averageImage.bmp'.\n");
	cvSaveImage("out_averageImage.bmp", avgTrainImg);

	// Create a large image made of many eigenface images.
	// Must also convert each eigenface image to a normal 8-bit UCHAR image instead of a 32-bit float image.
	printf("Saving the %d eigenvector images as 'out_eigenfaces.bmp'\n", nEigens);
	if (nEigens > 0) {
		// Put all the eigenfaces next to each other.
		int COLUMNS = 8;	// Put upto 8 images on a row.
		int nCols = min(nEigens, COLUMNS);
		int nRows = 1 + (nEigens / COLUMNS);	// Put the rest on new rows.
		int w = eigenVectArr[0]->width;
		int h = eigenVectArr[0]->height;
		CvSize size;
		size = cvSize(nCols * w, nRows * h);
		IplImage *bigImg = cvCreateImage(size, IPL_DEPTH_8U, 1);	// 8-bit Greyscale UCHAR image
		for (int i=0; i<nEigens; i++) {
			// Get the eigenface image.
			IplImage *byteImg = convertFloatImageToUcharImage(eigenVectArr[i]);
			// Paste it into the correct position.
			int x = w * (i % COLUMNS);
			int y = h * (i / COLUMNS);
			CvRect ROI = cvRect(x, y, w, h);
			cvSetImageROI(bigImg, ROI);
			cvCopyImage(byteImg, bigImg);
			cvResetImageROI(bigImg);
			cvReleaseImage(&byteImg);
		}
		cvSaveImage("out_eigenfaces.bmp", bigImg);
		cvReleaseImage(&bigImg);
	}
}

// Save all the eigenvectors as images, so that they can be checked.
static void saveTrainfaceImages(int nTrainFaces, IplImage** faceImgArr)
{
	// Create a large image made of many eigenface images.
	// Must also convert each eigenface image to a normal 8-bit UCHAR image instead of a 32-bit float image.
	printf("Saving the %d reference images as 'out_trainfaces.bmp'\n", nTrainFaces);
	if (nTrainFaces > 0) {
		// Put all the eigenfaces next to each other.
		int COLUMNS = 8;	// Put upto 8 images on a row.
		int nCols = min(nTrainFaces, COLUMNS);
		int nRows = 1 + (nTrainFaces / COLUMNS);	// Put the rest on new rows.
		int w = faceImgArr[0]->width;
		int h = faceImgArr[0]->height;
		CvSize size;
		size = cvSize(nCols * w, nRows * h);
		IplImage *bigImg = cvCreateImage(size, IPL_DEPTH_8U, 1);	// 8-bit Greyscale UCHAR image
		for (int i=0; i<nTrainFaces; i++) {
			// Paste it into the correct position.
			int x = w * (i % COLUMNS);
			int y = h * (i / COLUMNS);
			CvRect ROI = cvRect(x, y, w, h);
			cvSetImageROI(bigImg, ROI);
			cvCopyImage(faceImgArr[i], bigImg);
			cvResetImageROI(bigImg);
		}
		cvSaveImage("out_trainfaces.bmp", bigImg);
		cvReleaseImage(&bigImg);
	}
}

#ifdef BINARY_FILESTORAGE
static CvMat* freadCvMat(FILE * fileStorage)
{
	uchar *pdata;
	int step;
	CvSize size;
	int rows = 0, cols = 0, bytes;
	uchar type = 0;
	CvMat *mat;
	fread(&rows, sizeof(int), 1, fileStorage);
	fread(&cols, sizeof(int), 1, fileStorage);
	fread(&type, sizeof(uchar), 1, fileStorage);
	if (cols == 0 || rows == 0) {
		printf("Read CvMat error\n");
		return NULL;
	}

	if (type == 'i') {
		bytes = 4;
		mat = cvCreateMat( rows, cols, CV_32SC1 );
	} else if (type == 's') {
		bytes = 2;
		mat = cvCreateMat( rows, cols, CV_16SC1 );
	} else if (type == 'u') {
		bytes = 1;
		mat = cvCreateMat( rows, cols, CV_8UC1 );
	} else if (type == 'f') {
		bytes = 4;
		mat = cvCreateMat( rows, cols, CV_32FC1 );
	} else {
		printf("type error\n");
		return NULL;
	}

	cvGetRawData(mat, &pdata, &step, &size);
	fread(pdata, bytes, size.height*size.width, fileStorage);
	return mat;
}

static IplImage* freadCvImage(FILE * fileStorage)
{
	uchar *pdata;
	int step;
	CvSize size;
	int width = 0, height = 0, bytes;
	uchar type = 0;
	IplImage *img;
	fread(&width, sizeof(int), 1, fileStorage);
	fread(&height, sizeof(int), 1, fileStorage);
	fread(&type, sizeof(uchar), 1, fileStorage);
	if (width == 0 || height == 0) {
		printf("Read Image error\n");
		return NULL;
	}
	if (type == 'i') {
		bytes = 4;
		img = cvCreateImage(cvSize(width, height), IPL_DEPTH_32S, 1);  
	} else if (type == 's') {
		bytes = 2;
		img = cvCreateImage(cvSize(width, height), IPL_DEPTH_16S, 1);  
	} else if (type == 'u') {
		bytes = 1;
		img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);  
	} else if (type == 'f') {
		bytes = 4;
		img = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);  
	} else {
		printf("type error\n");
		return NULL;
	}
	cvGetImageRawData(img, &pdata, &step, &size);
	fread(pdata, bytes, size.height*size.width, fileStorage);
	return img;
}

// Open the training data from the file 'traindata.ite'.
static int PCA_load(const char *filename)
{
	FILE * fileStorage;
	int i;

	// create a file-storage interface
	fileStorage = fopen( filename, "rb" );
	if( !fileStorage ) {
		printf("Can't open training database file %s.\n", filename);
		return -1;
	}

	// Load the data
	fread(&pca_recognizer->nTrainFaces, sizeof(int), 1, fileStorage);
	fread(&pca_recognizer->nEigens, sizeof(int), 1, fileStorage);
	//pca_recognizer->eigenValMat = (CvMat *)freadCvMat(fileStorage); // s31
	pca_recognizer->projectMat = (CvMat *)freadCvMat(fileStorage);  // s15.16
	pca_recognizer->avgTrainImg = (IplImage *)freadCvImage(fileStorage);	//8
	pca_recognizer->eigenVectArr = (IplImage **)cvAlloc(pca_recognizer->nEigens*sizeof(IplImage *));
	for(i=0; i<pca_recognizer->nEigens; i++)
		pca_recognizer->eigenVectArr[i] = (IplImage *)freadCvImage(fileStorage);  //s.15

	fclose(fileStorage);
	//printf("Loaded %d eigen faces\n", pca_recognizer->nEigens);
	return 0;
}

static int fwriteCvMat(FILE *fileStorage, CvMat *mat)
{
	uchar *pdata;
	int step;
	CvSize size;
	int rows = 0, cols = 0, bytes;
	uchar c;

	if (CV_MAT_TYPE(mat->type) == CV_32SC1) {
		bytes = 4;
		c = 'i';
	} else if (CV_MAT_TYPE(mat->type) == CV_16SC1) {
		bytes = 2;
		c = 's';
	} else if (CV_MAT_TYPE(mat->type) == CV_8UC1) {
		bytes = 1;
		c = 'u';
	} else if (CV_MAT_TYPE(mat->type) == CV_32FC1) {
		bytes = 4;
		c = 'f';
	} else {
		printf("type error\n");
		return -1;
	}
	fwrite(&mat->rows, sizeof(int), 1, fileStorage);
	fwrite(&mat->cols, sizeof(int), 1, fileStorage);
	fwrite(&c, sizeof(uchar), 1, fileStorage);
	cvGetRawData(mat, &pdata, &step, &size);
	fwrite(pdata, bytes, size.height*size.width, fileStorage);
	return 0;
}

static int fwriteCvImage(FILE *fileStorage, IplImage *img)
{
	uchar *pdata;
	int step;
	CvSize size;
	int width = 0, height = 0, bytes;
	uchar c;

	if (img->depth == IPL_DEPTH_32S) {
		bytes = 4;
		c = 'i';
	} else if (img->depth == IPL_DEPTH_16S) {
		bytes = 2;
		c = 's';
	} else if (img->depth == IPL_DEPTH_8U) {
		bytes = 1;
		c = 'u';
	} else if (img->depth == IPL_DEPTH_32F) {
		bytes = 4;
		c = 'f';  
	} else {
		printf("type error\n");
		return -1;
	}

	fwrite(&img->width, sizeof(int), 1, fileStorage);
	fwrite(&img->height, sizeof(int), 1, fileStorage);
	fwrite(&c, sizeof(uchar), 1, fileStorage);
	cvGetImageRawData(img, &pdata, &step, &size);
	fwrite(pdata, bytes, size.height*size.width, fileStorage);
	return 0;
}

// Save the training data to the file 'facedata.xml'.
int PCA_save(const char *filename)
{
	FILE *fileStorage;
	int i;

	// create a file-storage interface
	fileStorage = fopen(filename, "wb" );
	if( !fileStorage ) {
		printf("Can't open training database file %s.\n", filename);
		return -1;
	}

	fwrite(&pca_recognizer->nTrainFaces, sizeof(int), 1, fileStorage);
	fwrite(&pca_recognizer->nEigens, sizeof(int), 1, fileStorage);
	//fwriteCvMat(fileStorage, pca_recognizer->eigenValMat);
	fwriteCvMat(fileStorage, pca_recognizer->projectMat);
	fwriteCvImage(fileStorage, pca_recognizer->avgTrainImg);
	for(i=0; i<pca_recognizer->nEigens; i++)
		fwriteCvImage(fileStorage, pca_recognizer->eigenVectArr[i]);

	fclose(fileStorage);
	//printf("Store %d training images\n", pca_recognizer->nTrainFaces);

	return 0;
}

#else
// Open the training data from the file 'facedata.xml'.
static int PCA_load(const char *filename)
{
	CvFileStorage * fileStorage;
	int i;

	// create a file-storage interface
	fileStorage = cvOpenFileStorage( filename, 0, CV_STORAGE_READ );
	if( !fileStorage ) {
		printf("Can't open training database file %s.\n", filename);
		return -1;
	}

	// Load the data
	pca_recognizer->nTrainFaces = cvReadIntByName(fileStorage, 0, "nTrainFaces", 0);
	pca_recognizer->nEigens = cvReadIntByName(fileStorage, 0, "nEigens", 0);
	//pca_recognizer->eigenValMat  = (CvMat *)cvReadByName(fileStorage, 0, "eigenValMat", 0);
	pca_recognizer->projectMat = (CvMat *)cvReadByName(fileStorage, 0, "projectMat", 0);
	pca_recognizer->avgTrainImg = (IplImage *)cvReadByName(fileStorage, 0, "avgTrainImg", 0);
	pca_recognizer->eigenVectArr = (IplImage **)cvAlloc(pca_recognizer->nEigens*sizeof(IplImage *));
	for(i=0; i<pca_recognizer->nEigens; i++)
	{
		char varname[200];
		snprintf( varname, sizeof(varname)-1, "eigenVect_%d", i );
		pca_recognizer->eigenVectArr[i] = (IplImage *)cvReadByName(fileStorage, 0, varname, 0);
	}

	// release the file-storage interface
	cvReleaseFileStorage( &fileStorage );
	//printf("Training data loaded (%d eigenfaces):\n", pca_recognizer->nEigens);
	return 0;
}

// Save the training data to the file 'facedata.xml'.
static int PCA_save(const char *filename)
{
	CvFileStorage * fileStorage;
	int i;

	// create a file-storage interface
	fileStorage = cvOpenFileStorage( filename, 0, CV_STORAGE_WRITE );

	// store all the data
	cvWriteInt( fileStorage, "nTrainFaces", pca_recognizer->nTrainFaces );
	cvWriteInt( fileStorage, "nEigens", pca_recognizer->nEigens );
	//cvWrite(fileStorage, "eigenValMat", pca_recognizer->eigenValMat, cvAttrList(0,0));
	cvWrite(fileStorage, "projectMat", pca_recognizer->projectMat, cvAttrList(0,0));
	cvWrite(fileStorage, "avgTrainImg", pca_recognizer->avgTrainImg, cvAttrList(0,0));
	for(i=0; i<pca_recognizer->nEigens; i++)
	{
		char varname[200];
		snprintf( varname, sizeof(varname)-1, "eigenVect_%d", i );
		cvWrite(fileStorage, varname, pca_recognizer->eigenVectArr[i], cvAttrList(0,0));
	}

	// release the file-storage interface
	cvReleaseFileStorage( &fileStorage );
	return 0;
}

#endif

// Find the most likely person based on a detection. Returns the index, and stores the confidence value into pConfidence.
static int findNearestNeighbor(int * projectedTestFace)
{
	//double leastDistSq = 1e12;
	int i, iTrain, iNearest = -1;
	int threshold_high = PCA_RECOGNIZED_HIGH * pca_recognizer->nEigens;
	int threshold_low = PCA_RECOGNIZED_LOW * pca_recognizer->nEigens;
	int leastDistSq = threshold_high;

	for(iTrain=0; iTrain<pca_recognizer->nTrainFaces; iTrain++)
	{
		int distSq=0;

		for(i=0; i<pca_recognizer->nEigens; i++)
		{
			int d_i = projectedTestFace[i] - pca_recognizer->projectMat->data.i[iTrain*pca_recognizer->nEigens + i];
#ifdef USE_MAHALANOBIS_DISTANCE
			distSq += d_i*d_i;// / eigenValMat->data.fl[i];  // Mahalanobis distance (might give better results than Eucalidean distance)
#else
			distSq += d_i*d_i; // Euclidean distance.
#endif
			if (distSq > threshold_high)
				break;
		}

		if (distSq < threshold_high)
			printf("<%d> %d\n", iTrain, distSq / pca_recognizer->nEigens);

		if(distSq < leastDistSq)
		{
			leastDistSq = distSq;
			iNearest = iTrain;
		}
		if(leastDistSq < threshold_low)
			break;
	}

	// Return the found index.
	return iNearest;
}

// Do the Principal Component Analysis, finding the average image
// and the eigenfaces that represent any image in the given dataset.
static void doPCA(IplImage** faceImgArr, int numComponents)
{
	int i;
	CvTermCriteria calcLimit;
	CvSize faceImgSize;

	pca_recognizer->nEigens = numComponents;

	// allocate the eigenvector images
	faceImgSize.width  = faceImgArr[0]->width;
	faceImgSize.height = faceImgArr[0]->height;
	pca_recognizer->eigenVectArr = (IplImage**)cvAlloc(sizeof(IplImage*) * pca_recognizer->nEigens);
	for(i=0; i<pca_recognizer->nEigens; i++)
		pca_recognizer->eigenVectArr[i] = cvCreateImage(faceImgSize, IPL_DEPTH_16S, 1);   //s.15

	// allocate the eigenvalue array
	//pca_recognizer->eigenValMat = cvCreateMat( 1, pca_recognizer->nTrainfaces, CV_32SC1 );

	// allocate the averaged image
	pca_recognizer->avgTrainImg = cvCreateImage(faceImgSize, IPL_DEPTH_8U, 1);  

	// set the PCA termination criterion
	calcLimit = cvTermCriteria( CV_TERMCRIT_ITER, pca_recognizer->nEigens, 1);

	// compute average image, eigenvalues, and eigenvectors
	cvCalcEigenObjects(
		pca_recognizer->nTrainFaces,
		(void*)faceImgArr,
		(void*)pca_recognizer->eigenVectArr,
		CV_EIGOBJ_NO_CALLBACK,
		0,
		0,
		&calcLimit,
		pca_recognizer->avgTrainImg,
		NULL);//pca_recognizer->eigenValMat->data.i);

	//cvNormalize(pca_recognizer->eigenValMat, pca_recognizer->eigenValMat, 1, 0, CV_L1, 0);

}

static void doProject(IplImage** faceImgArr)
{
	int i, offset;
	// project the training images onto the PCA subspace
	pca_recognizer->projectMat = cvCreateMat( pca_recognizer->nTrainFaces, pca_recognizer->nEigens, CV_32SC1 ); //s31
	offset = pca_recognizer->projectMat->step / sizeof(float);
	for(i=0; i<pca_recognizer->nTrainFaces; i++)
	{
		//int offset = i * nEigens;
		cvEigenDecomposite(
			faceImgArr[i],
			pca_recognizer->nEigens,
			pca_recognizer->eigenVectArr,
			0, 0,
			pca_recognizer->avgTrainImg,
			pca_recognizer->projectMat->data.i + i*offset);
	}

}

static void PCA_create(void)
{
	pca_recognizer = (PCA *)malloc(sizeof(PCA));
	memset(pca_recognizer, 0, sizeof(PCA));
	pca_recognizer->faceId = -1;
}

static void PCA_release(void)
{
	int i;
	// Release previous train data
	for(i=0; i<pca_recognizer->nEigens; i++)
		if (pca_recognizer->eigenVectArr && pca_recognizer->eigenVectArr[i])
			cvReleaseImage(&pca_recognizer->eigenVectArr[i]);
	if (pca_recognizer->eigenVectArr)
		cvFree(pca_recognizer->eigenVectArr);

	if (pca_recognizer->avgTrainImg)
			cvReleaseImage(&pca_recognizer->avgTrainImg);

//	if (pca_recognizer->eigenValMat)
//		cvReleaseMat(&pca_recognizer->eigenValMat);
//	pca_recognizer->eigenValMat = 0;

	if (pca_recognizer->projectMat)
		cvReleaseMat(&pca_recognizer->projectMat);
	pca_recognizer->projectMat = 0;

	free(pca_recognizer);
}

// Train from the data in the given text file, and store the trained data into the file 'facedata.xml'.
static int PCA_train(int nTrainFaces, IplImage** faceImgArr) 
{
	int numComponents = 0;

	if (nTrainFaces<2) {
		printf( "Need 2 or more training faces\n"
		        "Input file contains only %d\n", nTrainFaces);
		return -1;
	} 
	// do PCA on the training faces
	numComponents = (nTrainFaces-1<PCA_MAX_COMPONENTS) ? nTrainFaces-1 : PCA_MAX_COMPONENTS;
	pca_recognizer->nTrainFaces = nTrainFaces;
	doPCA(faceImgArr, numComponents);
	doProject(faceImgArr);

	// Save all the eigenvectors as images, so that they can be checked.
	if (SAVE_EIGENFACE_IMAGES) {
		saveEigenfaceImages(pca_recognizer->nEigens, pca_recognizer->eigenVectArr, pca_recognizer->avgTrainImg);
	}
	if (SAVE_TRAINFACE_IMAGES) {
		saveTrainfaceImages(nTrainFaces, faceImgArr);
	}
	return 0;
}

// Recognize the face in each of the test images given, and compare the results with the truth.
static int PCA_predict(IplImage* faceImg)
{
	CvMat * trainPersonNumMat = 0;  // the person numbers during training
	int * projectedTestFace = 0;
	int nCorrect = 0;
	int nWrong = 0;
	int iNearest = -1;
	int ret = 0;
	CvRect	faceRect = {0};

	// project the test images onto the PCA subspace
	projectedTestFace = (int *)cvAlloc( pca_recognizer->nEigens*sizeof(int) );

	// project the test image onto the PCA subspace
	cvEigenDecomposite(
			faceImg,
			pca_recognizer->nEigens,
			pca_recognizer->eigenVectArr,
			0, 0,
			pca_recognizer->avgTrainImg,
			projectedTestFace);

	iNearest = findNearestNeighbor(projectedTestFace);
	pca_recognizer->faceId = iNearest;

	{
	IplImage *projectImg = 0;
	projectImg = cvCreateImage(cvSize(pca_recognizer->avgTrainImg->width, pca_recognizer->avgTrainImg->height), IPL_DEPTH_8U, 1);
	cvEigenProjection( 
				pca_recognizer->eigenVectArr,
				pca_recognizer->nEigens,
                0, 0,
                projectedTestFace, 
                pca_recognizer->avgTrainImg,
				projectImg);

    cvShowImage( "project", projectImg );
	cvReleaseImage(&projectImg);
	}

	return iNearest;
}

static int PCA_status(void)
{
	return pca_recognizer->faceId;
}

FaceRecognizer PCARecognizer =
{
    "pca",
	PCA_create,
	PCA_release,
	PCA_load,
	PCA_save,
	PCA_train,
	PCA_predict,
	PCA_status
};