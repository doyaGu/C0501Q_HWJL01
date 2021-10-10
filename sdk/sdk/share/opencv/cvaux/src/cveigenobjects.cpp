/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "_cvaux.h"

/* Integer square root by Halleck's method, with Legalize's speedup */
static int isqrt (int64 x)
{
  int64   squaredbit, remainder, root;
   if (x<1) return 0;
   /* Load the binary constant 01 00 00 ... 00, where the number
    * of zero bits to the right of the single one bit
    * is even, and the one bit is as far left as is consistant
    * with that condition.)
    */
   squaredbit  = (int64) ((((uint64) ~0L) >> 1) & 
                        ~(((uint64) ~0L) >> 2));
   /* This portable load replaces the loop that used to be 
    * here, and was donated by  legalize@xmission.com 
    */

   /* Form bits of the answer. */
   remainder = x;  root = 0;
   while (squaredbit > 0) {
     if (remainder >= (squaredbit | root)) {
         remainder -= (squaredbit | root);
         root >>= 1; root |= squaredbit;
     } else {
         root >>= 1;
     }
     squaredbit >>= 2; 
   }

   return (int)root;
}

CvStatus CV_STDCALL
icvJacobiEigens_32f(int *A, int *V, int *E, int n, int eps)
{
    int i, j, k, ind;
    int *AA = A;
	int *VV = V;  // s16.15
    int Amax = 0, anorm = 0, ax;

    if( A == NULL || V == NULL || E == NULL )
        return CV_NULLPTR_ERR;
    if( n <= 0 )
        return CV_BADSIZE_ERR;

    /*-------- Prepare --------*/
    for( i = 0; i < n; i++, VV += n, AA += n )
    {
        //for( j = 0; j < i; j++ )
       // {
		//	if (Amax<abs(AA[j]))
		//		Amax = abs(AA[j]);
        //    double Am = AA[j];
        //    anorm += Am * Am;
        //}
        for( j = 0; j < n; j++ )
            VV[j] = 0;
        VV[i] = 32768;
    }

    //anorm = sqrt( anorm + anorm );
	ax = eps<50 ? 50 : eps;
    //Amax = anorm;

   // while( Amax > ax )
    {
        //Amax /= n;
        do                      /* while (ind) */
        {
            int p, q;
			int *A1 = A;
            int *V1 = V; 

            ind = 0;
            for( p = 0; p < n - 1; p++, A1 += n, V1 += n )
            {
                int *A2 = A + n * (p + 1);
				int *V2 = V + n * (p + 1);

                for( q = p + 1; q < n; q++, A2 += n, V2 += n )
                {
                    int x, y, c, s, c2, s2, a;

                    int *A3, Apq = A1[q], App, Aqq, Aip, Aiq;
					int Vpi, Vqi;

                    if( abs( Apq ) < ax )
                        continue;

                    ind = 1;

                    /*---- Calculation of rotation angle's sine & cosine ----*/
                    App = A1[p];
                    Aqq = A2[q];
                    y = (App - Aqq)>>1;
                    x = -(int)((int64)Apq * 32768 / isqrt( (int64)Apq * Apq + (int64)y * y ));
                    if( y < 0 )
                        x = -x;
					s = isqrt( ((int64)32768 + (int64)isqrt((int64)32768*32768 - (x * x)))<<(15+1));
                    s = (int)((int64)x * 32768 / s);
                    s2 = (s * s)>>15;
                    c = isqrt( ((int64)32768 - s2)<<15);
                    c2 = (c * c)>>15;
                    a = (int)(((int64)Apq * ((c * s) >>15)) >> 15) * 2;

                    /*---- Apq annulation ----*/
                    A3 = A;
                    for( i = 0; i < p; i++, A3 += n )
                    {
                        Aip = A3[p];
                        Aiq = A3[q];
                        Vpi = V1[i];
                        Vqi = V2[i];
                        A3[p] = (int) (((int64)Aip * c - (int64)Aiq * s)>>15);
                        A3[q] = (int) (((int64)Aiq * c + (int64)Aip * s)>>15);
                        V1[i] = (int) ((Vpi * c - Vqi * s)>>15);
                        V2[i] = (int) ((Vqi * c + Vpi * s)>>15);
                    }
                    for( ; i < q; i++, A3 += n )
                    {
                        Aip = A1[i];
                        Aiq = A3[q];
                        Vpi = V1[i];
                        Vqi = V2[i];
                        A1[i] = (int) (((int64)Aip * c - (int64)Aiq * s)>>15);
                        A3[q] = (int) (((int64)Aiq * c + (int64)Aip * s)>>15);
                        V1[i] = (int) ((Vpi * c - Vqi * s)>>15);
                        V2[i] = (int) ((Vqi * c + Vpi * s)>>15);
                    }
                    for( ; i < n; i++ )
                    {
                        Aip = A1[i];
                        Aiq = A2[i];
                        Vpi = V1[i];
                        Vqi = V2[i];
                        A1[i] = (int) (((int64)Aip * c - (int64)Aiq * s)>>15);
                        A2[i] = (int) (((int64)Aiq * c + (int64)Aip * s)>>15);
                        V1[i] = (int) ((Vpi * c - Vqi * s)>>15);
                        V2[i] = (int) ((Vqi * c + Vpi * s)>>15);
                    }
                    A1[p] = (int) ((((int64)App * c2 + (int64)Aqq * s2)>>15) - a);
                    A2[q] = (int) ((((int64)App * s2 + (int64)Aqq * c2)>>15) + a);
                    A1[q] = A2[p] = 0;
                }               /*q */
            }                   /*p */
        }
        while( ind );
        //Amax /= n;
    }                           /* while ( Amax > ax ) */

    for( i = 0, k = 0; i < n; i++, k += n + 1 )
        E[i] = A[k];
    /*printf(" M = %d\n", M); */

    /* -------- ordering -------- */
    for( i = 0; i < n; i++ )
    {
        int m = i;
        int Em = abs( E[i] );

        for( j = i + 1; j < n; j++ )
        {
            int Ej = abs( E[j] );

            m = (Em < Ej) ? j : m;
            Em = (Em < Ej) ? Ej : Em;
        }
        if( m != i )
        {
            int l;
            int b = E[i];

            E[i] = E[m];
            E[m] = b;
            for( j = 0, k = i * n, l = m * n; j < n; j++, k++, l++ )
            {
                b = V[k];
                V[k] = V[l];
                V[l] = b;
            }
        }
    }

    return CV_NO_ERR;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: icvCalcCovarMatrixEx_8u32fR
//    Purpose: The function calculates a covariance matrix for a group of input objects
//             (images, vectors, etc.). ROI supported.
//    Context:
//    Parameters:  nObjects    - number of source objects
//                 objects     - array of pointers to ROIs of the source objects
//                 imgStep     - full width of each source object row in bytes
//                 avg         - pointer to averaged object
//                 avgStep     - full width of averaged object row in bytes
//                 size        - ROI size of each source and averaged objects
//                 covarMatrix - covariance matrix (output parameter; must be allocated
//                               before call)
//
//    Returns: CV_NO_ERR or error code
//
//    Notes:   
//F*/
static CvStatus  CV_STDCALL
icvCalcCovarMatrixEx_8u32fR( int nObjects, void *input, int objStep1,
                             int ioFlags, int ioBufSize, uchar* buffer,
                             void *userData, uchar *avgData, int avgStep,
                             CvSize size, int *covarMatrix )
{
    int objStep = objStep1;

    /* ---- TEST OF PARAMETERS ---- */

    if( nObjects < 2 )
        return CV_BADFACTOR_ERR;
    if( ioFlags )
        return CV_BADFACTOR_ERR;
    if( input == NULL || avgData == NULL || covarMatrix == NULL )
        return CV_NULLPTR_ERR;
    if( size.width > objStep || size.width > avgStep || size.height < 1 )
        return CV_BADSIZE_ERR;

    //avgStep /= 2;

    /* ==== NOT USE INPUT CALLBACK ==== */
    {
        int i, j;
        uchar **objects = (uchar **) (((CvInput *) & input)->data);

        for( i = 0; i < nObjects; i++ )
        {
            uchar *bu = objects[i];

            for( j = i; j < nObjects; j++ )
            {
                int k, l;
                int w = 0;
                uchar *a = avgData;
                uchar *bu1 = bu;
                uchar *bu2 = objects[j];

                for( k = 0; k < size.height;
                     k++, bu1 += objStep, bu2 += objStep, a += avgStep )
                {
                    for( l = 0; l < size.width - 3; l += 4 )
                    {
                        int f = a[l];
                        int u1 = bu1[l];
                        int u2 = bu2[l];

                        w += (u1 - f) * (u2 - f);
                        f = a[l + 1];
                        u1 = bu1[l + 1];
                        u2 = bu2[l + 1];
                        w += (u1 - f) * (u2 - f);
                        f = a[l + 2];
                        u1 = bu1[l + 2];
                        u2 = bu2[l + 2];
                        w += (u1 - f) * (u2 - f);
                        f = a[l + 3];
                        u1 = bu1[l + 3];
                        u2 = bu2[l + 3];
                        w += (u1 - f) * (u2 - f);
                    }
                    for( ; l < size.width; l++ )
                    {
                        int f = a[l];
                        int u1 = bu1[l];
                        int u2 = bu2[l];

                        w += (u1 - f) * (u2 - f);
                    }
                }

                covarMatrix[i * nObjects + j] = covarMatrix[j * nObjects + i] = w;
            }
        }
    }                           /* else */

    return CV_NO_ERR;
}

/*======================== end of icvCalcCovarMatrixEx_8u32fR ===========================*/


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: icvCalcEigenObjects_8u32fR
//    Purpose: The function calculates an orthonormal eigen basis and a mean (averaged)
//             object for a group of input objects (images, vectors, etc.). ROI supported.
//    Context:
//    Parameters: nObjects  - number of source objects
//                input     - pointer either to array of pointers to input objects
//                            or to read callback function (depending on ioFlags)
//                imgStep   - full width of each source object row in bytes
//                output    - pointer either to array of pointers to output eigen objects
//                            or to write callback function (depending on ioFlags)
//                eigStep   - full width of each eigenobject row in bytes
//                size      - ROI size of each source object
//                ioFlags   - input/output flags (see Notes)
//                ioBufSize - input/output buffer size
//                userData  - pointer to the structure which contains all necessary
//                            data for the callback functions
//                calcLimit - determines the calculation finish conditions
//                avg       - pointer to averaged object (has the same size as ROI)
//                avgStep   - full width of averaged object row in bytes
//                eigVals   - pointer to corresponding eigenvalues (array of <nObjects>
//                            elements in descending order) 
//
//    Returns: CV_NO_ERR or error code
//
//    Notes: 1. input/output data (that is, input objects and eigen ones) may either
//              be allocated in the RAM or be read from/written to the HDD (or any
//              other device) by read/write callback functions. It depends on the
//              value of ioFlags paramater, which may be the following:
//                  CV_EIGOBJ_NO_CALLBACK, or 0;
//                  CV_EIGOBJ_INPUT_CALLBACK;
//                  CV_EIGOBJ_OUTPUT_CALLBACK;
//                  CV_EIGOBJ_BOTH_CALLBACK, or
//                            CV_EIGOBJ_INPUT_CALLBACK | CV_EIGOBJ_OUTPUT_CALLBACK.
//              The callback functions as well as the user data structure must be
//              developed by the user.
//
//           2. If ioBufSize = 0, or it's too large, the function dermines buffer size
//              itself.
//
//           3. Depending on calcLimit parameter, calculations are finished either if
//              eigenfaces number comes up to certain value or the relation of the
//              current eigenvalue and the largest one comes down to certain value
//              (or any of the above conditions takes place). The calcLimit->type value
//              must be CV_TERMCRIT_NUMB, CV_TERMCRIT_EPS or
//              CV_TERMCRIT_NUMB | CV_TERMCRIT_EPS. The function returns the real
//              values calcLimit->max_iter and calcLimit->epsilon.
//
//           4. eigVals may be equal to NULL (if you don't need eigen values in further).
//
//F*/
static CvStatus CV_STDCALL
icvCalcEigenObjects_8u32fR( int nObjects, void* input, int objStep,
                            void* output, int eigStep, CvSize size,
                            int  ioFlags, int ioBufSize, void* userData,
                            CvTermCriteria* calcLimit, uchar* avgData,
                            int    avgStep, int  *eigVals )
{
    int i, j, n, iev = 0, m1 = nObjects - 1, objStep1 = objStep, eigStep1 = eigStep / 4;
    CvSize objSize, eigSize, avgSize;
    int *c = 0;
    int *ev = 0;
    unsigned int *bi = 0, *bavg = 0;
	uchar *ba;
    unsigned int m = 32768 / nObjects; // m is 17.15
    CvStatus r;
	int *feigVals;


    if( m1 > calcLimit->max_iter && calcLimit->type != CV_TERMCRIT_EPS )
        m1 = calcLimit->max_iter;

    /* ---- TEST OF PARAMETERS ---- */
    if( nObjects < 2 )
        return CV_BADFACTOR_ERR;
    if( ioFlags )
        return CV_BADFACTOR_ERR;
    if( input == NULL || output == NULL || avgData == NULL )
        return CV_NULLPTR_ERR;
    if( size.width > objStep || 2 * size.width > eigStep ||
        size.width > avgStep || size.height < 1 )
        return CV_BADSIZE_ERR;
    if( !(ioFlags & CV_EIGOBJ_INPUT_CALLBACK) )
        for( i = 0; i < nObjects; i++ )
            if( ((uchar **) input)[i] == NULL )
                return CV_NULLPTR_ERR;
    if( !(ioFlags & CV_EIGOBJ_OUTPUT_CALLBACK) )
        for( i = 0; i < m1; i++ )
            if( ((int **) output)[i] == NULL )
                return CV_NULLPTR_ERR;

	eigStep /= 2;

    if( objStep == size.width && eigStep == size.width && avgStep == size.width )
    {
        size.width *= size.height;
        size.height = 1;
        objStep = objStep1 = eigStep = eigStep1 = avgStep = size.width;
    }
    objSize = eigSize = avgSize = size;

    /* Calculation of averaged object */
	bavg = (unsigned int *) cvAlloc( sizeof( unsigned int ) * size.width * size.height );

	memset(bavg, 0, sizeof( unsigned int ) * size.width * size.height);

    for( i = 0; i < nObjects; i++ )
    {
        int k, l;
        uchar *bu = ((uchar **) input)[i];
		bi = bavg;
        for( k = 0; k < avgSize.height; k++, bi += avgStep, bu += objStep1 )
            for( l = 0; l < avgSize.width; l++ )
                bi[l] += bu[l];
    }

	ba = avgData;
	bi = bavg;
    for( i = 0; i < avgSize.height; i++, bi += avgStep, ba += avgStep )
        for( j = 0; j < avgSize.width; j++ )
            ba[j] = (uchar)(((uint64)bi[j]*m)>>15);

    cvFree( &bavg );

    /* Calculation of covariance matrix */
    c = (int *) cvAlloc( sizeof( int ) * nObjects * nObjects );

    if( c == NULL )
        return CV_OUTOFMEM_ERR;


    r = icvCalcCovarMatrixEx_8u32fR( nObjects, input, objStep1, ioFlags, ioBufSize,
                                      NULL, userData, avgData, avgStep, size, c );
    if( r )
        return r;

    /* Calculation of eigenvalues & eigenvectors */
    ev = (int *) cvAlloc( sizeof( int ) * nObjects * nObjects );	// s16.15

	if( ev == NULL ) {
        cvFree( &c );
        return CV_OUTOFMEM_ERR;
	}

    feigVals = (int *) cvAlloc( sizeof( int ) * nObjects );

    if( feigVals == NULL )
    {
        cvFree( &c );
        cvFree( &ev );
        return CV_OUTOFMEM_ERR;
    }
    iev = 1;

    r = icvJacobiEigens_32f( c, ev, feigVals, nObjects, 0 );
    cvFree( &c );
    if( r )
    {
        cvFree( &ev );
        if( iev )
            cvFree( &feigVals );
        return r;
    }

    /* Eigen objects number determination */
    if( calcLimit->type != CV_TERMCRIT_NUMBER )
    {
        for( i = 0; i < m1; i++ )
            if( fabs( (float)feigVals[i] / feigVals[0] ) < calcLimit->epsilon )
                break;
        m1 = calcLimit->max_iter = i;
    }
    else
        m1 = calcLimit->max_iter;
    calcLimit->epsilon =  fabs( (float)feigVals[m1 - 1] / feigVals[0] );

	if( eigVals != NULL ) {
        for( i = 0; i < m1; i++ )
            eigVals[i] = feigVals[i];
	}

	for( i = 0; i < m1; i++ ) 
        feigVals[i] = (32768 / isqrt(feigVals[i])); // change to s16.15

    /* ----------------- Calculation of eigenobjects ----------------------- */
    {
        int k, p, l;

        for( i = 0; i < m1; i++ )       /* e.o. annulation */
        {
            short *be = ((short **) output)[i];

            for( p = 0; p < eigSize.height; p++, be += eigStep )
                for( l = 0; l < eigSize.width; l++ )
                    be[l] = 0;
        }

        for( k = 0; k < nObjects; k++ )
        {
            uchar *bv = ((uchar **) input)[k];

            for( i = 0; i < m1; i++ )
            {
                int v = (feigVals[i] * ev[i * nObjects + k])>>15;
                short *be = ((short **) output)[i];
                uchar *bu = bv;
                ba = avgData;

                for( p = 0; p < size.height; p++, bu += objStep1,
                     ba += avgStep, be += eigStep1 )
                {
                    for( l = 0; l < size.width - 3; l += 4 )
                    {
                        int a = ba[l];
                        int u = bu[l];

                        be[l] += (short)(v * (u - a));
                        a = ba[l + 1];
                        u = bu[l + 1];
                        be[l + 1] += (short)(v * (u - a));
                        a = ba[l + 2];
                        u = bu[l + 2];
                        be[l + 2] += (short)(v * (u - a));
                        a = ba[l + 3];
                        u = bu[l + 3];
                        be[l + 3] += (short)(v * (u - a));
                    }
					for( ; l < size.width; l++ ) 
					{
                       int a = ba[l];
                       int u = bu[l];
                       be[l] += (short)(v * (u - a));
					}
                }
            }                   /* i */
        }                       /* k */
    }                           /* else */

    cvFree( &ev );
	if( iev == 1 )
        cvFree( &feigVals );

    return CV_NO_ERR;
}

/* --- End of icvCalcEigenObjects_8u32fR --- */

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: icvCalcDecompCoeff_8u32fR
//    Purpose: The function calculates one decomposition coefficient of input object
//             using previously calculated eigen object and the mean (averaged) object
//    Context:
//    Parameters:  obj     - input object
//                 objStep - its step (in bytes)
//                 eigObj  - pointer to eigen object
//                 eigStep - its step (in bytes)
//                 avg     - pointer to averaged object
//                 avgStep - its step (in bytes)
//                 size    - ROI size of each source object
//
//    Returns: decomposition coefficient value or large negative value (if error)
//
//    Notes:
//F*/
static int CV_STDCALL
icvCalcDecompCoeff_8u32fR( uchar* obj, int objStep,
                           short *eigObj, int eigStep,
                           uchar *avgData, int avgStep, CvSize size )
{
    int i, k;
    int w = 0;
	uchar *ba = avgData;

    if( size.width > objStep || 2 * size.width > eigStep
        || size.width > avgStep || size.height < 1 )
        return 0x80000000;
    if( obj == NULL || eigObj == NULL || avgData == NULL )
        return 0x80000000;

    eigStep /= 2;

    if( size.width == objStep && size.width == eigStep && size.width == avgStep )
    {
        size.width *= size.height;
        size.height = 1;
        objStep = eigStep = avgStep = size.width;
    }

    for( i = 0; i < size.height; i++, obj += objStep, eigObj += eigStep, ba += avgStep )
    {
        for( k = 0; k < size.width - 4; k += 4 )
        {
            int o = obj[k];
            short e = eigObj[k];  // s.15
            int a = ba[k];

            w += e * (o - a);
            o = obj[k + 1];
            e = eigObj[k + 1];
            a = ba[k + 1];
            w += e * (o - a);
            o = obj[k + 2];
            e = eigObj[k + 2];
            a = ba[k + 2];
            w += e * (o - a);
            o = obj[k + 3];
            e = eigObj[k + 3];
            a = ba[k +3];
            w += e * (o - a);
        }
		for( ; k < size.width; k++ ) 
		{
           int o = obj[k];
           short e = eigObj[k];
           int a = ba[k];
           w += e * (o - a);
		}
    }

    return w>>15;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Names: icvEigenDecomposite_8u32fR
//    Purpose: The function calculates all decomposition coefficients for input object
//             using previously calculated eigen objects basis and the mean (averaged)
//             object
//    Context:
//    Parameters:  obj         - input object
//                 objStep     - its step (in bytes)
//                 nEigObjs    - number of eigen objects
//                 eigInput    - pointer either to array of pointers to eigen objects
//                               or to read callback function (depending on ioFlags)
//                 eigStep     - eigen objects step (in bytes)
//                 ioFlags     - input/output flags
//                 iserData    - pointer to the structure which contains all necessary
//                               data for the callback function
//                 avg         - pointer to averaged object
//                 avgStep     - its step (in bytes)
//                 size        - ROI size of each source object
//                 coeffs      - calculated coefficients (output data)
//
//    Returns: icv status
//
//    Notes:   see notes for icvCalcEigenObjects_8u32fR function
//F*/
static CvStatus CV_STDCALL
icvEigenDecomposite_8u32fR( uchar * obj, int objStep, int nEigObjs,
                            void *eigInput, int eigStep, int ioFlags,
                            void *userData, uchar *avgData, int avgStep,
                            CvSize size, int *coeffs )
{
    int i;

    if( nEigObjs < 1 )
        return CV_BADFACTOR_ERR;
    if( ioFlags < 0 || ioFlags > 0 )
        return CV_BADFACTOR_ERR;
    if( size.width > objStep || 2 * size.width > eigStep ||
        size.width > avgStep || size.height < 1 )
        return CV_BADSIZE_ERR;
    if( obj == NULL || eigInput == NULL || coeffs == NULL || avgData == NULL )
        return CV_NULLPTR_ERR;
    if( !ioFlags )
        for( i = 0; i < nEigObjs; i++ )
            if( ((uchar **) eigInput)[i] == NULL )
                return CV_NULLPTR_ERR;

        /* no callback */
        for( i = 0; i < nEigObjs; i++ )
        {
            int w = icvCalcDecompCoeff_8u32fR( obj, objStep, ((short **) eigInput)[i],
                                                  eigStep, avgData, avgStep, size );

            if( w == 0x80000000 )
                return CV_NOTDEFINED_ERR;
            coeffs[i] = w;
        }

    return CV_NO_ERR;
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Names: icvEigenProjection_8u32fR
//    Purpose: The function calculates object projection to the eigen sub-space (restores
//             an object) using previously calculated eigen objects basis, mean (averaged)
//             object and decomposition coefficients of the restored object
//    Context:
//    Parameters:  nEigObjs - number of eigen objects
//                 eigens  - array of pointers to eigen objects  format s.15
//                 eigStep  - eigen objects step (in bytes)
//                 coeffs   - previousely calculated decomposition coefficients
//                 avg      - pointer to averaged object
//                 avgStep  - its step (in bytes)
//                 rest     - pointer to restored object
//                 restStep - its step (in bytes)
//                 size     - ROI size of each object
//
//    Returns: CV status
//
//    Notes:
//F*/
static CvStatus CV_STDCALL
icvEigenProjection_8u32fR( int nEigObjs, void *eigInput, int eigStep,
                           int ioFlags, void *userData, int *coeffs,
                           uchar *avgData, int avgStep, uchar * rest,
                           int restStep, CvSize size )
{
    int i, j, k;
    int *buf;
    int *b;
	uchar *ba;

    if( size.width > avgStep || 2 * size.width > eigStep || size.height < 1 )
        return CV_BADSIZE_ERR;
    if( rest == NULL || eigInput == NULL || avgData == NULL || coeffs == NULL )
        return CV_NULLPTR_ERR;
    if( ioFlags < 0 || ioFlags > 0 )
        return CV_BADFACTOR_ERR;
    if( !ioFlags )
        for( i = 0; i < nEigObjs; i++ )
            if( ((uchar **) eigInput)[i] == NULL )
                return CV_NULLPTR_ERR;
    eigStep /= 2;

    if( size.width == restStep && size.width == eigStep && size.width == avgStep )
    {
        size.width *= size.height;
        size.height = 1;
        restStep = eigStep = avgStep = size.width;
    }

    buf = (int *) cvAlloc( sizeof( int ) * size.width * size.height );

    if( buf == NULL )
        return CV_OUTOFMEM_ERR;
    b = buf;
	ba = avgData;
    for( i = 0; i < size.height; i++, ba += avgStep, b += size.width )
        for( j = 0; j < size.width; j++ )
            b[j] = ba[j];

    for( k = 0; k < nEigObjs; k++ )
    {
        short *e = ((short **) eigInput)[k];
        int c = coeffs[k];

        b = buf;
        for( i = 0; i < size.height; i++, e += eigStep, b += size.width )
        {
            for( j = 0; j < size.width - 3; j += 4 )
            {
                int b0 = (c * e[j])>>15;
                int b1 = (c * e[j + 1])>>15;
                int b2 = (c * e[j + 2])>>15;
                int b3 = (c * e[j + 3])>>15;

                b[j] += b0;
                b[j + 1] += b1;
                b[j + 2] += b2;
                b[j + 3] += b3;
            }
            for( ; j < size.width; j++ )
                b[j] += (c * e[j])>>15;
        }
    }

    b = buf;
    for( i = 0; i < size.height; i++, b += size.width, rest += restStep )
        for( j = 0; j < size.width; j++ )
        {
            int w = b[j];
            w = !(w & ~255) ? w : w < 0 ? 0 : 255;
            rest[j] = (uchar) w;
        }

    cvFree( &buf );
    return CV_NO_ERR;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCalcCovarMatrixEx
//    Purpose: The function calculates a covariance matrix for a group of input objects
//             (images, vectors, etc.).
//    Context:
//    Parameters:  nObjects    - number of source objects
//                 input       - pointer either to array of input objects
//                               or to read callback function (depending on ioFlags)
//                 ioFlags     - input/output flags (see Notes to
//                               cvCalcEigenObjects function)
//                 ioBufSize   - input/output buffer size
//                 userData    - pointer to the structure which contains all necessary
//                               data for the callback functions
//                 avg         - averaged object
//                 covarMatrix - covariance matrix (output parameter; must be allocated
//                               before call)
//
//    Notes:  See Notes to cvCalcEigenObjects function
//F*/

CV_IMPL void
cvCalcCovarMatrixEx( int  nObjects, void*  input, int  ioFlags,
                     int  ioBufSize, uchar*  buffer, void*  userData,
                     IplImage* avg, int*  covarMatrix )
{
    uchar *avg_data;
    int avg_step = 0;
    CvSize avg_size;
    int i;

    CV_FUNCNAME( "cvCalcCovarMatrixEx" );

    __BEGIN__;

    cvGetImageRawData( avg, (uchar **) & avg_data, &avg_step, &avg_size );
    if( avg->depth != IPL_DEPTH_8U )
        CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
    if( avg->nChannels != 1 )
        CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );

    if( ioFlags == CV_EIGOBJ_NO_CALLBACK )
    {
        IplImage **images = (IplImage **) (((CvInput *) & input)->data);
        uchar **objects = (uchar **) cvAlloc( sizeof( uchar * ) * nObjects );
        int img_step = 0, old_step = 0;
        CvSize img_size = avg_size, old_size = avg_size;

        if( objects == NULL )
            CV_ERROR( CV_StsBadArg, "Insufficient memory" );

        for( i = 0; i < nObjects; i++ )
        {
            IplImage *img = images[i];
            uchar *img_data;

            cvGetImageRawData( img, &img_data, &img_step, &img_size );
            if( img->depth != IPL_DEPTH_8U )
                CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
            if( img_size != avg_size || img_size != old_size )
                CV_ERROR( CV_StsBadArg, "Different sizes of objects" );
            if( img->nChannels != 1 )
                CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );
            if( i > 0 && img_step != old_step )
                CV_ERROR( CV_StsBadArg, "Different steps of objects" );

            old_step = img_step;
            old_size = img_size;
            objects[i] = img_data;
        }

        CV_CALL( icvCalcCovarMatrixEx_8u32fR( nObjects,
                                              (void*) objects,
                                              img_step,
                                              CV_EIGOBJ_NO_CALLBACK,
                                              0,
                                              NULL,
                                              NULL,
                                              avg_data,
                                              avg_step,
                                              avg_size,
                                              covarMatrix ));
        cvFree( &objects );
    }

    else

    {
		CV_ERROR( CV_StsBadArg, "Unsupported i/o flag" );
    }

    __END__;
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCalcEigenObjects
//    Purpose: The function calculates an orthonormal eigen basis and a mean (averaged)
//             object for a group of input objects (images, vectors, etc.).
//    Context:
//    Parameters: nObjects  - number of source objects
//                input     - pointer either to array of input objects
//                            or to read callback function (depending on ioFlags)
//                output    - pointer either to output eigen objects
//                            or to write callback function (depending on ioFlags)
							  format: s.15
//                ioFlags   - input/output flags (see Notes)
//                ioBufSize - input/output buffer size
//                userData  - pointer to the structure which contains all necessary
//                            data for the callback functions
//                calcLimit - determines the calculation finish conditions
//                avg       - averaged object (has the same size as ROI) format: 8
//                eigVals   - pointer to corresponding eigen values (array of <nObjects>
//                            elements in descending order) format: s31
//
//    Notes: 1. input/output data (that is, input objects and eigen ones) may either
//              be allocated in the RAM or be read from/written to the HDD (or any
//              other device) by read/write callback functions. It depends on the
//              value of ioFlags paramater, which may be the following:
//                  CV_EIGOBJ_NO_CALLBACK, or 0;
//                  CV_EIGOBJ_INPUT_CALLBACK;
//                  CV_EIGOBJ_OUTPUT_CALLBACK;
//                  CV_EIGOBJ_BOTH_CALLBACK, or
//                            CV_EIGOBJ_INPUT_CALLBACK | CV_EIGOBJ_OUTPUT_CALLBACK.
//              The callback functions as well as the user data structure must be
//              developed by the user.
//
//           2. If ioBufSize = 0, or it's too large, the function dermines buffer size
//              itself.
//
//           3. Depending on calcLimit parameter, calculations are finished either if
//              eigenfaces number comes up to certain value or the relation of the
//              current eigenvalue and the largest one comes down to certain value
//              (or any of the above conditions takes place). The calcLimit->type value
//              must be CV_TERMCRIT_NUMB, CV_TERMCRIT_EPS or
//              CV_TERMCRIT_NUMB | CV_TERMCRIT_EPS. The function returns the real
//              values calcLimit->max_iter and calcLimit->epsilon.
//
//           4. eigVals may be equal to NULL (if you don't need eigen values in further).
//
//F*/
CV_IMPL void
cvCalcEigenObjects( int       nObjects,
                    void*     input,
                    void*     output,
                    int       ioFlags,
                    int       ioBufSize,
                    void*     userData,
                    CvTermCriteria* calcLimit,
                    IplImage* avg, 
                    int*    eigVals )
{
    uchar *avg_data;
    int avg_step = 0;
    CvSize avg_size;
    int i;
    int nEigens = nObjects - 1;

    CV_FUNCNAME( "cvCalcEigenObjects" );

    __BEGIN__;

    cvGetImageRawData( avg, (uchar **) & avg_data, &avg_step, &avg_size );
    if( avg->depth != IPL_DEPTH_8U )
        CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
    if( avg->nChannels != 1 )
        CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );

    if( nEigens > calcLimit->max_iter && calcLimit->type != CV_TERMCRIT_EPS )
        nEigens = calcLimit->max_iter;

    switch (ioFlags)
    {
    case CV_EIGOBJ_NO_CALLBACK:
        {
            IplImage **objects = (IplImage **) (((CvInput *) & input)->data);
            IplImage **eigens = (IplImage **) (((CvInput *) & output)->data);
            uchar **objs = (uchar **) cvAlloc( sizeof( uchar * ) * nObjects );
            short **eigs = (short **) cvAlloc( sizeof( short * ) * nEigens );
            int obj_step = 0, old_step = 0;
            int eig_step = 0, oldeig_step = 0;
            CvSize obj_size = avg_size, old_size = avg_size,

                eig_size = avg_size, oldeig_size = avg_size;

            if( objects == NULL || eigens == NULL )
                CV_ERROR( CV_StsBadArg, "Insufficient memory" );

            for( i = 0; i < nObjects; i++ )
            {
                IplImage *img = objects[i];
                uchar *obj_data;

                cvGetImageRawData( img, &obj_data, &obj_step, &obj_size );
                if( img->depth != IPL_DEPTH_8U )
                    CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
                if( obj_size != avg_size || obj_size != old_size )
                    CV_ERROR( CV_StsBadArg, "Different sizes of objects" );
                if( img->nChannels != 1 )
                    CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );
                if( i > 0 && obj_step != old_step )
                    CV_ERROR( CV_StsBadArg, "Different steps of objects" );

                old_step = obj_step;
                old_size = obj_size;
                objs[i] = obj_data;
            }
            for( i = 0; i < nEigens; i++ )
            {
                IplImage *eig = eigens[i];
                short *eig_data;

                cvGetImageRawData( eig, (uchar **) & eig_data, &eig_step, &eig_size );
                if( eig->depth != IPL_DEPTH_16S )
                    CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
                if( eig_size != avg_size || eig_size != oldeig_size )
                    CV_ERROR( CV_StsBadArg, "Different sizes of objects" );
                if( eig->nChannels != 1 )
                    CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );
                if( i > 0 && eig_step != oldeig_step )
                    CV_ERROR( CV_StsBadArg, "Different steps of objects" );

                oldeig_step = eig_step;
                oldeig_size = eig_size;
                eigs[i] = eig_data;
            }
            CV_CALL( icvCalcEigenObjects_8u32fR( nObjects, (void*) objs, obj_step,
                                                 (void*) eigs, eig_step, obj_size,
                                                 ioFlags, ioBufSize, userData,
                                                 calcLimit, avg_data, avg_step, eigVals ));
            cvFree( &objs );
            cvFree( &eigs );
            break;
        }

    default:
        CV_ERROR( CV_StsBadArg, "Unsupported i/o flag" );
    }

    __END__;
}

/*--------------------------------------------------------------------------------------*/
/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvCalcDecompCoeff
//    Purpose: The function calculates one decomposition coefficient of input object
//             using previously calculated eigen object and the mean (averaged) object
//    Context:
//    Parameters:  obj     - input object
//                 eigObj  - eigen object
//                 avg     - averaged object
//
//    Returns: decomposition coefficient value or large negative value (if error)
//
//    Notes:
//F*/

CV_IMPL double
cvCalcDecompCoeff( IplImage * obj, IplImage * eigObj, IplImage * avg )
{
    int coeff = 0x80000000;

    uchar *obj_data;
    short *eig_data;
    uchar *avg_data;
    int obj_step = 0, eig_step = 0, avg_step = 0;
    CvSize obj_size, eig_size, avg_size;

    CV_FUNCNAME( "cvCalcDecompCoeff" );

    __BEGIN__;

    cvGetImageRawData( obj, &obj_data, &obj_step, &obj_size );
    if( obj->depth != IPL_DEPTH_8U )
        CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
    if( obj->nChannels != 1 )
        CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );

    cvGetImageRawData( eigObj, (uchar **) & eig_data, &eig_step, &eig_size );
    if( eigObj->depth != IPL_DEPTH_16S )
        CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
    if( eigObj->nChannels != 1 )
        CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );

    cvGetImageRawData( avg, (uchar **) & avg_data, &avg_step, &avg_size );
    if( avg->depth != IPL_DEPTH_8U )
        CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
    if( avg->nChannels != 1 )
        CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );

    if( obj_size != eig_size || obj_size != avg_size )
        CV_ERROR( CV_StsBadArg, "different sizes of images" );

    coeff = icvCalcDecompCoeff_8u32fR( obj_data, obj_step,
                                       eig_data, eig_step,
                                       avg_data, avg_step, obj_size );

    __END__;
    
    return coeff;
}

/*--------------------------------------------------------------------------------------*/
/*F///////////////////////////////////////////////////////////////////////////////////////
//    Names: cvEigenDecomposite
//    Purpose: The function calculates all decomposition coefficients for input object
//             using previously calculated eigen objects basis and the mean (averaged)
//             object
//
//    Parameters:  obj         - input object
//                 nEigObjs    - number of eigen objects
//                 eigInput    - pointer either to array of pointers to eigen objects
//                               or to read callback function (depending on ioFlags)
//                 ioFlags     - input/output flags
//                 userData    - pointer to the structure which contains all necessary
//                               data for the callback function
//                 avg         - averaged object
//                 coeffs      - calculated coefficients (output data)
//
//    Notes:   see notes for cvCalcEigenObjects function
//F*/

CV_IMPL void
cvEigenDecomposite( IplImage* obj,
                    int       nEigObjs,
                    void*     eigInput,
                    int       ioFlags, 
                    void*     userData, 
                    IplImage* avg, 
                    int*    coeffs )
{
    uchar *avg_data;
    uchar *obj_data;
    int avg_step = 0, obj_step = 0;
    CvSize avg_size, obj_size;
    int i;

    CV_FUNCNAME( "cvEigenDecomposite" );

    __BEGIN__;

    cvGetImageRawData( avg, (uchar **) & avg_data, &avg_step, &avg_size );
    if( avg->depth != IPL_DEPTH_8U )
        CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
    if( avg->nChannels != 1 )
        CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );

    cvGetImageRawData( obj, &obj_data, &obj_step, &obj_size );
    if( obj->depth != IPL_DEPTH_8U )
        CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
    if( obj->nChannels != 1 )
        CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );

    if( obj_size != avg_size )
        CV_ERROR( CV_StsBadArg, "Different sizes of objects" );

    if( ioFlags == CV_EIGOBJ_NO_CALLBACK )
    {
        IplImage **eigens = (IplImage **) (((CvInput *) & eigInput)->data);
        short **eigs = (short **) cvAlloc( sizeof( short * ) * nEigObjs );
        int eig_step = 0, old_step = 0;
        CvSize eig_size = avg_size, old_size = avg_size;

        if( eigs == NULL )
            CV_ERROR( CV_StsBadArg, "Insufficient memory" );

        for( i = 0; i < nEigObjs; i++ )
        {
            IplImage *eig = eigens[i];
            short *eig_data;

            cvGetImageRawData( eig, (uchar **) & eig_data, &eig_step, &eig_size );
            if( eig->depth != IPL_DEPTH_16S )
                CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
            if( eig_size != avg_size || eig_size != old_size )
                CV_ERROR( CV_StsBadArg, "Different sizes of objects" );
            if( eig->nChannels != 1 )
                CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );
            if( i > 0 && eig_step != old_step )
                CV_ERROR( CV_StsBadArg, "Different steps of objects" );

            old_step = eig_step;
            old_size = eig_size;
            eigs[i] = eig_data;
        }

        CV_CALL( icvEigenDecomposite_8u32fR( obj_data,
                                             obj_step,
                                             nEigObjs,
                                             (void*) eigs,
                                             eig_step,
                                             ioFlags,
                                             userData,
                                             avg_data,
                                             avg_step,
                                             obj_size,
                                             coeffs   ));
        cvFree( &eigs );
    }
	else 
	{
		CV_ERROR( CV_StsBadArg, "Unsupported i/o flag" );
	}

    __END__;
}

/*--------------------------------------------------------------------------------------*/
/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: cvEigenProjection
//    Purpose: The function calculates object projection to the eigen sub-space (restores
//             an object) using previously calculated eigen objects basis, mean (averaged)
//             object and decomposition coefficients of the restored object
//    Context:
//    Parameters:  nEigObjs    - number of eigen objects
//                 eigInput    - pointer either to array of pointers to eigen objects
//                               or to read callback function (depending on ioFlags)
//                 ioFlags     - input/output flags
//                 userData    - pointer to the structure which contains all necessary
//                               data for the callback function
//                 coeffs      - array of decomposition coefficients
//                 avg         - averaged object
//                 proj        - object projection (output data)
//
//    Notes:   see notes for cvCalcEigenObjects function
//F*/

CV_IMPL void
cvEigenProjection( void*     eigInput,
                   int       nEigObjs,
                   int       ioFlags,
                   void*     userData,
                   int*    coeffs, 
                   IplImage* avg,
                   IplImage* proj )
{
    uchar *avg_data;
    uchar *proj_data;
    int avg_step = 0, proj_step = 0;
    CvSize avg_size, proj_size;
    int i;

    CV_FUNCNAME( "cvEigenProjection" );

    __BEGIN__;

    cvGetImageRawData( avg, (uchar **) & avg_data, &avg_step, &avg_size );
    if( avg->depth != IPL_DEPTH_8U )
        CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
    if( avg->nChannels != 1 )
        CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );

    cvGetImageRawData( proj, &proj_data, &proj_step, &proj_size );
    if( proj->depth != IPL_DEPTH_8U )
        CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
    if( proj->nChannels != 1 )
        CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );

    if( proj_size != avg_size )
        CV_ERROR( CV_StsBadArg, "Different sizes of projects" );

    if( ioFlags == CV_EIGOBJ_NO_CALLBACK )
    {
        IplImage **eigens = (IplImage**) (((CvInput *) & eigInput)->data);
        short **eigs = (short**) cvAlloc( sizeof( short * ) * nEigObjs );
        int eig_step = 0, old_step = 0;
        CvSize eig_size = avg_size, old_size = avg_size;

        if( eigs == NULL )
            CV_ERROR( CV_StsBadArg, "Insufficient memory" );

        for( i = 0; i < nEigObjs; i++ )
        {
            IplImage *eig = eigens[i];
            short *eig_data;

            cvGetImageRawData( eig, (uchar **) & eig_data, &eig_step, &eig_size );
            if( eig->depth != IPL_DEPTH_16S )
                CV_ERROR( CV_BadDepth, cvUnsupportedFormat );
            if( eig_size != avg_size || eig_size != old_size )
                CV_ERROR( CV_StsBadArg, "Different sizes of objects" );
            if( eig->nChannels != 1 )
                CV_ERROR( CV_BadNumChannels, cvUnsupportedFormat );
            if( i > 0 && eig_step != old_step )
                CV_ERROR( CV_StsBadArg, "Different steps of objects" );

            old_step = eig_step;
            old_size = eig_size;
            eigs[i] = eig_data;
        }

        CV_CALL( icvEigenProjection_8u32fR( nEigObjs,
                                            (void*) eigs,
                                            eig_step,
                                            ioFlags,
                                            userData,
                                            coeffs,
                                            avg_data,
                                            avg_step,
                                            proj_data,
                                            proj_step,
                                            avg_size   ));
        cvFree( &eigs );
    }

    else
    {
		CV_ERROR( CV_StsBadArg, "Unsupported i/o flag" );
    }

    __END__;
}

/* End of file. */
