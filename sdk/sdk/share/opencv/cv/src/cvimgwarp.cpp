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

/* ////////////////////////////////////////////////////////////////////
//
//  Geometrical transforms on images and matrices: rotation, zoom etc.
//
// */

#include "_cv.h"


/************** interpolation constants and tables ***************/

#define ICV_WARP_MUL_ONE_8U(x)  ((x) << ICV_WARP_SHIFT)
#define ICV_WARP_DESCALE_8U(x)  CV_DESCALE((x), ICV_WARP_SHIFT*2)
#define ICV_WARP_CLIP_X(x)      ((unsigned)(x) < (unsigned)ssize.width ? \
                                (x) : (x) < 0 ? 0 : ssize.width - 1)
#define ICV_WARP_CLIP_Y(y)      ((unsigned)(y) < (unsigned)ssize.height ? \
                                (y) : (y) < 0 ? 0 : ssize.height - 1)

/****************************************************************************************\
*                                         Resize                                         *
\****************************************************************************************/

static CvStatus CV_STDCALL
icvResize_NN_8u_C1R( const uchar* src, int srcstep, CvSize ssize,
                     uchar* dst, int dststep, CvSize dsize, int pix_size )
{
    int* x_ofs = (int*)cvStackAlloc( dsize.width * sizeof(x_ofs[0]) );
    int pix_size4 = pix_size / sizeof(int);
    int x, y, t;

    for( x = 0; x < dsize.width; x++ )
    {
        t = (ssize.width*x*2 + MIN(ssize.width, dsize.width) - 1)/(dsize.width*2);
        t -= t >= ssize.width;
        x_ofs[x] = t*pix_size;
    }

    for( y = 0; y < dsize.height; y++, dst += dststep )
    {
        const uchar* tsrc;
        t = (ssize.height*y*2 + MIN(ssize.height, dsize.height) - 1)/(dsize.height*2);
        t -= t >= ssize.height;
        tsrc = src + srcstep*t;

        switch( pix_size )
        {
        case 1:
            for( x = 0; x <= dsize.width - 2; x += 2 )
            {
                uchar t0 = tsrc[x_ofs[x]];
                uchar t1 = tsrc[x_ofs[x+1]];

                dst[x] = t0;
                dst[x+1] = t1;
            }

            for( ; x < dsize.width; x++ )
                dst[x] = tsrc[x_ofs[x]];
            break;
        case 2:
            for( x = 0; x < dsize.width; x++ )
                *(ushort*)(dst + x*2) = *(ushort*)(tsrc + x_ofs[x]);
            break;
        case 3:
            for( x = 0; x < dsize.width; x++ )
            {
                const uchar* _tsrc = tsrc + x_ofs[x];
                dst[x*3] = _tsrc[0]; dst[x*3+1] = _tsrc[1]; dst[x*3+2] = _tsrc[2];
            }
            break;
        case 4:
            for( x = 0; x < dsize.width; x++ )
                *(int*)(dst + x*4) = *(int*)(tsrc + x_ofs[x]);
            break;
        case 6:
            for( x = 0; x < dsize.width; x++ )
            {
                const ushort* _tsrc = (const ushort*)(tsrc + x_ofs[x]);
                ushort* _tdst = (ushort*)(dst + x*6);
                _tdst[0] = _tsrc[0]; _tdst[1] = _tsrc[1]; _tdst[2] = _tsrc[2];
            }
            break;
        default:
            for( x = 0; x < dsize.width; x++ )
                CV_MEMCPY_INT( dst + x*pix_size, tsrc + x_ofs[x], pix_size4 );
        }
    }

    return CV_OK;
}


typedef struct CvResizeAlpha
{
    int idx;
    union
    {
        float alpha;
        int ialpha;
    };
}
CvResizeAlpha;


#define  ICV_DEF_RESIZE_BILINEAR_FUNC( flavor, arrtype, worktype, alpha_field,  \
                                       mul_one_macro, descale_macro )           \
static CvStatus CV_STDCALL                                                      \
icvResize_Bilinear_##flavor##_CnR( const arrtype* src, int srcstep, CvSize ssize,\
                                   arrtype* dst, int dststep, CvSize dsize,     \
                                   int cn, int xmax,                            \
                                   const CvResizeAlpha* xofs,                   \
                                   const CvResizeAlpha* yofs,                   \
                                   worktype* buf0, worktype* buf1 )             \
{                                                                               \
    int prev_sy0 = -1, prev_sy1 = -1;                                           \
    int k, dx, dy;                                                              \
                                                                                \
    srcstep /= sizeof(src[0]);                                                  \
    dststep /= sizeof(dst[0]);                                                  \
    dsize.width *= cn;                                                          \
    xmax *= cn;                                                                 \
                                                                                \
    for( dy = 0; dy < dsize.height; dy++, dst += dststep )                      \
    {                                                                           \
        worktype fy = yofs[dy].alpha_field, *swap_t;                            \
        int sy0 = yofs[dy].idx, sy1 = sy0 + (fy > 0 && sy0 < ssize.height-1);   \
                                                                                \
        if( sy0 == prev_sy0 && sy1 == prev_sy1 )                                \
            k = 2;                                                              \
        else if( sy0 == prev_sy1 )                                              \
        {                                                                       \
            CV_SWAP( buf0, buf1, swap_t );                                      \
            k = 1;                                                              \
        }                                                                       \
        else                                                                    \
            k = 0;                                                              \
                                                                                \
        for( ; k < 2; k++ )                                                     \
        {                                                                       \
            worktype* _buf = k == 0 ? buf0 : buf1;                              \
            const arrtype* _src;                                                \
            int sy = k == 0 ? sy0 : sy1;                                        \
            if( k == 1 && sy1 == sy0 )                                          \
            {                                                                   \
                memcpy( buf1, buf0, dsize.width*sizeof(buf0[0]) );              \
                continue;                                                       \
            }                                                                   \
                                                                                \
            _src = src + sy*srcstep;                                            \
            for( dx = 0; dx < xmax; dx++ )                                      \
            {                                                                   \
                int sx = xofs[dx].idx;                                          \
                worktype fx = xofs[dx].alpha_field;                             \
                worktype t = _src[sx];                                          \
                _buf[dx] = mul_one_macro(t) + fx*(_src[sx+cn] - t);             \
            }                                                                   \
                                                                                \
            for( ; dx < dsize.width; dx++ )                                     \
                _buf[dx] = mul_one_macro(_src[xofs[dx].idx]);                   \
        }                                                                       \
                                                                                \
        prev_sy0 = sy0;                                                         \
        prev_sy1 = sy1;                                                         \
                                                                                \
        if( sy0 == sy1 )                                                        \
            for( dx = 0; dx < dsize.width; dx++ )                               \
                dst[dx] = (arrtype)descale_macro( mul_one_macro(buf0[dx]));     \
        else                                                                    \
            for( dx = 0; dx < dsize.width; dx++ )                               \
                dst[dx] = (arrtype)descale_macro( mul_one_macro(buf0[dx]) +     \
                                                  fy*(buf1[dx] - buf0[dx]));    \
    }                                                                           \
                                                                                \
    return CV_OK;                                                               \
}


typedef struct CvDecimateAlpha
{
    int si, di;
    float alpha;
}
CvDecimateAlpha;

ICV_DEF_RESIZE_BILINEAR_FUNC( 8u, uchar, int, ialpha,
                              ICV_WARP_MUL_ONE_8U, ICV_WARP_DESCALE_8U )
ICV_DEF_RESIZE_BILINEAR_FUNC( 16u, ushort, float, alpha, CV_NOP, cvRound )
ICV_DEF_RESIZE_BILINEAR_FUNC( 32f, float, float, alpha, CV_NOP, CV_NOP )

static void icvInitResizeTab( CvFuncTable* bilin_tab,
                              CvFuncTable* bicube_tab,
                              CvFuncTable* areafast_tab,
                              CvFuncTable* area_tab )
{
    bilin_tab->fn_2d[CV_8U] = (void*)icvResize_Bilinear_8u_CnR;
    bilin_tab->fn_2d[CV_16U] = (void*)icvResize_Bilinear_16u_CnR;
    bilin_tab->fn_2d[CV_32F] = (void*)icvResize_Bilinear_32f_CnR;
}


typedef CvStatus (CV_STDCALL * CvResizeBilinearFunc)
                    ( const void* src, int srcstep, CvSize ssize,
                      void* dst, int dststep, CvSize dsize,
                      int cn, int xmax, const CvResizeAlpha* xofs,
                      const CvResizeAlpha* yofs, float* buf0, float* buf1 );


CV_IMPL void
cvResize( const CvArr* srcarr, CvArr* dstarr, int method )
{
    static CvFuncTable bilin_tab, bicube_tab, areafast_tab, area_tab;
    static int inittab = 0;
    void* temp_buf = 0;

    CV_FUNCNAME( "cvResize" );

    __BEGIN__;
    
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize ssize, dsize;
    float scale_x, scale_y;
    int k, sx, sy, dx, dy;
    int type, depth, cn;
    
    CV_CALL( src = cvGetMat( srcarr, &srcstub ));
    CV_CALL( dst = cvGetMat( dstarr, &dststub ));
    
    if( CV_ARE_SIZES_EQ( src, dst ))
        CV_CALL( cvCopy( src, dst ));

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( !inittab )
    {
        icvInitResizeTab( &bilin_tab, &bicube_tab, &areafast_tab, &area_tab );
        inittab = 1;
    }

    ssize = cvGetMatSize( src );
    dsize = cvGetMatSize( dst );
    type = CV_MAT_TYPE(src->type);
    depth = CV_MAT_DEPTH(type);
    cn = CV_MAT_CN(type);
    scale_x = (float)ssize.width/dsize.width;
    scale_y = (float)ssize.height/dsize.height;

    if( method == CV_INTER_CUBIC &&
        (MIN(ssize.width, dsize.width) <= 4 ||
        MIN(ssize.height, dsize.height) <= 4) )
        method = CV_INTER_LINEAR;

    if( method == CV_INTER_NN )
    {
        IPPI_CALL( icvResize_NN_8u_C1R( src->data.ptr, src->step, ssize,
                                        dst->data.ptr, dst->step, dsize,
                                        CV_ELEM_SIZE(src->type)));
    }
    else if( method == CV_INTER_LINEAR)
    {
        {
            float inv_scale_x = (float)dsize.width/ssize.width;
            float inv_scale_y = (float)dsize.height/ssize.height;
            int xmax = dsize.width, width = dsize.width*cn, buf_size;
            float *buf0, *buf1;
            CvResizeAlpha *xofs, *yofs;
            int area_mode = method == CV_INTER_AREA;
            float fx, fy;
            CvResizeBilinearFunc func = (CvResizeBilinearFunc)bilin_tab.fn_2d[depth];

            if( !func )
                CV_ERROR( CV_StsUnsupportedFormat, "" );

            buf_size = width*2*sizeof(float) + (width + dsize.height)*sizeof(CvResizeAlpha);
            if( buf_size < CV_MAX_LOCAL_SIZE )
                buf0 = (float*)cvStackAlloc(buf_size);
            else
                CV_CALL( temp_buf = buf0 = (float*)cvAlloc(buf_size));
            buf1 = buf0 + width;
            xofs = (CvResizeAlpha*)(buf1 + width);
            yofs = xofs + width;

            for( dx = 0; dx < dsize.width; dx++ )
            {
                if( !area_mode )
                {
                    fx = (float)((dx+0.5)*scale_x - 0.5);
                    sx = cvFloor(fx);
                    fx -= sx;
                }
                else
                {
                    sx = cvFloor(dx*scale_x);
                    fx = (dx+1) - (sx+1)*inv_scale_x;
                    fx = fx <= 0 ? 0.f : fx - cvFloor(fx);
                }

                if( sx < 0 )
                    fx = 0, sx = 0;

                if( sx >= ssize.width-1 )
                {
                    fx = 0, sx = ssize.width-1;
                    if( xmax >= dsize.width )
                        xmax = dx;
                }

                if( depth != CV_8U )
                    for( k = 0, sx *= cn; k < cn; k++ )
                        xofs[dx*cn + k].idx = sx + k, xofs[dx*cn + k].alpha = fx;
                else
                    for( k = 0, sx *= cn; k < cn; k++ )
                        xofs[dx*cn + k].idx = sx + k,
                        xofs[dx*cn + k].ialpha = CV_FLT_TO_FIX(fx, ICV_WARP_SHIFT);
            }

            for( dy = 0; dy < dsize.height; dy++ )
            {
                if( !area_mode )
                {
                    fy = (float)((dy+0.5)*scale_y - 0.5);
                    sy = cvFloor(fy);
                    fy -= sy;
                    if( sy < 0 )
                        sy = 0, fy = 0;
                }
                else
                {
                    sy = cvFloor(dy*scale_y);
                    fy = (dy+1) - (sy+1)*inv_scale_y;
                    fy = fy <= 0 ? 0.f : fy - cvFloor(fy);
                }

                yofs[dy].idx = sy;
                if( depth != CV_8U )
                    yofs[dy].alpha = fy;
                else
                    yofs[dy].ialpha = CV_FLT_TO_FIX(fy, ICV_WARP_SHIFT);
            }

            IPPI_CALL( func( src->data.ptr, src->step, ssize, dst->data.ptr,
                             dst->step, dsize, cn, xmax, xofs, yofs, buf0, buf1 ));
        }
    }
    else
        CV_ERROR( CV_StsBadFlag, "Unknown/unsupported interpolation method" );

    __END__;

    cvFree( &temp_buf );
}


/****************************************************************************************\
*                                     WarpAffine                                         *
\****************************************************************************************/

#define ICV_DEF_WARP_AFFINE_BILINEAR_FUNC( flavor, arrtype, worktype,       \
            scale_alpha_macro, mul_one_macro, descale_macro, cast_macro )   \
static CvStatus CV_STDCALL                                                  \
icvWarpAffine_Bilinear_##flavor##_CnR(                                      \
    const arrtype* src, int step, CvSize ssize,                             \
    arrtype* dst, int dststep, CvSize dsize,                                \
    const double* matrix, int cn,                                           \
    const arrtype* fillval, const int* ofs )                                \
{                                                                           \
    int x, y, k;                                                            \
    double  A12 = matrix[1], b1 = matrix[2];                                \
    double  A22 = matrix[4], b2 = matrix[5];                                \
                                                                            \
    step /= sizeof(src[0]);                                                 \
    dststep /= sizeof(dst[0]);                                              \
                                                                            \
    for( y = 0; y < dsize.height; y++, dst += dststep )                     \
    {                                                                       \
        int xs = CV_FLT_TO_FIX( A12*y + b1, ICV_WARP_SHIFT );               \
        int ys = CV_FLT_TO_FIX( A22*y + b2, ICV_WARP_SHIFT );               \
                                                                            \
        for( x = 0; x < dsize.width; x++ )                                  \
        {                                                                   \
            int ixs = xs + ofs[x*2];                                        \
            int iys = ys + ofs[x*2+1];                                      \
            worktype a = scale_alpha_macro( ixs & ICV_WARP_MASK );          \
            worktype b = scale_alpha_macro( iys & ICV_WARP_MASK );          \
            worktype p0, p1;                                                \
            ixs >>= ICV_WARP_SHIFT;                                         \
            iys >>= ICV_WARP_SHIFT;                                         \
                                                                            \
            if( (unsigned)ixs < (unsigned)(ssize.width - 1) &&              \
                (unsigned)iys < (unsigned)(ssize.height - 1) )              \
            {                                                               \
                const arrtype* ptr = src + step*iys + ixs*cn;               \
                                                                            \
                for( k = 0; k < cn; k++ )                                   \
                {                                                           \
                    p0 = mul_one_macro(ptr[k]) +                            \
                        a * (ptr[k+cn] - ptr[k]);                           \
                    p1 = mul_one_macro(ptr[k+step]) +                       \
                        a * (ptr[k+cn+step] - ptr[k+step]);                 \
                    p0 = descale_macro(mul_one_macro(p0) + b*(p1 - p0));    \
                    dst[x*cn+k] = (arrtype)cast_macro(p0);                  \
                }                                                           \
            }                                                               \
            else if( (unsigned)(ixs+1) < (unsigned)(ssize.width+1) &&       \
                     (unsigned)(iys+1) < (unsigned)(ssize.height+1))        \
            {                                                               \
                int x0 = ICV_WARP_CLIP_X( ixs );                            \
                int y0 = ICV_WARP_CLIP_Y( iys );                            \
                int x1 = ICV_WARP_CLIP_X( ixs + 1 );                        \
                int y1 = ICV_WARP_CLIP_Y( iys + 1 );                        \
                const arrtype* ptr0, *ptr1, *ptr2, *ptr3;                   \
                                                                            \
                ptr0 = src + y0*step + x0*cn;                               \
                ptr1 = src + y0*step + x1*cn;                               \
                ptr2 = src + y1*step + x0*cn;                               \
                ptr3 = src + y1*step + x1*cn;                               \
                                                                            \
                for( k = 0; k < cn; k++ )                                   \
                {                                                           \
                    p0 = mul_one_macro(ptr0[k]) + a * (ptr1[k] - ptr0[k]);  \
                    p1 = mul_one_macro(ptr2[k]) + a * (ptr3[k] - ptr2[k]);  \
                    p0 = descale_macro( mul_one_macro(p0) + b*(p1 - p0) );  \
                    dst[x*cn+k] = (arrtype)cast_macro(p0);                  \
                }                                                           \
            }                                                               \
            else if( fillval )                                              \
                for( k = 0; k < cn; k++ )                                   \
                    dst[x*cn+k] = fillval[k];                               \
        }                                                                   \
    }                                                                       \
                                                                            \
    return CV_OK;                                                           \
}


#define ICV_WARP_SCALE_ALPHA(x) ((x)*(1./(ICV_WARP_MASK+1)))

ICV_DEF_WARP_AFFINE_BILINEAR_FUNC( 8u, uchar, int, CV_NOP, ICV_WARP_MUL_ONE_8U,
                                   ICV_WARP_DESCALE_8U, CV_NOP )
//ICV_DEF_WARP_AFFINE_BILINEAR_FUNC( 8u, uchar, double, ICV_WARP_SCALE_ALPHA, CV_NOP,
//                                   CV_NOP, ICV_WARP_CAST_8U )
ICV_DEF_WARP_AFFINE_BILINEAR_FUNC( 16u, ushort, double, ICV_WARP_SCALE_ALPHA, CV_NOP,
                                   CV_NOP, cvRound )
ICV_DEF_WARP_AFFINE_BILINEAR_FUNC( 32f, float, double, ICV_WARP_SCALE_ALPHA, CV_NOP,
                                   CV_NOP, CV_NOP )


typedef CvStatus (CV_STDCALL * CvWarpAffineFunc)(
    const void* src, int srcstep, CvSize ssize,
    void* dst, int dststep, CvSize dsize,
    const double* matrix, int cn,
    const void* fillval, const int* ofs );

static void icvInitWarpAffineTab( CvFuncTable* bilin_tab )
{
    bilin_tab->fn_2d[CV_8U] = (void*)icvWarpAffine_Bilinear_8u_CnR;
    bilin_tab->fn_2d[CV_16U] = (void*)icvWarpAffine_Bilinear_16u_CnR;
    bilin_tab->fn_2d[CV_32F] = (void*)icvWarpAffine_Bilinear_32f_CnR;
}

//////////////////////////////////////////////////////////////////////////////////////////

CV_IMPL void
cvWarpAffine( const CvArr* srcarr, CvArr* dstarr, const CvMat* matrix,
              int flags, CvScalar fillval )
{
    static CvFuncTable bilin_tab;
    static int inittab = 0;

    CV_FUNCNAME( "cvWarpAffine" );

    __BEGIN__;
    
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    int k, type, depth, cn, *ofs = 0;
    double src_matrix[6], dst_matrix[6];
    double fillbuf[4];
    int method = flags & 3;
    CvMat srcAb = cvMat( 2, 3, CV_64F, src_matrix ),
          dstAb = cvMat( 2, 3, CV_64F, dst_matrix ),
          A, b, invA, invAb;
    CvWarpAffineFunc func;
    CvSize ssize, dsize;
    
    if( !inittab )
    {
        icvInitWarpAffineTab( &bilin_tab );
        inittab = 1;
    }

    CV_CALL( src = cvGetMat( srcarr, &srcstub ));
    CV_CALL( dst = cvGetMat( dstarr, &dststub ));
    
    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( !CV_IS_MAT(matrix) || CV_MAT_CN(matrix->type) != 1 ||
        CV_MAT_DEPTH(matrix->type) < CV_32F || matrix->rows != 2 || matrix->cols != 3 )
        CV_ERROR( CV_StsBadArg,
        "Transformation matrix should be 2x3 floating-point single-channel matrix" );

    if( flags & CV_WARP_INVERSE_MAP )
        cvConvertScale( matrix, &dstAb );
    else
    {
        // [R|t] -> [R^-1 | -(R^-1)*t]
        cvConvertScale( matrix, &srcAb );
        cvGetCols( &srcAb, &A, 0, 2 );
        cvGetCol( &srcAb, &b, 2 );
        cvGetCols( &dstAb, &invA, 0, 2 );
        cvGetCol( &dstAb, &invAb, 2 );
        cvInvert( &A, &invA, CV_SVD );
        cvGEMM( &invA, &b, -1, 0, 0, &invAb );
    }

    type = CV_MAT_TYPE(src->type);
    depth = CV_MAT_DEPTH(type);
    cn = CV_MAT_CN(type);
    if( cn > 4 )
        CV_ERROR( CV_BadNumChannels, "" );

    ssize = cvGetMatSize(src);
    dsize = cvGetMatSize(dst);

    cvScalarToRawData( &fillval, fillbuf, CV_MAT_TYPE(src->type), 0 );
    ofs = (int*)cvStackAlloc( dst->cols*2*sizeof(ofs[0]) );
    for( k = 0; k < dst->cols; k++ )
    {
        ofs[2*k] = CV_FLT_TO_FIX( dst_matrix[0]*k, ICV_WARP_SHIFT );
        ofs[2*k+1] = CV_FLT_TO_FIX( dst_matrix[3]*k, ICV_WARP_SHIFT );
    }

    /*if( method == CV_INTER_LINEAR )*/
    {
        func = (CvWarpAffineFunc)bilin_tab.fn_2d[depth];
        if( !func )
            CV_ERROR( CV_StsUnsupportedFormat, "" );

        IPPI_CALL( func( src->data.ptr, src->step, ssize, dst->data.ptr,
                         dst->step, dsize, dst_matrix, cn,
                         flags & CV_WARP_FILL_OUTLIERS ? fillbuf : 0, ofs ));
    }

    __END__;
}


CV_IMPL CvMat*
cv2DRotationMatrix( CvPoint2D32f center, double angle,
                    double scale, CvMat* matrix )
{
    CV_FUNCNAME( "cvGetRotationMatrix" );

    __BEGIN__;

    double m[6];
    CvMat M = cvMat( 2, 3, CV_64FC1, m );
    double alpha, beta;

    if( !matrix )
        CV_ERROR( CV_StsNullPtr, "" );

    angle *= CV_PI/180;
    alpha = cos(angle)*scale;
    beta = sin(angle)*scale;

    m[0] = alpha;
    m[1] = beta;
    m[2] = (1-alpha)*center.x - beta*center.y;
    m[3] = -beta;
    m[4] = alpha;
    m[5] = beta*center.x + (1-alpha)*center.y;

    cvConvert( &M, matrix );

    __END__;

    return matrix;
}

/* End of file. */
