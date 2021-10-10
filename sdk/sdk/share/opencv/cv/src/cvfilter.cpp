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

#include "_cv.h"


/****************************************************************************************\
                                    Base Image Filter
\****************************************************************************************/

static void default_x_filter_func( const uchar*, uchar*, void* )
{
}

static void default_y_filter_func( uchar**, uchar*, int, int, void* )
{
}

CvBaseImageFilter::CvBaseImageFilter()
{
    min_depth = CV_8U;
    buffer = 0;
    rows = 0;
    max_width = 0;
    x_func = default_x_filter_func;
    y_func = default_y_filter_func;
}


CvBaseImageFilter::CvBaseImageFilter( int _max_width, int _src_type, int _dst_type,
                                      bool _is_separable, CvSize _ksize, CvPoint _anchor,
                                      int _border_mode, CvScalar _border_value )
{
    min_depth = CV_8U;
    buffer = 0;
    rows = 0;
    max_width = 0;
    x_func = default_x_filter_func;
    y_func = default_y_filter_func;

    init( _max_width, _src_type, _dst_type, _is_separable,
          _ksize, _anchor, _border_mode, _border_value );
}


void CvBaseImageFilter::clear()
{
    cvFree( &buffer );
    rows = 0;
}


CvBaseImageFilter::~CvBaseImageFilter()
{
    clear();
}


void CvBaseImageFilter::get_work_params()
{
    int min_rows = max_ky*2 + 3, rows = MAX(min_rows,10), row_sz;
    int width = max_width, trow_sz = 0;

    if( is_separable )
    {
        int max_depth = MAX(CV_MAT_DEPTH(src_type), CV_MAT_DEPTH(dst_type));
        int max_cn = MAX(CV_MAT_CN(src_type), CV_MAT_CN(dst_type));
        max_depth = MAX( max_depth, min_depth );
        work_type = CV_MAKETYPE( max_depth, max_cn );
        trow_sz = cvAlign( (max_width + ksize.width - 1)*CV_ELEM_SIZE(src_type), ALIGN );
    }
    else
    {
        work_type = src_type;
        width += ksize.width - 1;
    }
    row_sz = cvAlign( width*CV_ELEM_SIZE(work_type), ALIGN );
    buf_size = rows*row_sz;
    buf_size = MIN( buf_size, 1 << 16 );
    buf_size = MAX( buf_size, min_rows*row_sz );
    max_rows = (buf_size/row_sz)*3 + max_ky*2 + 8;
    buf_size += trow_sz;
}


void CvBaseImageFilter::init( int _max_width, int _src_type, int _dst_type,
                              bool _is_separable, CvSize _ksize, CvPoint _anchor,
                              int _border_mode, CvScalar _border_value )
{
    CV_FUNCNAME( "CvBaseImageFilter::init" );

    __BEGIN__;

    int total_buf_sz, src_pix_sz, row_tab_sz, bsz;
    uchar* ptr;

    if( !(buffer && _max_width <= max_width && _src_type == src_type &&
        _dst_type == dst_type && _is_separable == is_separable &&
        _ksize.width == ksize.width && _ksize.height == ksize.height &&
        _anchor.x == anchor.x && _anchor.y == anchor.y) )
        clear();

    is_separable = _is_separable != 0;
    max_width = _max_width; //MAX(_max_width,_ksize.width);
    src_type = CV_MAT_TYPE(_src_type);
    dst_type = CV_MAT_TYPE(_dst_type);
    ksize = _ksize;
    anchor = _anchor;

    if( anchor.x == -1 )
        anchor.x = ksize.width / 2;
    if( anchor.y == -1 )
        anchor.y = ksize.height / 2;

    max_ky = MAX( anchor.y, ksize.height - anchor.y - 1 ); 
    border_mode = _border_mode;
    border_value = _border_value;

    if( ksize.width <= 0 || ksize.height <= 0 ||
        (unsigned)anchor.x >= (unsigned)ksize.width ||
        (unsigned)anchor.y >= (unsigned)ksize.height )
        CV_ERROR( CV_StsOutOfRange, "invalid kernel size and/or anchor position" );

    if( border_mode != IPL_BORDER_CONSTANT && border_mode != IPL_BORDER_REPLICATE &&
        border_mode != IPL_BORDER_REFLECT && border_mode != IPL_BORDER_REFLECT_101 )
        CV_ERROR( CV_StsBadArg, "Invalid/unsupported border mode" );

    get_work_params();

    prev_width = 0;
    prev_x_range = cvSlice(0,0);

    buf_size = cvAlign( buf_size, ALIGN );

    src_pix_sz = CV_ELEM_SIZE(src_type);
    border_tab_sz1 = anchor.x*src_pix_sz;
    border_tab_sz = (ksize.width-1)*src_pix_sz;
    bsz = cvAlign( border_tab_sz*sizeof(int), ALIGN );

    assert( max_rows > max_ky*2 );
    row_tab_sz = cvAlign( max_rows*sizeof(uchar*), ALIGN );
    total_buf_sz = buf_size + row_tab_sz + bsz;
    
    CV_CALL( ptr = buffer = (uchar*)cvAlloc( total_buf_sz ));
    
    rows = (uchar**)ptr;
    ptr += row_tab_sz;
    border_tab = (int*)ptr;
    ptr += bsz;

    buf_start = ptr;
    const_row = 0;

    if( border_mode == IPL_BORDER_CONSTANT )
        cvScalarToRawData( &border_value, border_tab, src_type, 0 );

    __END__;
}


void CvBaseImageFilter::start_process( CvSlice x_range, int width )
{
    int mode = border_mode;
    int pix_sz = CV_ELEM_SIZE(src_type), work_pix_sz = CV_ELEM_SIZE(work_type);
    int bsz = buf_size, bw = x_range.end_index - x_range.start_index, bw1 = bw + ksize.width - 1;
    int tr_step = cvAlign(bw1*pix_sz, ALIGN );
    int i, j, k, ofs;
    
    if( x_range.start_index == prev_x_range.start_index &&
        x_range.end_index == prev_x_range.end_index &&
        width == prev_width )
        return;

    prev_x_range = x_range;
    prev_width = width;

    if( !is_separable )
        bw = bw1;
    else
        bsz -= tr_step;

    buf_step = cvAlign(bw*work_pix_sz, ALIGN);

    if( mode == IPL_BORDER_CONSTANT )
        bsz -= buf_step;
    buf_max_count = bsz/buf_step;
    buf_max_count = MIN( buf_max_count, max_rows - max_ky*2 );
    buf_end = buf_start + buf_max_count*buf_step;

    if( mode == IPL_BORDER_CONSTANT )
    {
        int i, tab_len = ksize.width*pix_sz;
        uchar* bt = (uchar*)border_tab;
        uchar* trow = buf_end;
        const_row = buf_end + (is_separable ? 1 : 0)*tr_step;

        for( i = pix_sz; i < tab_len; i++ )
            bt[i] = bt[i - pix_sz];
        for( i = 0; i < pix_sz; i++ )
            trow[i] = bt[i];
        for( i = pix_sz; i < tr_step; i++ )
            trow[i] = trow[i - pix_sz];
        if( is_separable )
            x_func( trow, const_row, this );
        return;
    }

    if( x_range.end_index - x_range.start_index <= 1 )
        mode = IPL_BORDER_REPLICATE;

    width = (width - 1)*pix_sz;
    ofs = (anchor.x-x_range.start_index)*pix_sz;

    for( k = 0; k < 2; k++ )
    {
        int idx, delta;
        int i1, i2, di;

        if( k == 0 )
        {
            idx = (x_range.start_index - 1)*pix_sz;
            delta = di = -pix_sz;
            i1 = border_tab_sz1 - pix_sz;
            i2 = -pix_sz;
        }
        else
        {
            idx = x_range.end_index*pix_sz;
            delta = di = pix_sz;
            i1 = border_tab_sz1;
            i2 = border_tab_sz;
        }

        if( (unsigned)idx > (unsigned)width )
        {
            int shift = mode == IPL_BORDER_REFLECT_101 ? pix_sz : 0;
            idx = k == 0 ? shift : width - shift;
            delta = -delta;
        }

        for( i = i1; i != i2; i += di )
        {
            for( j = 0; j < pix_sz; j++ )
                border_tab[i + j] = idx + ofs + j;
            if( mode != IPL_BORDER_REPLICATE )
            {
                if( delta > 0 && idx == width ||
                    delta < 0 && idx == 0 )
                {
                    if( mode == IPL_BORDER_REFLECT_101 )
                        idx -= delta*2;
                    delta = -delta;
                }
                else
                    idx += delta;
            }
        }
    }
}


void CvBaseImageFilter::make_y_border( int row_count, int top_rows, int bottom_rows )
{
    int i;
    
    if( border_mode == IPL_BORDER_CONSTANT ||
        border_mode == IPL_BORDER_REPLICATE )
    {
        uchar* row1 = border_mode == IPL_BORDER_CONSTANT ? const_row : rows[max_ky];
        
        for( i = 0; i < top_rows && rows[i] == 0; i++ )
            rows[i] = row1;

        row1 = border_mode == IPL_BORDER_CONSTANT ? const_row : rows[row_count-1];
        for( i = 0; i < bottom_rows; i++ )
            rows[i + row_count] = row1;
    }
    else
    {
        int j, dj = 1, shift = border_mode == IPL_BORDER_REFLECT_101;

        for( i = top_rows-1, j = top_rows+shift; i >= 0; i-- )
        {
            if( rows[i] == 0 )
                rows[i] = rows[j];
            j += dj;
            if( dj > 0 && j >= row_count )
            {
                if( !bottom_rows )
                    break;
                j -= 1 + shift;
                dj = -dj;
            }
        }

        for( i = 0, j = row_count-1-shift; i < bottom_rows; i++, j-- )
            rows[i + row_count] = rows[j];
    }
}


int CvBaseImageFilter::fill_cyclic_buffer( const uchar* src, int src_step,
                                           int y0, int y1, int y2 )
{
    int i, y = y0, bsz1 = border_tab_sz1, bsz = border_tab_sz;
    int pix_size = CV_ELEM_SIZE(src_type);
    int width = prev_x_range.end_index - prev_x_range.start_index, width_n = width*pix_size;
    bool can_use_src_as_trow = is_separable && width >= ksize.width;

    // fill the cyclic buffer
    for( ; buf_count < buf_max_count && y < y2; buf_count++, y++, src += src_step )
    {
        uchar* trow = is_separable ? buf_end : buf_tail;
        uchar* bptr = can_use_src_as_trow && y1 < y && y+1 < y2 ? (uchar*)(src - bsz1) : trow;

        if( bptr != trow )
        {
            for( i = 0; i < bsz1; i++ )
                trow[i] = bptr[i];
            for( ; i < bsz; i++ )
                trow[i] = bptr[i + width_n];
        }
        else if( !(((size_t)(bptr + bsz1)|(size_t)src|width_n) & (sizeof(int)-1)) )
            for( i = 0; i < width_n; i += sizeof(int) )
                *(int*)(bptr + i + bsz1) = *(int*)(src + i);
        else
            for( i = 0; i < width_n; i++ )
                bptr[i + bsz1] = src[i];

        if( border_mode != IPL_BORDER_CONSTANT )
        {
            for( i = 0; i < bsz1; i++ )
            {
                int j = border_tab[i];
                bptr[i] = bptr[j];
            }
            for( ; i < bsz; i++ )
            {
                int j = border_tab[i];
                bptr[i + width_n] = bptr[j];
            }
        }
        else
        {
            const uchar *bt = (uchar*)border_tab; 
            for( i = 0; i < bsz1; i++ )
                bptr[i] = bt[i];

            for( ; i < bsz; i++ )
                bptr[i + width_n] = bt[i];
        }

        if( is_separable )
        {
            x_func( bptr, buf_tail, this );
            if( bptr != trow )
            {
                for( i = 0; i < bsz1; i++ )
                    bptr[i] = trow[i];
                for( ; i < bsz; i++ )
                    bptr[i + width_n] = trow[i];
            }
        }

        buf_tail += buf_step;
        if( buf_tail >= buf_end )
            buf_tail = buf_start;
    }

    return y - y0;
}

int CvBaseImageFilter::process( const CvMat* src, CvMat* dst,
                                CvRect src_roi, CvPoint dst_origin, int flags )
{
    int rows_processed = 0;

    /*
        check_parameters
        initialize_horizontal_border_reloc_tab_if_not_initialized_yet
        
        for_each_source_row: src starts from src_roi.y, buf starts with the first available row
            1) if separable,
                   1a.1) copy source row to temporary buffer, form a border using border reloc tab.
                   1a.2) apply row-wise filter (symmetric, asymmetric or generic)
               else
                   1b.1) copy source row to the buffer, form a border
            2) if the buffer is full, or it is the last source row:
                 2.1) if stage != middle, form the pointers to other "virtual" rows.
                 if separable
                    2a.2) apply column-wise filter, store the results.
                 else
                    2b.2) form a sparse (offset,weight) tab
                    2b.3) apply generic non-separable filter, store the results
            3) update row pointers etc.
    */

    CV_FUNCNAME( "CvBaseImageFilter::process" );

    __BEGIN__;

    int i, width, _src_y1, _src_y2;
    int src_x, src_y, src_y1, src_y2, dst_y;
    int pix_size = CV_ELEM_SIZE(src_type);
    uchar *sptr = 0, *dptr;
    int phase = flags & (CV_START|CV_END|CV_MIDDLE);
    bool isolated_roi = (flags & CV_ISOLATED_ROI) != 0;

    if( !CV_IS_MAT(src) )
        CV_ERROR( CV_StsBadArg, "" );

    if( CV_MAT_TYPE(src->type) != src_type )
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    width = src->cols;

    if( src_roi.width == -1 && src_roi.x == 0 )
        src_roi.width = width;

    if( src_roi.height == -1 && src_roi.y == 0 )
    {
        src_roi.y = 0;
        src_roi.height = src->rows;
    }

    if( src_roi.width > max_width ||
        src_roi.x < 0 || src_roi.width < 0 ||
        src_roi.y < 0 || src_roi.height < 0 ||
        src_roi.x + src_roi.width > width ||
        src_roi.y + src_roi.height > src->rows )
        CV_ERROR( CV_StsOutOfRange, "Too large source image or its ROI" );

    src_x = src_roi.x;
    _src_y1 = 0;
    _src_y2 = src->rows;

    if( isolated_roi )
    {
        src_roi.x = 0;
        width = src_roi.width;
        _src_y1 = src_roi.y;
        _src_y2 = src_roi.y + src_roi.height;
    }

    if( !CV_IS_MAT(dst) )
        CV_ERROR( CV_StsBadArg, "" );

    if( CV_MAT_TYPE(dst->type) != dst_type )
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( dst_origin.x < 0 || dst_origin.y < 0 )
        CV_ERROR( CV_StsOutOfRange, "Incorrect destination ROI origin" );

    if( phase == CV_WHOLE )
        phase = CV_START | CV_END;
    phase &= CV_START | CV_END | CV_MIDDLE;

    // initialize horizontal border relocation tab if it is not initialized yet
    if( phase & CV_START )
        start_process( cvSlice(src_roi.x, src_roi.x + src_roi.width), width );
    else if( prev_width != width || prev_x_range.start_index != src_roi.x ||
        prev_x_range.end_index != src_roi.x + src_roi.width )
        CV_ERROR( CV_StsBadArg,
            "In a middle or at the end the horizontal placement of the stripe can not be changed" );

    dst_y = dst_origin.y;
    src_y1 = src_roi.y;
    src_y2 = src_roi.y + src_roi.height;

    if( phase & CV_START )
    {
        for( i = 0; i <= max_ky*2; i++ )
            rows[i] = 0;
        
        src_y1 -= max_ky;
        top_rows = bottom_rows = 0;

        if( src_y1 < _src_y1 )
        {
            top_rows = _src_y1 - src_y1;
            src_y1 = _src_y1;
        }

        buf_head = buf_tail = buf_start;
        buf_count = 0;
    }

    if( phase & CV_END )
    {
        src_y2 += max_ky;

        if( src_y2 > _src_y2 )
        {
            bottom_rows = src_y2 - _src_y2;
            src_y2 = _src_y2;
        }
    }
    
    dptr = dst->data.ptr + dst_origin.y*dst->step + dst_origin.x*CV_ELEM_SIZE(dst_type);
    sptr = src->data.ptr + src_y1*src->step + src_x*pix_size;
        
    for( src_y = src_y1; src_y < src_y2; )
    {
        uchar* bptr;
        int row_count, delta;

        delta = fill_cyclic_buffer( sptr, src->step, src_y, src_y1, src_y2 );

        src_y += delta;
        sptr += src->step*delta;

        // initialize the cyclic buffer row pointers
        bptr = buf_head;
        for( i = 0; i < buf_count; i++ )
        {
            rows[i+top_rows] = bptr;
            bptr += buf_step;
            if( bptr >= buf_end )
                bptr = buf_start;
        }

        row_count = top_rows + buf_count;
        
        if( !rows[0] || (phase & CV_END) && src_y == src_y2 )
        {
            int br = (phase & CV_END) && src_y == src_y2 ? bottom_rows : 0;
            make_y_border( row_count, top_rows, br );
            row_count += br;
        }

        if( rows[0] && row_count > max_ky*2 )
        {
            int count = row_count - max_ky*2;
            if( dst_y + count > dst->rows )
                CV_ERROR( CV_StsOutOfRange, "The destination image can not fit the result" );

            assert( count >= 0 );
            y_func( rows + max_ky - anchor.y, dptr, dst->step, count, this );
            row_count -= count;
            dst_y += count;
            dptr += dst->step*count;
            for( bptr = row_count > 0 ?rows[count] : 0; buf_head != bptr && buf_count > 0; buf_count-- )
            {
                buf_head += buf_step;
                if( buf_head >= buf_end )
                    buf_head = buf_start;
            }
            rows_processed += count;
            top_rows = MAX(top_rows - count, 0);
        }
    }

    __END__;

    return rows_processed;
}


/****************************************************************************************\
                              Non-separable Linear Filter
\****************************************************************************************/

static void icvLinearFilter_8u( const uchar** src, uchar* dst, int dst_step,
                                int count, void* params );
static void icvLinearFilter_16s( const short** src, short* dst, int dst_step,
                                 int count, void* params );
static void icvLinearFilter_16u( const ushort** src, ushort* dst, int dst_step,
                                 int count, void* params );

CvLinearFilter::CvLinearFilter()
{
    kernel = 0;
    k_sparse = 0;
}

CvLinearFilter::CvLinearFilter( int _max_width, int _src_type, int _dst_type,
                                const CvMat* _kernel, CvPoint _anchor,
                                int _border_mode, CvScalar _border_value )
{
    kernel = 0;
    k_sparse = 0;
    init( _max_width, _src_type, _dst_type, _kernel,
          _anchor, _border_mode, _border_value );
}


void CvLinearFilter::clear()
{
    cvReleaseMat( &kernel );
    cvFree( &k_sparse );
    CvBaseImageFilter::clear();
}


CvLinearFilter::~CvLinearFilter()
{
    clear();
}


void CvLinearFilter::init( int _max_width, int _src_type, int _dst_type,
                           const CvMat* _kernel, CvPoint _anchor,
                           int _border_mode, CvScalar _border_value )
{
    CV_FUNCNAME( "CvLinearFilter::init" );

    __BEGIN__;

    int depth = CV_MAT_DEPTH(_src_type);
    int cn = CV_MAT_CN(_src_type);
    CvPoint* nz_loc;
    char* coeffs;
    int i, j, k = 0;

    if( !CV_IS_MAT(_kernel) )
        CV_ERROR( CV_StsBadArg, "kernel is not valid matrix" );

    _src_type = CV_MAT_TYPE(_src_type);
    _dst_type = CV_MAT_TYPE(_dst_type);

    if( _src_type != _dst_type )
        CV_ERROR( CV_StsUnmatchedFormats,
        "The source and destination image types must be the same" );

    CV_CALL( CvBaseImageFilter::init( _max_width, _src_type, _dst_type,
        false, cvGetMatSize(_kernel), _anchor, _border_mode, _border_value ));

    if( !(kernel && k_sparse && ksize.width == kernel->cols && ksize.height == kernel->rows ))
    {
        cvReleaseMat( &kernel );
        cvFree( &k_sparse );
        CV_CALL( kernel = cvCreateMat( ksize.height, ksize.width, CV_8SC1 ));
        CV_CALL( k_sparse = (uchar*)cvAlloc(
            ksize.width*ksize.height*(2*sizeof(int) + sizeof(uchar*) + sizeof(char))));
    }
  
    CV_CALL( cvConvert( _kernel, kernel ));
    
    nz_loc = (CvPoint*)k_sparse;
    for( i = 0; i < ksize.height; i++ )
    {
        for( j = 0; j < ksize.width; j++ )
            if( abs(((char*)(kernel->data.ptr + i*kernel->step))[j])>FLT_EPSILON )
                nz_loc[k++] = cvPoint(j,i);
    }
    if( k == 0 )
        nz_loc[k++] = anchor;
    k_sparse_count = k;
    coeffs = (char*)((uchar**)(nz_loc + k_sparse_count) + k_sparse_count);

    for( k = 0; k < k_sparse_count; k++ )
    {
        coeffs[k] = CV_MAT_ELEM( *kernel, char, nz_loc[k].y, nz_loc[k].x );
        nz_loc[k].x *= cn;
    }

    x_func = 0;
    if( depth == CV_8U )
        y_func = (CvColumnFilterFunc)icvLinearFilter_8u;
    else if( depth == CV_16S )
        y_func = (CvColumnFilterFunc)icvLinearFilter_16s;
    else if( depth == CV_16U )
        y_func = (CvColumnFilterFunc)icvLinearFilter_16u;
    else
        CV_ERROR( CV_StsUnsupportedFormat, "Unsupported image type" );

    __END__;
}


void CvLinearFilter::init( int _max_width, int _src_type, int _dst_type,
                           bool _is_separable, CvSize _ksize,
                           CvPoint _anchor, int _border_mode,
                           CvScalar _border_value )
{
    CvBaseImageFilter::init( _max_width, _src_type, _dst_type, _is_separable,
                             _ksize, _anchor, _border_mode, _border_value );
}


#define ICV_FILTER( flavor, arrtype, worktype, load_macro,          \
                    cast_macro1, cast_macro2 )                      \
static void                                                         \
icvLinearFilter_##flavor( const arrtype** src, arrtype* dst,        \
                    int dst_step, int count, void* params )         \
{                                                                   \
    CvLinearFilter* state = (CvLinearFilter*)params;                \
    int width = state->get_width();                                 \
    int cn = CV_MAT_CN(state->get_src_type());                      \
    int i, k;                                                       \
    CvPoint* k_sparse = (CvPoint*)state->get_kernel_sparse_buf();   \
    int k_count = state->get_kernel_sparse_count();                 \
    const arrtype** k_ptr = (const arrtype**)(k_sparse + k_count);  \
    const arrtype** k_end = k_ptr + k_count;                        \
    const char* k_coeffs = (const char*)(k_ptr + k_count);        \
                                                                    \
    width *= cn;                                                    \
    dst_step /= sizeof(dst[0]);                                     \
                                                                    \
    for( ; count > 0; count--, dst += dst_step, src++ )             \
    {                                                               \
        for( k = 0; k < k_count; k++ )                              \
            k_ptr[k] = src[k_sparse[k].y] + k_sparse[k].x;          \
                                                                    \
        for( i = 0; i <= width - 4; i += 4 )                        \
        {                                                           \
            const arrtype** kp = k_ptr;                             \
            const char* kc = k_coeffs;                             \
            int s0 = 0, s1 = 0, s2 = 0, s3 = 0;                  \
            worktype t0, t1;                                        \
                                                                    \
            while( kp != k_end )                                    \
            {                                                       \
                const arrtype* sptr = (*kp++) + i;                  \
                char f = *kc++;                                    \
                s0 += f*load_macro(sptr[0]);                        \
                s1 += f*load_macro(sptr[1]);                        \
                s2 += f*load_macro(sptr[2]);                        \
                s3 += f*load_macro(sptr[3]);                        \
            }                                                       \
                                                                    \
            t0 = cast_macro1(s0); t1 = cast_macro1(s1);             \
            dst[i] = cast_macro2(t0);                               \
            dst[i+1] = cast_macro2(t1);                             \
            t0 = cast_macro1(s2); t1 = cast_macro1(s3);             \
            dst[i+2] = cast_macro2(t0);                             \
            dst[i+3] = cast_macro2(t1);                             \
        }                                                           \
                                                                    \
        for( ; i < width; i++ )                                     \
        {                                                           \
            const arrtype** kp = k_ptr;                             \
            const char* kc = k_coeffs;                             \
            int s0 = 0;                                          \
            worktype t0;                                            \
                                                                    \
            while( kp != k_end )                                    \
            {                                                       \
                const arrtype* sptr = *kp++;                        \
                char f = *kc++;                                    \
                s0 += f*load_macro(sptr[i]);                        \
            }                                                       \
                                                                    \
            t0 = cast_macro1(s0);                                   \
            dst[i] = cast_macro2(t0);                               \
        }                                                           \
    }                                                               \
}


ICV_FILTER( 8u, uchar, int, CV_NOP, CV_NOP, CV_CAST_8U )
ICV_FILTER( 16u, ushort, int, CV_NOP, CV_NOP, CV_CAST_16U )
ICV_FILTER( 16s, short, int, CV_NOP, CV_NOP, CV_CAST_16S )

CV_IMPL void
cvFilter2D( const CvArr* _src, CvArr* _dst, const CvMat* kernel, CvPoint anchor )
{
    const int dft_filter_size = 100;

    CvLinearFilter filter;
    CvMat* temp = 0;

    CV_FUNCNAME( "cvFilter2D" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0;
    CvMat srcstub, *src = (CvMat*)_src;
    CvMat dststub, *dst = (CvMat*)_dst;
    int type;

    CV_CALL( src = cvGetMat( src, &srcstub, &coi1 ));
    CV_CALL( dst = cvGetMat( dst, &dststub, &coi2 ));

    if( coi1 != 0 || coi2 != 0 )
        CV_ERROR( CV_BadCOI, "" );

    type = CV_MAT_TYPE( src->type );

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( !CV_ARE_TYPES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    CV_CALL( filter.init( src->cols, type, type, kernel, anchor ));
    CV_CALL( filter.process( src, dst ));

    __END__;

    cvReleaseMat( &temp );
}


/* End of file. */
