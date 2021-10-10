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

#define BINARY_FILESTORAGE
#define PERSON_FACE_NUM		3

using namespace std;

#define SAVE_EIGENFACE_IMAGES	1		// Set to 0 if you dont want images of the Eigenvectors saved to files (for debugging).
#define SAVE_TRAINFACE_IMAGES	1		// Set to 0 if you dont want images of the trained saved to files (for debugging).
//#define USE_MAHALANOBIS_DISTANCE	// You might get better recognition accuracy if you enable this.

typedef struct _LDA {
	int					nTrainFaces;			// the number of training images
	int					faceId;
	IplImage		*avgTrainImg;			// the average image // U8 8 format
	int					nEigens;				// the number of eigenvalues
	IplImage		**eigenVectArr;			// eigenvectors
	CvMat			*ldaEigenVect;			  // ldaEigenvectors
	CvMat			*ldaEigenValMat;			// ldaEigenvalues
	CvMat			*projectMat;
	//CvMat			*eigenValMat;
} LDA;

static LDA *lda_recognizer = 0;

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
static int LDA_load(const char *filename)
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
	fread(&lda_recognizer->nTrainFaces, sizeof(int), 1, fileStorage);
	fread(&lda_recognizer->nEigens, sizeof(int), 1, fileStorage);
	//lda_recognizer->eigenValMat = (CvMat *)freadCvMat(fileStorage); // s31
	lda_recognizer->projectMat = (CvMat *)freadCvMat(fileStorage);  // s15.16
	lda_recognizer->avgTrainImg = (IplImage *)freadCvImage(fileStorage);	//8
	lda_recognizer->eigenVectArr = (IplImage **)cvAlloc(lda_recognizer->nEigens*sizeof(IplImage *));
	for(i=0; i<lda_recognizer->nEigens; i++)
		lda_recognizer->eigenVectArr[i] = (IplImage *)freadCvImage(fileStorage);  //s.15

	fclose(fileStorage);
	//printf("Loaded %d eigen faces\n", lda_recognizer->nEigens);
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
int LDA_save(const char *filename)
{
	FILE *fileStorage;
	int i;

	// create a file-storage interface
	fileStorage = fopen(filename, "wb" );
	if( !fileStorage ) {
		printf("Can't open training database file %s.\n", filename);
		return -1;
	}

	fwrite(&lda_recognizer->nTrainFaces, sizeof(int), 1, fileStorage);
	fwrite(&lda_recognizer->nEigens, sizeof(int), 1, fileStorage);
	//fwriteCvMat(fileStorage, lda_recognizer->eigenValMat);
	fwriteCvMat(fileStorage, lda_recognizer->projectMat);
	fwriteCvImage(fileStorage, lda_recognizer->avgTrainImg);
	for(i=0; i<lda_recognizer->nEigens; i++)
		fwriteCvImage(fileStorage, lda_recognizer->eigenVectArr[i]);

	fclose(fileStorage);
	//printf("Store %d training images\n", lda_recognizer->nTrainFaces);

	return 0;
}

#else
// Open the training data from the file 'facedata.xml'.
static int LDA_load(const char *filename)
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
	lda_recognizer->nTrainFaces = cvReadIntByName(fileStorage, 0, "nTrainFaces", 0);
	lda_recognizer->nEigens = cvReadIntByName(fileStorage, 0, "nEigens", 0);
	//lda_recognizer->eigenValMat  = (CvMat *)cvReadByName(fileStorage, 0, "eigenValMat", 0);
	lda_recognizer->projectMat = (CvMat *)cvReadByName(fileStorage, 0, "projectMat", 0);
	lda_recognizer->avgTrainImg = (IplImage *)cvReadByName(fileStorage, 0, "avgTrainImg", 0);
	lda_recognizer->eigenVectArr = (IplImage **)cvAlloc(lda_recognizer->nEigens*sizeof(IplImage *));
	for(i=0; i<lda_recognizer->nEigens; i++)
	{
		char varname[200];
		snprintf( varname, sizeof(varname)-1, "eigenVect_%d", i );
		lda_recognizer->eigenVectArr[i] = (IplImage *)cvReadByName(fileStorage, 0, varname, 0);
	}

	// release the file-storage interface
	cvReleaseFileStorage( &fileStorage );
	//printf("Training data loaded (%d eigenfaces):\n", lda_recognizer->nEigens);
	return 0;
}

// Save the training data to the file 'facedata.xml'.
static int LDA_save(const char *filename)
{
	CvFileStorage * fileStorage;
	int i;

	// create a file-storage interface
	fileStorage = cvOpenFileStorage( filename, 0, CV_STORAGE_WRITE );

	// store all the data
	cvWriteInt( fileStorage, "nTrainFaces", lda_recognizer->nTrainFaces );
	cvWriteInt( fileStorage, "nEigens", lda_recognizer->nEigens );
	//cvWrite(fileStorage, "eigenValMat", lda_recognizer->eigenValMat, cvAttrList(0,0));
	cvWrite(fileStorage, "projectMat", lda_recognizer->projectMat, cvAttrList(0,0));
	cvWrite(fileStorage, "avgTrainImg", lda_recognizer->avgTrainImg, cvAttrList(0,0));
	for(i=0; i<lda_recognizer->nEigens; i++)
	{
		char varname[200];
		snprintf( varname, sizeof(varname)-1, "eigenVect_%d", i );
		cvWrite(fileStorage, varname, lda_recognizer->eigenVectArr[i], cvAttrList(0,0));
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
	int threshold_high = LDA_RECOGNIZED_HIGH * lda_recognizer->nEigens;
	int threshold_low = LDA_RECOGNIZED_LOW * lda_recognizer->nEigens;
	int leastDistSq = threshold_high;

	for(iTrain=0; iTrain<lda_recognizer->nTrainFaces; iTrain++)
	{
		int distSq=0;

		for(i=0; i<lda_recognizer->nEigens; i++)
		{
			int d_i = projectedTestFace[i] - lda_recognizer->projectMat->data.i[iTrain*lda_recognizer->nEigens + i];
#ifdef USE_MAHALANOBIS_DISTANCE
			distSq += d_i*d_i;// / eigenValMat->data.fl[i];  // Mahalanobis distance (might give better results than Eucalidean distance)
#else
			distSq += d_i*d_i; // Euclidean distance.
#endif
			if (distSq > threshold_high)
				break;
		}

		//if (distSq < threshold_high)
			printf("<%d> %d\n", iTrain, distSq / lda_recognizer->nEigens);

		if(distSq < leastDistSq)
		{
			leastDistSq = distSq;
			iNearest = iTrain;
		}
		//if(leastDistSq < threshold_low)
		//	break;
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

	lda_recognizer->nEigens = numComponents;

	// allocate the eigenvector images
	faceImgSize.width  = faceImgArr[0]->width;
	faceImgSize.height = faceImgArr[0]->height;
	lda_recognizer->eigenVectArr = (IplImage**)cvAlloc(sizeof(IplImage*) * lda_recognizer->nEigens);
	for(i=0; i<lda_recognizer->nEigens; i++)
		lda_recognizer->eigenVectArr[i] = cvCreateImage(faceImgSize, IPL_DEPTH_16S, 1);   //s.15

	// allocate the eigenvalue array
	//lda_recognizer->eigenValMat = cvCreateMat( 1, lda_recognizer->nTrainfaces, CV_32SC1 );

	// allocate the averaged image
	lda_recognizer->avgTrainImg = cvCreateImage(faceImgSize, IPL_DEPTH_8U, 1);  

	// set the PCA termination criterion
	calcLimit = cvTermCriteria( CV_TERMCRIT_ITER, lda_recognizer->nEigens, 1);

	// compute average image, eigenvalues, and eigenvectors
	cvCalcEigenObjects(
		lda_recognizer->nTrainFaces,
		(void*)faceImgArr,
		(void*)lda_recognizer->eigenVectArr,
		CV_EIGOBJ_NO_CALLBACK,
		0,
		0,
		&calcLimit,
		lda_recognizer->avgTrainImg,
		NULL);//lda_recognizer->eigenValMat->data.i);

	//cvNormalize(lda_recognizer->eigenValMat, lda_recognizer->eigenValMat, 1, 0, CV_L1, 0);

}

static void doProject(IplImage** faceImgArr)
{
	int i, offset;
	// project the training images onto the PCA subspace
	lda_recognizer->projectMat = cvCreateMat( lda_recognizer->nTrainFaces, lda_recognizer->nEigens, CV_32SC1 ); //s31
	offset = lda_recognizer->projectMat->step / sizeof(float);
	for(i=0; i<lda_recognizer->nTrainFaces; i++)
	{
		//int offset = i * nEigens;
		cvEigenDecomposite(
			faceImgArr[i],
			lda_recognizer->nEigens,
			lda_recognizer->eigenVectArr,
			0, 0,
			lda_recognizer->avgTrainImg,
			lda_recognizer->projectMat->data.i + i*offset);
	}

}

static void doLDA(void)
{
	int i;
	CvMat *src = lda_recognizer->projectMat;
	int N = src->rows;
	int D = src->cols;
	int C = N - D;
    CvMat *meanTotal;
	CvMat **meanClass;
	CvMat *Sw, *Sb, *Swi, *M;
	CvMat *evector, *eval;
	CvMat row;

	// calculate total mean and class mean
	meanTotal = cvCreateMat( 1, D, CV_32SC1 );
	cvSetZero(meanTotal);
	meanClass = (CvMat **)cvAlloc(C * sizeof (CvMat *));
	for (i=0;i<C;i++) {
		meanClass[i] = cvCreateMat( 1, D, CV_32SC1 );
		cvSetZero(meanClass[i]);
	}

    // calculate sums
    for (i = 0; i < N; i++) {
        int classIdx = i/PERSON_FACE_NUM;
		cvGetRow(src, &row, i);
        cvAdd(meanTotal, &row, meanTotal);
        cvAdd(meanClass[classIdx], &row, meanClass[classIdx]);
    }
	
    // calculate total mean
	cvConvertScale(meanTotal, meanTotal, 1.0/N, 0);
    // calculate class means
    for (i = 0; i < C; i++) 
        cvConvertScale(meanClass[i], meanClass[i], 1.0 / PERSON_FACE_NUM, 0);

    // subtract class means
    for (i = 0; i < N; i++) {
        int classIdx = i/PERSON_FACE_NUM;
		cvGetRow(src, &row, i);
        cvSub(&row, meanClass[classIdx], &row);
    }

    // calculate within-classes scatter
    Sw = cvCreateMat( D, D, CV_32FC1 );
	cvSetZero(Sw);	
    cvMulTransposed(src, Sw, 1);

    // calculate between-classes scatter
    Sb = cvCreateMat( D, D, CV_32FC1 );
	cvSetZero(Sb);	
    
	CvMat *tmp = cvCreateMat( D, D, CV_32FC1 );
    for (i = 0; i < C; i++) {
        cvSub(meanClass[i], meanTotal, meanClass[i]);
        cvMulTransposed(meanClass[i], tmp, 1);
        cvAdd(Sb, tmp, Sb);
    }
	cvReleaseMat(&tmp);

    // invert Sw
    Swi = cvCreateMat( D, D, CV_32FC1 );
    cvInvert(Sw, Swi);

    // M = inv(Sw)*Sb
    M = cvCreateMat( D, D, CV_32FC1 );
	cvGEMM(Swi, Sb, 1.0, NULL, 0.0, M);

	evector = cvCreateMat( D, D, CV_32FC1 );
	eval = cvCreateMat( 1, D, CV_32FC1 );

	cvEigenVV(M, evector, eval, 0.01);
/*
	printf("Swi=\n");
	for(i=0;i<D;i++) {
		for(j=0;j<D;j++) 
			printf("%f, ", Swi->data.fl[i*M->cols+j]);
		printf("\n");
	}
	printf("Sw=\n");
	for(i=0;i<D;i++) {
		for(j=0;j<D;j++) 
			printf("%.1f, ", Sw->data.fl[i*M->cols+j]);
		printf("\n");
	}
	printf("Sb=\n");
	for(i=0;i<D;i++) {
		for(j=0;j<D;j++) 
			printf("%.1f, ", Sb->data.fl[i*M->cols+j]);
		printf("\n");
	}
	printf("M=\n");
	for(i=0;i<D;i++) {
		for(j=0;j<D;j++) 
			printf("%f, ", M->data.fl[i*M->cols+j]);
		printf("\n");
	}
	printf("lda eigen vector=\n");
	for(i=0;i<D;i++) {
		for(j=0;j<D;j++) 
			printf("%.2f, ", evector->data.fl[i*M->cols+j]);
		printf("\n");
	}
	printf("lda eigen value=\n");
	for(i=0;i<D;i++)
		printf("%.2f, ", eval->data.fl[i]);
	printf("\n");
*/
	cvReleaseMat(&M);
	cvReleaseMat(&Swi);
	cvReleaseMat(&Sb);
	cvReleaseMat(&Sw);
	for (i=0;i<C;i++)
		cvReleaseMat(&meanClass[i]);
	cvFree(meanClass);
	cvReleaseMat(&meanTotal);

	lda_recognizer->ldaEigenVect = evector;
	lda_recognizer->ldaEigenValMat = eval;

}

void imgGEMM(CvMat *mat, int nSrcEigens, IplImage **srcImgArr, int nDstEigens, IplImage **dstImgArr)
{
	int c,i,j,k;
	short *D, *S;
	float *M;
	int width = srcImgArr[0]->width;
	int height = srcImgArr[0]->height;
	printf("mat rows = %d, cols = %d\n", mat->rows, mat->cols);
	if (mat->cols != nSrcEigens)
		printf("mat cols size mismatch with image array number\n");
	if (mat->rows < nDstEigens)
		printf("mat rows small than dst eigen number\n");

	M = mat->data.fl;
/*
	for (c=0;c<nDstEigens;c++) {
		for (i=0;i<height;i++) {
			for (j=0;j<width;j++) {
				D = (float *)dstImgArr[c]->imageData;
				D[i*width+j] = 0.;
				for (k=0;k<nSrcEigens;k++) {
					S = (float *)srcImgArr[k]->imageData;
					D[i*width+j] += M[c*mat->cols+k]*S[i*width+j];
				}
			}
		}
	}
*/
	for (c=0;c<nDstEigens;c++,M+=nSrcEigens) {
		D = (short *)dstImgArr[c]->imageData;
		for (i=0;i<height;i++) {
			for (j=0;j<width;j++) {
				int offset = i*width+j;
				D[offset] = 0;
				for (k=0;k<nSrcEigens;k++) {
					S = (short *)srcImgArr[k]->imageData;
					D[offset] += M[k]*S[offset];
				}
			}
		}
	}
}

static void LDA_create(void)
{
	lda_recognizer = (LDA *)malloc(sizeof(LDA));
	memset(lda_recognizer, 0, sizeof(LDA));
	lda_recognizer->faceId = -1;
}

static void LDA_release(void)
{
	int i;
	// Release previous train data
	for(i=0; i<lda_recognizer->nEigens; i++)
		if (lda_recognizer->eigenVectArr && lda_recognizer->eigenVectArr[i])
			cvReleaseImage(&lda_recognizer->eigenVectArr[i]);
	if (lda_recognizer->eigenVectArr)
		cvFree(lda_recognizer->eigenVectArr);

	if (lda_recognizer->ldaEigenVect)
		cvFree(&lda_recognizer->ldaEigenVect);
	if (lda_recognizer->ldaEigenValMat)
		cvReleaseMat(&lda_recognizer->ldaEigenValMat);

	if (lda_recognizer->avgTrainImg)
			cvReleaseImage(&lda_recognizer->avgTrainImg);

//	if (lda_recognizer->eigenValMat)
//		cvReleaseMat(&lda_recognizer->eigenValMat);
//	lda_recognizer->eigenValMat = 0;

	if (lda_recognizer->projectMat)
		cvReleaseMat(&lda_recognizer->projectMat);
	lda_recognizer->projectMat = 0;

	free(lda_recognizer);
}

// Train from the data in the given text file, and store the trained data into the file 'facedata.xml'.
static int LDA_train(int nTrainFaces, IplImage** faceImgArr) 
{
	int i;
	int nClasses;
	int numComponents = 0;
	CvSize fs;
	IplImage** tmpImgArr;

	if (nTrainFaces<2) {
		printf( "ERROR: Need 2 or more training faces\n"
		        "Input file contains only %d\n", nTrainFaces);
		return -1;
	}

	nClasses = nTrainFaces / PERSON_FACE_NUM;
	if (nClasses*PERSON_FACE_NUM != nTrainFaces) {
		printf("ERROR: Training faces number mismatch, need persion*%d, but face number is %d\n", 
					 PERSON_FACE_NUM, lda_recognizer->nTrainFaces);
		return -1;
	}

	lda_recognizer->nTrainFaces = nTrainFaces;

	if (nClasses==1) {
		doPCA(faceImgArr, lda_recognizer->nTrainFaces-1);
		doProject(faceImgArr);
	} else {
		doPCA(faceImgArr, lda_recognizer->nTrainFaces - nClasses);
		doProject(faceImgArr);
		numComponents = (nClasses-1<LDA_MAX_COMPONENTS) ? nClasses-1 : LDA_MAX_COMPONENTS;
		doLDA();

		tmpImgArr = (IplImage**)cvAlloc(sizeof(IplImage*) * numComponents);
		fs.width = faceImgArr[0]->width;
		fs.height = faceImgArr[0]->height;
		for(i=0; i<numComponents; i++)
			tmpImgArr[i] = cvCreateImage(fs, IPL_DEPTH_16S, 1);   //s.15

		imgGEMM(lda_recognizer->ldaEigenVect, 
				lda_recognizer->nEigens,
				lda_recognizer->eigenVectArr, 
				numComponents,
				tmpImgArr);

		for(i=0; i<lda_recognizer->nEigens; i++)
			if (lda_recognizer->eigenVectArr && lda_recognizer->eigenVectArr[i])
				cvReleaseImage(&lda_recognizer->eigenVectArr[i]);
		if (lda_recognizer->eigenVectArr)
			cvFree(lda_recognizer->eigenVectArr);
		if (lda_recognizer->projectMat)
			cvReleaseMat(&lda_recognizer->projectMat);
		lda_recognizer->projectMat = 0;

		if (lda_recognizer->ldaEigenVect)
			cvFree(&lda_recognizer->ldaEigenVect);
		lda_recognizer->ldaEigenVect = 0;
		if (lda_recognizer->ldaEigenValMat)
			cvFree(&lda_recognizer->ldaEigenValMat);
		lda_recognizer->ldaEigenValMat = 0;

		lda_recognizer->nEigens = numComponents;
		lda_recognizer->eigenVectArr = tmpImgArr;

		doProject(faceImgArr);
	}

	// Save all the eigenvectors as images, so that they can be checked.
	if (SAVE_EIGENFACE_IMAGES) {
		saveEigenfaceImages(lda_recognizer->nEigens, lda_recognizer->eigenVectArr, lda_recognizer->avgTrainImg);
	}
	if (SAVE_TRAINFACE_IMAGES) {
		saveTrainfaceImages(nTrainFaces, faceImgArr);
	}
	return 0;
}

// Recognize the face in each of the test images given, and compare the results with the truth.
static int LDA_predict(IplImage* faceImg)
{
	CvMat * trainPersonNumMat = 0;  // the person numbers during training
	int * projectedTestFace = 0;
	int nCorrect = 0;
	int nWrong = 0;
	int iNearest = -1;
	int ret = 0;
	CvRect	faceRect = {0};

#ifdef WEBER_FILTER_ENABLE
	IplImage* weberImg = 0;
	weberImg = cvCreateImage (cvSize(faceImg->width, faceImg->height), IPL_DEPTH_8U, 1);
	weber_filter(faceImg, weberImg);
    cvShowImage( "project", weberImg );
#endif

	// project the test images onto the PCA subspace
	projectedTestFace = (int *)cvAlloc( lda_recognizer->nEigens*sizeof(int) );

	// project the test image onto the PCA subspace
	cvEigenDecomposite(
#ifdef WEBER_FILTER_ENABLE
			weberImg,
#else
			faceImg,
#endif
			lda_recognizer->nEigens,
			lda_recognizer->eigenVectArr,
			0, 0,
			lda_recognizer->avgTrainImg,
			projectedTestFace);

	iNearest = findNearestNeighbor(projectedTestFace);
	lda_recognizer->faceId = iNearest;

	{
	IplImage *projectImg = 0;
	projectImg = cvCreateImage(cvSize(lda_recognizer->avgTrainImg->width, lda_recognizer->avgTrainImg->height), IPL_DEPTH_8U, 1);
	cvEigenProjection( 
				lda_recognizer->eigenVectArr,
				lda_recognizer->nEigens,
                0, 0,
                projectedTestFace, 
                lda_recognizer->avgTrainImg,
				projectImg);

    cvShowImage( "project", projectImg );
	cvReleaseImage(&projectImg);
	}
#ifdef WEBER_FILTER_ENABLE
	cvReleaseImage(&weberImg);
#endif

	return iNearest;
}


static int LDA_status(void)
{
	return lda_recognizer->faceId;
}

FaceRecognizer LDARecognizer =
{
    "lda",
	LDA_create,
	LDA_release,
	LDA_load,
	LDA_save,
	LDA_train,
	LDA_predict,
	LDA_status
};