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

/* Haar features calculation */

#include "_cv.h"
#include <stdio.h>
//#include "haarcascade_frontface_alt.h"

/* these settings affect the quality of detection: change with care */
#define CV_ADJUST_FEATURES 1
#define CV_ADJUST_WEIGHTS  0

typedef int sumtype;
typedef int sqsumtype;

typedef struct CvHidHaarFeature
{
    struct
    {
        sumtype *p0, *p1, *p2, *p3;
        int weight;   // s15.16
    }
    rect[CV_HAAR_FEATURE_MAX];
}
CvHidHaarFeature;


typedef struct CvHidHaarTreeNode
{
    CvHidHaarFeature feature;
    int threshold;  // s15.16
    int left;
    int right;
}
CvHidHaarTreeNode;


typedef struct CvHidHaarClassifier
{
    int count;
    //CvHaarFeature* orig_feature;
    CvHidHaarTreeNode* node;
    int* alpha;  // s15.16
}
CvHidHaarClassifier;


typedef struct CvHidHaarStageClassifier
{
    int  count;
    int threshold;  // s15.16
    CvHidHaarClassifier* classifier;
    int two_rects;
    
    struct CvHidHaarStageClassifier* next;
    struct CvHidHaarStageClassifier* child;
    struct CvHidHaarStageClassifier* parent;
}
CvHidHaarStageClassifier;


struct CvHidHaarClassifierCascade
{
    int  count;
    int  is_stump_based;
    int  has_tilted_features;
    int  is_tree;
    int inv_window_area;  // s15.16
    CvMat sum, sqsum, tilted;
    CvHidHaarStageClassifier* stage_classifier;
    sqsumtype *pq0, *pq1, *pq2, *pq3;
    sumtype *p0, *p1, *p2, *p3;
};

const int icv_object_win_border = 1;
const int icv_stage_threshold_bias = (int)(0.0001f*65536);  // s15.16

static CvHaarClassifierCascade*
icvCreateHaarClassifierCascade( int stage_count )
{
    CvHaarClassifierCascade* cascade = 0;
    
    CV_FUNCNAME( "icvCreateHaarClassifierCascade" );

    __BEGIN__;

    int block_size = sizeof(*cascade) + stage_count*sizeof(*cascade->stage_classifier);

    if( stage_count <= 0 )
        CV_ERROR( CV_StsOutOfRange, "Number of stages should be positive" );

    CV_CALL( cascade = (CvHaarClassifierCascade*)cvAlloc( block_size ));
    memset( cascade, 0, block_size );

    cascade->stage_classifier = (CvHaarStageClassifier*)(cascade + 1);
    cascade->flags = CV_HAAR_MAGIC_VAL;
    cascade->count = stage_count;

    __END__;

    return cascade;
}

static void
icvReleaseHidHaarClassifierCascade( CvHidHaarClassifierCascade** _cascade )
{
    if( _cascade && *_cascade )
    {
        CvHidHaarClassifierCascade* cascade = *_cascade;
        cvFree( _cascade );
    }
}

/* create more efficient internal representation of haar classifier cascade */
static CvHidHaarClassifierCascade*
icvCreateHidHaarClassifierCascade( CvHaarClassifierCascade* cascade )
{
    CvHidHaarClassifierCascade* out = 0;

    CV_FUNCNAME( "icvCreateHidHaarClassifierCascade" );

    __BEGIN__;

    int i, j, k, l;
    int datasize;
    int total_classifiers = 0;
    int total_nodes = 0;
    char errorstr[100];
    CvHidHaarClassifier* haar_classifier_ptr;
    CvHidHaarTreeNode* haar_node_ptr;
    CvSize orig_window_size;
    int has_tilted_features = 0;
    int max_count = 0;

    if( !CV_IS_HAAR_CLASSIFIER(cascade) )
        CV_ERROR( !cascade ? CV_StsNullPtr : CV_StsBadArg, "Invalid classifier pointer" );

    if( cascade->hid_cascade )
        CV_ERROR( CV_StsError, "hid_cascade has been already created" );

    if( !cascade->stage_classifier )
        CV_ERROR( CV_StsNullPtr, "" );

    if( cascade->count <= 0 )
        CV_ERROR( CV_StsOutOfRange, "Negative number of cascade stages" );

    orig_window_size = cascade->orig_window_size;
    
    /* check input structure correctness and calculate total memory size needed for
       internal representation of the classifier cascade */
    for( i = 0; i < cascade->count; i++ )
    {
        CvHaarStageClassifier* stage_classifier = cascade->stage_classifier + i;

        if( !stage_classifier->classifier ||
            stage_classifier->count <= 0 )
        {
            sprintf( errorstr, "header of the stage classifier #%d is invalid "
                     "(has null pointers or non-positive classfier count)", i );
            CV_ERROR( CV_StsError, errorstr );
        }

        max_count = MAX( max_count, stage_classifier->count );
        total_classifiers += stage_classifier->count;

        for( j = 0; j < stage_classifier->count; j++ )
        {
            CvHaarClassifier* classifier = stage_classifier->classifier + j;
            total_nodes += classifier->count;
        }
    }

    // this is an upper boundary for the whole hidden cascade size
    datasize = sizeof(CvHidHaarClassifierCascade) +
               sizeof(CvHidHaarStageClassifier)*cascade->count +
               sizeof(CvHidHaarClassifier) * total_classifiers +
               sizeof(CvHidHaarTreeNode) * total_nodes +
               sizeof(void*)*(total_nodes + total_classifiers);

    CV_CALL( out = (CvHidHaarClassifierCascade*)cvAlloc( datasize ));
    memset( out, 0, sizeof(*out) );

    /* init header */
    out->count = cascade->count;
    out->stage_classifier = (CvHidHaarStageClassifier*)(out + 1);
    haar_classifier_ptr = (CvHidHaarClassifier*)(out->stage_classifier + cascade->count);
    haar_node_ptr = (CvHidHaarTreeNode*)(haar_classifier_ptr + total_classifiers);

    out->is_stump_based = 1;
    out->has_tilted_features = has_tilted_features;
    out->is_tree = 0;

    /* initialize internal representation */
    for( i = 0; i < cascade->count; i++ )
    {
        CvHaarStageClassifier* stage_classifier = cascade->stage_classifier + i;
        CvHidHaarStageClassifier* hid_stage_classifier = out->stage_classifier + i;

        hid_stage_classifier->count = stage_classifier->count;
        hid_stage_classifier->threshold = stage_classifier->threshold - icv_stage_threshold_bias;
        hid_stage_classifier->classifier = haar_classifier_ptr;
        hid_stage_classifier->two_rects = 1;
        haar_classifier_ptr += stage_classifier->count;

        hid_stage_classifier->parent = (stage_classifier->parent == -1)
            ? NULL : out->stage_classifier + stage_classifier->parent;
        hid_stage_classifier->next = (stage_classifier->next == -1)
            ? NULL : out->stage_classifier + stage_classifier->next;
        hid_stage_classifier->child = (stage_classifier->child == -1)
            ? NULL : out->stage_classifier + stage_classifier->child;
        
        out->is_tree |= hid_stage_classifier->next != NULL;

        for( j = 0; j < stage_classifier->count; j++ )
        {
            CvHaarClassifier* classifier = stage_classifier->classifier + j;
            CvHidHaarClassifier* hid_classifier = hid_stage_classifier->classifier + j;
            int node_count = classifier->count;
            int* alpha_ptr = (int*)(haar_node_ptr + node_count);

            hid_classifier->count = node_count;
            hid_classifier->node = haar_node_ptr;
            hid_classifier->alpha = alpha_ptr;
            
            for( l = 0; l < node_count; l++ )
            {
                CvHidHaarTreeNode* node = hid_classifier->node + l;
                CvHaarFeature* feature = classifier->haar_feature + l;
                memset( node, -1, sizeof(*node) );
                node->threshold = classifier->threshold[l];
                node->left = classifier->left[l];
                node->right = classifier->right[l];

                if( /*fabs(feature->rect[2].weight) < DBL_EPSILON ||*/
                    feature->rect[2].r.width == 0 ||
                    feature->rect[2].r.height == 0 )
                    memset( &(node->feature.rect[2]), 0, sizeof(node->feature.rect[2]) );
                else
                    hid_stage_classifier->two_rects = 0;
            }

            memcpy( alpha_ptr, classifier->alpha, (node_count+1)*sizeof(alpha_ptr[0]));
            haar_node_ptr =
                (CvHidHaarTreeNode*)cvAlignPtr(alpha_ptr+node_count+1, sizeof(void*));

            out->is_stump_based &= node_count == 1;
        }
    }

    cascade->hid_cascade = out;
    assert( (char*)haar_node_ptr - (char*)out <= datasize );

    __END__;

    if( cvGetErrStatus() < 0 )
        icvReleaseHidHaarClassifierCascade( &out );

    return out;
}


#define sum_elem_ptr(sum,row,col)  \
    ((sumtype*)CV_MAT_ELEM_PTR_FAST((sum),(row),(col),sizeof(sumtype)))

#define sqsum_elem_ptr(sqsum,row,col)  \
    ((sqsumtype*)CV_MAT_ELEM_PTR_FAST((sqsum),(row),(col),sizeof(sqsumtype)))

#define calc_sum(rect,offset) \
    ((rect).p0[offset] - (rect).p1[offset] - (rect).p2[offset] + (rect).p3[offset])


CV_IMPL void
cvSetImagesForHaarClassifierCascade( CvHaarClassifierCascade* _cascade,
                                     const CvArr* _sum,
                                     const CvArr* _sqsum,
                                     const CvArr* _tilted_sum)
{
    CV_FUNCNAME("cvSetImagesForHaarClassifierCascade");

    __BEGIN__;

    CvMat sum_stub, *sum = (CvMat*)_sum;
    CvMat sqsum_stub, *sqsum = (CvMat*)_sqsum;
    CvMat tilted_stub, *tilted = (CvMat*)_tilted_sum;
    CvHidHaarClassifierCascade* cascade;
    int coi0 = 0, coi1 = 0;
    int i;
    CvRect equ_rect;
    int weight_scale;  // s15.16

    if( !CV_IS_HAAR_CLASSIFIER(_cascade) )
        CV_ERROR( !_cascade ? CV_StsNullPtr : CV_StsBadArg, "Invalid classifier pointer" );

    CV_CALL( sum = cvGetMat( sum, &sum_stub, &coi0 ));
    CV_CALL( sqsum = cvGetMat( sqsum, &sqsum_stub, &coi1 ));

    if( coi0 || coi1 )
        CV_ERROR( CV_BadCOI, "COI is not supported" );

    if( !CV_ARE_SIZES_EQ( sum, sqsum ))
        CV_ERROR( CV_StsUnmatchedSizes, "All integral images must have the same size" );

    if( CV_MAT_TYPE(sqsum->type) != CV_32SC1 ||
        CV_MAT_TYPE(sum->type) != CV_32SC1 )
        CV_ERROR( CV_StsUnsupportedFormat,
        "Only (32s, 32s, 32s) combination of (sum,sqsum,tilted_sum) formats is allowed" );

    if( !_cascade->hid_cascade )
        CV_CALL( icvCreateHidHaarClassifierCascade(_cascade) );

    cascade = _cascade->hid_cascade;

    if( cascade->has_tilted_features )
    {
        CV_CALL( tilted = cvGetMat( tilted, &tilted_stub, &coi1 ));

        if( CV_MAT_TYPE(tilted->type) != CV_32SC1 )
            CV_ERROR( CV_StsUnsupportedFormat,
            "Only (32s, 32s, 32s) combination of (sum,sqsum,tilted_sum) formats is allowed" );

        if( sum->step != tilted->step )
            CV_ERROR( CV_StsUnmatchedSizes,
            "Sum and tilted_sum must have the same stride (step, widthStep)" );

        if( !CV_ARE_SIZES_EQ( sum, tilted ))
            CV_ERROR( CV_StsUnmatchedSizes, "All integral images must have the same size" );
        cascade->tilted = *tilted;
    }
    
    //_cascade->scale = scale;
    _cascade->real_window_size.width =  _cascade->orig_window_size.width;
    _cascade->real_window_size.height =  _cascade->orig_window_size.height;

    cascade->sum = *sum;
    cascade->sqsum = *sqsum;
    
    equ_rect.x = equ_rect.y = 1;
    equ_rect.width = (_cascade->orig_window_size.width-2);
    equ_rect.height = (_cascade->orig_window_size.height-2);
    weight_scale = 65536/(equ_rect.width*equ_rect.height);  // s15.16
    cascade->inv_window_area = weight_scale;

    cascade->p0 = sum_elem_ptr(*sum, equ_rect.y, equ_rect.x);
    cascade->p1 = sum_elem_ptr(*sum, equ_rect.y, equ_rect.x + equ_rect.width );
    cascade->p2 = sum_elem_ptr(*sum, equ_rect.y + equ_rect.height, equ_rect.x );
    cascade->p3 = sum_elem_ptr(*sum, equ_rect.y + equ_rect.height,
                                     equ_rect.x + equ_rect.width );

    cascade->pq0 = sqsum_elem_ptr(*sqsum, equ_rect.y, equ_rect.x);
    cascade->pq1 = sqsum_elem_ptr(*sqsum, equ_rect.y, equ_rect.x + equ_rect.width );
    cascade->pq2 = sqsum_elem_ptr(*sqsum, equ_rect.y + equ_rect.height, equ_rect.x );
    cascade->pq3 = sqsum_elem_ptr(*sqsum, equ_rect.y + equ_rect.height,
                                          equ_rect.x + equ_rect.width );

    /* init pointers in haar features according to real window size and
       given image pointers */
    {
#ifdef _OPENMP
    int max_threads = cvGetNumThreads();
    #pragma omp parallel for num_threads(max_threads), schedule(dynamic) 
#endif // _OPENMP
    for( i = 0; i < _cascade->count; i++ )
    {
        int j, k, l;
        for( j = 0; j < cascade->stage_classifier[i].count; j++ )
        {
            for( l = 0; l < cascade->stage_classifier[i].classifier[j].count; l++ )
            {
                CvHaarFeature* feature = 
                    &_cascade->stage_classifier[i].classifier[j].haar_feature[l];
                /* CvHidHaarClassifier* classifier =
                    cascade->stage_classifier[i].classifier + j; */
                CvHidHaarFeature* hidfeature = 
                    &cascade->stage_classifier[i].classifier[j].node[l].feature;
                int sum0 = 0, area0 = 0;  // s15.16
                CvRect r[3];
#if CV_ADJUST_FEATURES
                int base_w = -1, base_h = -1;
                int new_base_w = 0, new_base_h = 0;
                int kx, ky;
                int flagx = 0, flagy = 0;
                int x0 = 0, y0 = 0;
#endif
                int nr;

                /* align blocks */
                for( k = 0; k < CV_HAAR_FEATURE_MAX; k++ )
                {
                    if( !hidfeature->rect[k].p0 )
                        break;
#if CV_ADJUST_FEATURES
                    r[k] = feature->rect[k].r;
                    base_w = (int)CV_IMIN( (unsigned)base_w, (unsigned)(r[k].width-1) );
                    base_w = (int)CV_IMIN( (unsigned)base_w, (unsigned)(r[k].x - r[0].x-1) );
                    base_h = (int)CV_IMIN( (unsigned)base_h, (unsigned)(r[k].height-1) );
                    base_h = (int)CV_IMIN( (unsigned)base_h, (unsigned)(r[k].y - r[0].y-1) );
#endif
                }

                nr = k;

#if CV_ADJUST_FEATURES
                base_w += 1;
                base_h += 1;
                kx = r[0].width / base_w;
                ky = r[0].height / base_h;

                if( kx <= 0 )
                {
                    flagx = 1;
                    new_base_w = r[0].width / kx;
                    x0 = r[0].x;
                }

                if( ky <= 0 )
                {
                    flagy = 1;
                    new_base_h = r[0].height/ ky;
                    y0 = r[0].y;
                }
#endif
        
                for( k = 0; k < nr; k++ )
                {
                    CvRect tr;
                    int correction_ratio;  // s15.16
            
#if CV_ADJUST_FEATURES
                    if( flagx )
                    {
                        tr.x = (r[k].x - r[0].x) * new_base_w / base_w + x0;
                        tr.width = r[k].width * new_base_w / base_w;
                    }
                    else
#endif
                    {
                        tr.x = r[k].x;
                        tr.width = r[k].width;
                    }

#if CV_ADJUST_FEATURES
                    if( flagy )
                    {
                        tr.y = (r[k].y - r[0].y) * new_base_h / base_h + y0;
                        tr.height = r[k].height * new_base_h / base_h;
                    }
                    else
#endif
                    {
                        tr.y = r[k].y;
                        tr.height = r[k].height;
                    }

#if CV_ADJUST_WEIGHTS
                    {
                    // RAINER START
                    const float orig_feature_size =  (float)(feature->rect[k].r.width)*feature->rect[k].r.height; 
                    const float orig_norm_size = (float)(_cascade->orig_window_size.width)*(_cascade->orig_window_size.height);
                    const float feature_size = float(tr.width*tr.height);
                    //const float normSize    = float(equ_rect.width*equ_rect.height);
                    float target_ratio = orig_feature_size / orig_norm_size;
                    //float isRatio = featureSize / normSize;
                    //correctionRatio = targetRatio / isRatio / normSize;
                    correction_ratio = target_ratio / feature_size;
                    // RAINER END
                    }
#else
					correction_ratio = (!feature->tilted) ? weight_scale : (weight_scale>>1); //weight_scale* (!feature->tilted ? 1 : 0.5);
#endif

                    if( !feature->tilted )
                    {
                        hidfeature->rect[k].p0 = sum_elem_ptr(*sum, tr.y, tr.x);
                        hidfeature->rect[k].p1 = sum_elem_ptr(*sum, tr.y, tr.x + tr.width);
                        hidfeature->rect[k].p2 = sum_elem_ptr(*sum, tr.y + tr.height, tr.x);
                        hidfeature->rect[k].p3 = sum_elem_ptr(*sum, tr.y + tr.height, tr.x + tr.width);
                    }
                    else
                    {
                        hidfeature->rect[k].p2 = sum_elem_ptr(*tilted, tr.y + tr.width, tr.x + tr.width);
                        hidfeature->rect[k].p3 = sum_elem_ptr(*tilted, tr.y + tr.width + tr.height,
                                                              tr.x + tr.width - tr.height);
                        hidfeature->rect[k].p0 = sum_elem_ptr(*tilted, tr.y, tr.x);
                        hidfeature->rect[k].p1 = sum_elem_ptr(*tilted, tr.y + tr.height, tr.x - tr.height);
                    }

                    hidfeature->rect[k].weight = (feature->rect[k].weight * correction_ratio)>>16;

                    if( k == 0 )
                        area0 = (tr.width * tr.height);
                    else
                        sum0 += hidfeature->rect[k].weight * tr.width * tr.height;
                }

                hidfeature->rect[0].weight = (-sum0/area0);
            } /* l */
        } /* j */
    }
    }

    __END__;
}


CV_INLINE
int icvEvalHidHaarClassifier( CvHidHaarClassifier* classifier,
                                 int variance_norm_factor,
                                 size_t p_offset )
{
    int idx = 0;
    do 
    {
        CvHidHaarTreeNode* node = classifier->node + idx;
        int t = (node->threshold * variance_norm_factor)>>16; 

        int sum = (calc_sum(node->feature.rect[0],p_offset) * node->feature.rect[0].weight)>>16;
        sum += (calc_sum(node->feature.rect[1],p_offset) * node->feature.rect[1].weight)>>16;

        if( node->feature.rect[2].p0 )
            sum += (calc_sum(node->feature.rect[2],p_offset) * node->feature.rect[2].weight)>>16;

        idx = sum < t ? node->left : node->right;
    }
    while( idx > 0 );
    return classifier->alpha[-idx];
}


CV_IMPL int
cvRunHaarClassifierCascade( CvHaarClassifierCascade* _cascade,
                            CvPoint pt, int start_stage )
{
    int result = -1;
    CV_FUNCNAME("cvRunHaarClassifierCascade");

    __BEGIN__;

    int p_offset, pq_offset;
    int i, j;
    int mean, variance_norm_factor;
    CvHidHaarClassifierCascade* cascade;

    if( !CV_IS_HAAR_CLASSIFIER(_cascade) )
        CV_ERROR( !_cascade ? CV_StsNullPtr : CV_StsBadArg, "Invalid cascade pointer" );

    cascade = _cascade->hid_cascade;
    if( !cascade )
        CV_ERROR( CV_StsNullPtr, "Hidden cascade has not been created.\n"
            "Use cvSetImagesForHaarClassifierCascade" );

    if( pt.x < 0 || pt.y < 0 ||
        pt.x + _cascade->real_window_size.width >= cascade->sum.width-2 ||
        pt.y + _cascade->real_window_size.height >= cascade->sum.height-2 )
        EXIT;

    p_offset = pt.y * (cascade->sum.step/sizeof(sumtype)) + pt.x;
    pq_offset = pt.y * (cascade->sqsum.step/sizeof(sqsumtype)) + pt.x;
    mean = (calc_sum(*cascade,p_offset)*cascade->inv_window_area)>>16;
    variance_norm_factor = cascade->pq0[pq_offset] - cascade->pq1[pq_offset] -
                           cascade->pq2[pq_offset] + cascade->pq3[pq_offset];
    variance_norm_factor = ((variance_norm_factor*cascade->inv_window_area)>>16) - mean*mean;
    if( variance_norm_factor >= 0 )
        variance_norm_factor = (int)sqrt((float)variance_norm_factor);
    else
        variance_norm_factor = 1;

    if( cascade->is_tree )
    {
        CvHidHaarStageClassifier* ptr;
        assert( start_stage == 0 );

        result = 1;
        ptr = cascade->stage_classifier;

        while( ptr )
        {
            int stage_sum = 0;  // s15.16

            for( j = 0; j < ptr->count; j++ )
            {
                stage_sum += icvEvalHidHaarClassifier( ptr->classifier + j,
                    variance_norm_factor, p_offset );
            }

            if( stage_sum >= ptr->threshold )
            {
                ptr = ptr->child;
            }
            else
            {
                while( ptr && ptr->next == NULL ) ptr = ptr->parent;
                if( ptr == NULL )
                {
                    result = 0;
                    EXIT;
                }
                ptr = ptr->next;
            }
        }
    }
    else if( cascade->is_stump_based )
    {
        for( i = start_stage; i < cascade->count; i++ )
        {
            int stage_sum = 0;  // s15.16

            if( cascade->stage_classifier[i].two_rects )
            {
                for( j = 0; j < cascade->stage_classifier[i].count; j++ )
                {
                    CvHidHaarClassifier* classifier = cascade->stage_classifier[i].classifier + j;
                    CvHidHaarTreeNode* node = classifier->node;
                    int sum, t = node->threshold*variance_norm_factor;  // s15.16
					int a, b; // s15.16

                    sum = calc_sum(node->feature.rect[0],p_offset) * node->feature.rect[0].weight;
                    sum += calc_sum(node->feature.rect[1],p_offset) * node->feature.rect[1].weight;

                    a = classifier->alpha[0];
                    b = classifier->alpha[1];
                    stage_sum += sum < t ? a : b;
                }
            }
            else
            {
                for( j = 0; j < cascade->stage_classifier[i].count; j++ )
                {
                    CvHidHaarClassifier* classifier = cascade->stage_classifier[i].classifier + j;
                    CvHidHaarTreeNode* node = classifier->node;
                    int sum, t = node->threshold*variance_norm_factor;  // s15.16
					int a, b; // s15.16

                    sum = calc_sum(node->feature.rect[0],p_offset) * node->feature.rect[0].weight;
                    sum += calc_sum(node->feature.rect[1],p_offset) * node->feature.rect[1].weight;

                    if( node->feature.rect[2].p0 )
                        sum += calc_sum(node->feature.rect[2],p_offset) * node->feature.rect[2].weight;

                    a = classifier->alpha[0];
                    b = classifier->alpha[1];
                    stage_sum += sum < t ? a : b;
                }
            }

            if( stage_sum < cascade->stage_classifier[i].threshold )
            {
                result = -i;
                EXIT;
            }
        }
    }
    else
    {
        for( i = start_stage; i < cascade->count; i++ )
        {
            int stage_sum = 0; // s15.16

            for( j = 0; j < cascade->stage_classifier[i].count; j++ )
            {
                stage_sum += icvEvalHidHaarClassifier(
                    cascade->stage_classifier[i].classifier + j,
                    variance_norm_factor, p_offset );
            }

            if( stage_sum < cascade->stage_classifier[i].threshold )
            {
                result = -i;
                EXIT;
            }
        }
    }

    result = 1;

    __END__;

    return result;
}


static int is_equal( const void* _r1, const void* _r2, void* )
{
    const CvRect* r1 = (const CvRect*)_r1;
    const CvRect* r2 = (const CvRect*)_r2;
    int distance = r1->width>>2;

    return r2->x <= r1->x + distance &&
           r2->x >= r1->x - distance &&
           r2->y <= r1->y + distance &&
           r2->y >= r1->y - distance &&
           r2->width <= (r1->width+(r1->width>>2)) &&
           ( r2->width+(r2->width>>2) ) >= r1->width;
}


CV_IMPL CvSeq*
cvHaarDetectObjects( const CvArr* _img,
                     CvHaarClassifierCascade* cascade,
                     CvMemStorage* storage, int scale_factor, // s23.8
                     int min_neighbors, int flags, CvSize min_size, CvSize max_size)
{
    int split_stage = 2;

    CvMat stub, *img = (CvMat*)_img;
    CvMat *temp = 0, *sum = 0, *tilted = 0, *sqsum = 0, *img_small = 0;
    CvSeq* seq = 0;
    CvSeq* seq2 = 0;
    CvSeq* idx_seq = 0;
    CvSeq* result_seq = 0;
    CvMemStorage* temp_storage = 0;
    CvAvgComp* comps = 0;
    int i;
    
#ifdef _OPENMP
    CvSeq* seq_thread[CV_MAX_THREADS] = {0};
    int max_threads = 0;
#endif
    
    CV_FUNCNAME( "cvHaarDetectObjects" );

    __BEGIN__;

    int factor;
    int coi;

    if( !CV_IS_HAAR_CLASSIFIER(cascade) )
        CV_ERROR( !cascade ? CV_StsNullPtr : CV_StsBadArg, "Invalid classifier cascade" );

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "Null storage pointer" );

    CV_CALL( img = cvGetMat( img, &stub, &coi ));
    if( coi )
        CV_ERROR( CV_BadCOI, "COI is not supported" );

    if( CV_MAT_DEPTH(img->type) != CV_8U )
        CV_ERROR( CV_StsUnsupportedFormat, "Only 8-bit images are supported" );

    CV_CALL( temp = cvCreateMat( img->rows, img->cols, CV_8UC1 ));
    CV_CALL( sum = cvCreateMat( img->rows + 1, img->cols + 1, CV_32SC1 ));
    CV_CALL( sqsum = cvCreateMat( img->rows + 1, img->cols + 1, CV_32SC1 ));
    CV_CALL( temp_storage = cvCreateChildMemStorage( storage ));

#ifdef _OPENMP
    max_threads = cvGetNumThreads();
    for( i = 0; i < max_threads; i++ )
    {
        CvMemStorage* temp_storage_thread;
        CV_CALL( temp_storage_thread = cvCreateMemStorage(0));
        CV_CALL( seq_thread[i] = cvCreateSeq( 0, sizeof(CvSeq),
                        sizeof(CvRect), temp_storage_thread ));
    }
#endif

    if( !cascade->hid_cascade )
        CV_CALL( icvCreateHidHaarClassifierCascade(cascade) );

    if( cascade->hid_cascade->has_tilted_features )
        tilted = cvCreateMat( img->rows + 1, img->cols + 1, CV_32SC1 );

    seq = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvRect), temp_storage );
    seq2 = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvAvgComp), temp_storage );
    result_seq = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvAvgComp), storage );

    if( min_neighbors == 0 )
        seq = result_seq;

    if( CV_MAT_CN(img->type) > 1 )
    {
        cvCvtColor( img, temp, CV_BGR2GRAY );
		cvReleaseMat( &img );
        img = temp;
    }
    
    //if( flags & CV_HAAR_SCALE_IMAGE )
    {
        CvSize win_size0 = cascade->orig_window_size;

        CV_CALL( img_small = cvCreateMat( img->rows + 1, img->cols + 1, CV_8UC1 ));

        for( factor = (int)(1.*256); ; factor = (factor*scale_factor)>>8 )
        {
            int positive = 0;
            int x, y;
            CvSize win_size = { (win_size0.width*factor)>>8,
                                (win_size0.height*factor)>>8 };
            CvSize sz = { (img->cols<<8)/factor, (img->rows<<8)/factor };
            CvSize sz1 = { sz.width - win_size0.width, sz.height - win_size0.height };
            CvRect rect1 = { icv_object_win_border, icv_object_win_border,
                win_size0.width - icv_object_win_border*2,
                win_size0.height - icv_object_win_border*2 };
            CvMat img1, sum1, sqsum1, tilted1, mask1;
            CvMat* _tilted = 0;

            if( sz1.width <= 0 || sz1.height <= 0 )
                break;
            if( win_size.width < min_size.width || win_size.height < min_size.height )
                continue;
            if( (max_size.width && win_size.width > max_size.width) || 
				(max_size.height && win_size.height > max_size.height) )
                break;


            img1 = cvMat( sz.height, sz.width, CV_8UC1, img_small->data.ptr );
            sum1 = cvMat( sz.height+1, sz.width+1, CV_32SC1, sum->data.ptr );
            sqsum1 = cvMat( sz.height+1, sz.width+1, CV_32SC1, sqsum->data.ptr );
            if( tilted )
            {
                tilted1 = cvMat( sz.height+1, sz.width+1, CV_32SC1, tilted->data.ptr );
                _tilted = &tilted1;
            }
            mask1 = cvMat( sz1.height, sz1.width, CV_8UC1, temp->data.ptr );

            cvResize( img, &img1, CV_INTER_LINEAR );
            cvIntegral( &img1, &sum1, &sqsum1, _tilted );

            cvSetImagesForHaarClassifierCascade( cascade, &sum1, &sqsum1, 0);
            for( y = 0, positive = 0; y < sz1.height; y++ )
                for( x = 0; x < sz1.width; x++ )
                {
                    mask1.data.ptr[mask1.step*y + x] =
                            cvRunHaarClassifierCascade( cascade, cvPoint(x,y), 0 ) > 0;
                    positive += mask1.data.ptr[mask1.step*y + x];
                }

            if( positive > 0 )
            {
                for( y = 0; y < sz1.height; y++ )
                    for( x = 0; x < sz1.width; x++ )
                        if( mask1.data.ptr[mask1.step*y + x] != 0 )
                        {
                            CvRect obj_rect = { (x*factor)>>8, (y*factor)>>8,  // opencv1.1bug
                                                win_size.width, win_size.height };
                            cvSeqPush( seq, &obj_rect );
                        }
            }
        }
    }

#ifdef _OPENMP
	// gather the results
	for( i = 0; i < max_threads; i++ )
	{
		CvSeq* s = seq_thread[i];
        int j, total = s->total;
        CvSeqBlock* b = s->first;
        for( j = 0; j < total; j += b->count, b = b->next )
            cvSeqPushMulti( seq, b->data, b->count );
	}
#endif

    if( min_neighbors != 0 )
    {
        // group retrieved rectangles in order to filter out noise 
        int ncomp = cvSeqPartition( seq, 0, &idx_seq, is_equal, 0 );
        CV_CALL( comps = (CvAvgComp*)cvAlloc( (ncomp+1)*sizeof(comps[0])));
        memset( comps, 0, (ncomp+1)*sizeof(comps[0]));

        // count number of neighbors
        for( i = 0; i < seq->total; i++ )
        {
            CvRect r1 = *(CvRect*)cvGetSeqElem( seq, i );
            int idx = *(int*)cvGetSeqElem( idx_seq, i );
            assert( (unsigned)idx < (unsigned)ncomp );

            comps[idx].neighbors++;
             
            comps[idx].rect.x += r1.x;
            comps[idx].rect.y += r1.y;
            comps[idx].rect.width += r1.width;
            comps[idx].rect.height += r1.height;
        }

        // calculate average bounding box
        for( i = 0; i < ncomp; i++ )
        {
            int n = comps[i].neighbors;
            if( n >= min_neighbors )
            {
                CvAvgComp comp;
                comp.rect.x = (comps[i].rect.x*2 + n)/(2*n);
                comp.rect.y = (comps[i].rect.y*2 + n)/(2*n);
                comp.rect.width = (comps[i].rect.width*2 + n)/(2*n);
                comp.rect.height = (comps[i].rect.height*2 + n)/(2*n);
                comp.neighbors = comps[i].neighbors;

                cvSeqPush( seq2, &comp );
            }
        }

        // filter out small face rectangles inside large face rectangles
        for( i = 0; i < seq2->total; i++ )
        {
            CvAvgComp r1 = *(CvAvgComp*)cvGetSeqElem( seq2, i );
            int j, flag = 1;

            for( j = 0; j < seq2->total; j++ )
            {
                CvAvgComp r2 = *(CvAvgComp*)cvGetSeqElem( seq2, j );
                int distance = r2.rect.width>>2;
            
                if( i != j &&
                    r1.rect.x >= r2.rect.x - distance &&
                    r1.rect.y >= r2.rect.y - distance &&
                    r1.rect.x + r1.rect.width <= r2.rect.x + r2.rect.width + distance &&
                    r1.rect.y + r1.rect.height <= r2.rect.y + r2.rect.height + distance &&
                    (r2.neighbors > MAX( 3, r1.neighbors ) || r1.neighbors < 3) )
                {
                    flag = 0;
                    break;
                }
            }

            if( flag )
            {
                cvSeqPush( result_seq, &r1 );
                /* cvSeqPush( result_seq, &r1.rect ); */
            }
        }
    }

    __END__;

#ifdef _OPENMP
	for( i = 0; i < max_threads; i++ )
	{
		if( seq_thread[i] )
            cvReleaseMemStorage( &seq_thread[i]->storage );
	}
#endif

    cvReleaseMemStorage( &temp_storage );
    cvReleaseMat( &sum );
    cvReleaseMat( &sqsum );
    cvReleaseMat( &tilted );
    cvReleaseMat( &temp );
    cvReleaseMat( &img_small );
    cvFree( &comps );

    return result_seq;
}


static CvHaarClassifierCascade*
icvLoadCascadeCART( const char** input_cascade, int n, CvSize orig_window_size )
{
    int i;
    CvHaarClassifierCascade* cascade = icvCreateHaarClassifierCascade(n);
    cascade->orig_window_size = orig_window_size;

    for( i = 0; i < n; i++ )
    {
        int j, count, l;
        int threshold = 0; // s15.16
        const char* stage = input_cascade[i];
        int dl = 0;
		float tmp;

        /* tree links */
        int parent = -1;
        int next = -1;

        sscanf( stage, "%d%n", &count, &dl );
        stage += dl;
        
        assert( count > 0 );
        cascade->stage_classifier[i].count = count;
        cascade->stage_classifier[i].classifier =
            (CvHaarClassifier*)cvAlloc( count*sizeof(cascade->stage_classifier[i].classifier[0]));

        for( j = 0; j < count; j++ )
        {
            CvHaarClassifier* classifier = cascade->stage_classifier[i].classifier + j;
            int k, rects = 0;
            char str[100];

            sscanf( stage, "%d%n", &classifier->count, &dl );
            stage += dl;

            classifier->haar_feature = (CvHaarFeature*) cvAlloc( 
                classifier->count * ( sizeof( *classifier->haar_feature ) +
                                      sizeof( *classifier->threshold ) +
                                      sizeof( *classifier->left ) +
                                      sizeof( *classifier->right ) ) +
                (classifier->count + 1) * sizeof( *classifier->alpha ) );
            classifier->threshold = (int*) (classifier->haar_feature+classifier->count);
            classifier->left = (int*) (classifier->threshold + classifier->count);
            classifier->right = (int*) (classifier->left + classifier->count);
            classifier->alpha = (int*) (classifier->right + classifier->count);
            
            for( l = 0; l < classifier->count; l++ )
            {
                sscanf( stage, "%d%n", &rects, &dl );
                stage += dl;

                assert( rects >= 2 && rects <= CV_HAAR_FEATURE_MAX );

                for( k = 0; k < rects; k++ )
                {
                    CvRect r;
                    int band = 0;
                    sscanf( stage, "%d%d%d%d%d%f%n",
                            &r.x, &r.y, &r.width, &r.height, &band, &tmp, &dl );                            
                    stage += dl;
                    classifier->haar_feature[l].rect[k].r = r;
					classifier->haar_feature[l].rect[k].weight = (int)(tmp*65536);
                }
                sscanf( stage, "%s%n", str, &dl );
                stage += dl;
            
                classifier->haar_feature[l].tilted = strncmp( str, "tilted", 6 ) == 0;
            
                for( k = rects; k < CV_HAAR_FEATURE_MAX; k++ )
                {
                    memset( classifier->haar_feature[l].rect + k, 0,
                            sizeof(classifier->haar_feature[l].rect[k]) );
                }
                
                sscanf( stage, "%f%d%d%n", &tmp, &(classifier->left[l]),
                                       &(classifier->right[l]), &dl );
				classifier->threshold[l] = (int)(tmp*65536);  // s15.16
                stage += dl;
            }
            for( l = 0; l <= classifier->count; l++ )
            {
                sscanf( stage, "%f%n", &tmp, &dl );
				classifier->alpha[l] = (int)(tmp*65536);  // s15.16
                stage += dl;
            }
        }
       
        sscanf( stage, "%f%n", &tmp, &dl );
		threshold = (int)(tmp*65536);  // s15.16
        stage += dl;

        cascade->stage_classifier[i].threshold = threshold;

        /* load tree links */
        if( sscanf( stage, "%d%d%n", &parent, &next, &dl ) != 2 )
        {
            parent = i - 1;
            next = -1;
        }
        stage += dl;

        cascade->stage_classifier[i].parent = parent;
        cascade->stage_classifier[i].next = next;
        cascade->stage_classifier[i].child = -1;

        if( parent != -1 && cascade->stage_classifier[parent].child == -1 )
        {
            cascade->stage_classifier[parent].child = i;
        }
    }

    return cascade;
}

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

CV_IMPL CvHaarClassifierCascade*
cvLoadHaarClassifierCascade( const char* directory, CvSize orig_window_size )
{
    const char** input_cascade = 0; 
    CvHaarClassifierCascade *cascade = 0;

    CV_FUNCNAME( "cvLoadHaarClassifierCascade" );

    __BEGIN__;

    int i, n;
    const char* slash;
    char name[_MAX_PATH];
    int size = 0;
    char* ptr = 0;

    if( !directory )
        CV_ERROR( CV_StsNullPtr, "Null path is passed" );

    n = (int)strlen(directory)-1;
    slash = directory[n] == '\\' || directory[n] == '/' ? "" : "/";

    /* try to read the classifier from directory */
    for( n = 0; ; n++ )
    {
        sprintf( name, "%s%s%d/AdaBoostCARTHaarClassifier.txt", directory, slash, n );
        FILE* f = fopen( name, "rb" );
        if( !f )
            break;
        fseek( f, 0, SEEK_END );
        size += ftell( f ) + 1;
        fclose(f);
    }

    if( n == 0 && slash[0] )
    {
        CV_CALL( cascade = (CvHaarClassifierCascade*)cvLoad( directory ));
        EXIT;
    }
    else if( n == 0 )
        CV_ERROR( CV_StsBadArg, "Invalid path" );
    
    size += (n+1)*sizeof(char*);
    CV_CALL( input_cascade = (const char**)cvAlloc( size ));
    ptr = (char*)(input_cascade + n + 1);

    for( i = 0; i < n; i++ )
    {
        sprintf( name, "%s/%d/AdaBoostCARTHaarClassifier.txt", directory, i );
        FILE* f = fopen( name, "rb" );
        if( !f )
            CV_ERROR( CV_StsError, "" );
        fseek( f, 0, SEEK_END );
        size = ftell( f );
        fseek( f, 0, SEEK_SET );
        fread( ptr, 1, size, f );
        fclose(f);
        input_cascade[i] = ptr;
        ptr += size;
        *ptr++ = '\0';
    }

    input_cascade[n] = 0;
    cascade = icvLoadCascadeCART( input_cascade, n, orig_window_size );

    __END__;

    if( input_cascade )
        cvFree( &input_cascade );

    if( cvGetErrStatus() < 0 )
        cvReleaseHaarClassifierCascade( &cascade );

    return cascade;
}


CV_IMPL void
cvReleaseHaarClassifierCascade( CvHaarClassifierCascade** _cascade )
{
    if( _cascade && *_cascade )
    {
        int i, j;
        CvHaarClassifierCascade* cascade = *_cascade;

        for( i = 0; i < cascade->count; i++ )
        {
            for( j = 0; j < cascade->stage_classifier[i].count; j++ )
                cvFree( &cascade->stage_classifier[i].classifier[j].haar_feature );
            cvFree( &cascade->stage_classifier[i].classifier );
        }
        icvReleaseHidHaarClassifierCascade( &cascade->hid_cascade );
        cvFree( _cascade );
    }
}


/****************************************************************************************\
*                                  Persistence functions                                 *
\****************************************************************************************/

/* field names */

#define ICV_HAAR_SIZE_NAME            "size"
#define ICV_HAAR_STAGES_NAME          "stages"
#define ICV_HAAR_TREES_NAME             "trees"
#define ICV_HAAR_FEATURE_NAME             "feature"
#define ICV_HAAR_RECTS_NAME                 "rects"
#define ICV_HAAR_TILTED_NAME                "tilted"
#define ICV_HAAR_THRESHOLD_NAME           "threshold"
#define ICV_HAAR_LEFT_NODE_NAME           "left_node"
#define ICV_HAAR_LEFT_VAL_NAME            "left_val"
#define ICV_HAAR_RIGHT_NODE_NAME          "right_node"
#define ICV_HAAR_RIGHT_VAL_NAME           "right_val"
#define ICV_HAAR_STAGE_THRESHOLD_NAME   "stage_threshold"
#define ICV_HAAR_PARENT_NAME            "parent"
#define ICV_HAAR_NEXT_NAME              "next"

//#define CREATE_HAAR_H_FILE	

static int
icvIsHaarClassifier( const void* struct_ptr )
{
    return CV_IS_HAAR_CLASSIFIER( struct_ptr );
}

static void*
icvReadHaarClassifier( CvFileStorage* fs, CvFileNode* node )
{
    CvHaarClassifierCascade* cascade = NULL;

    CV_FUNCNAME( "cvReadHaarClassifier" );

    __BEGIN__;

    char buf[256];
    CvFileNode* seq_fn = NULL; /* sequence */
    CvFileNode* fn = NULL;
    CvFileNode* stages_fn = NULL;
    CvSeqReader stages_reader;
    int n;
    int i, j, k, l;
    int parent, next;

    CV_CALL( stages_fn = cvGetFileNodeByName( fs, node, ICV_HAAR_STAGES_NAME ) );
    if( !stages_fn || !CV_NODE_IS_SEQ( stages_fn->tag) )
        CV_ERROR( CV_StsError, "Invalid stages node" );

    n = stages_fn->data.seq->total;
    CV_CALL( cascade = icvCreateHaarClassifierCascade(n) );

    /* read size */
    CV_CALL( seq_fn = cvGetFileNodeByName( fs, node, ICV_HAAR_SIZE_NAME ) );
    if( !seq_fn || !CV_NODE_IS_SEQ( seq_fn->tag ) || seq_fn->data.seq->total != 2 )
        CV_ERROR( CV_StsError, "size node is not a valid sequence." );
    CV_CALL( fn = (CvFileNode*) cvGetSeqElem( seq_fn->data.seq, 0 ) );
    if( !CV_NODE_IS_INT( fn->tag ) || fn->data.i <= 0 )
        CV_ERROR( CV_StsError, "Invalid size node: width must be positive integer" );
    cascade->orig_window_size.width = fn->data.i;
    CV_CALL( fn = (CvFileNode*) cvGetSeqElem( seq_fn->data.seq, 1 ) );
    if( !CV_NODE_IS_INT( fn->tag ) || fn->data.i <= 0 )
        CV_ERROR( CV_StsError, "Invalid size node: height must be positive integer" );
    cascade->orig_window_size.height = fn->data.i;


    CV_CALL( cvStartReadSeq( stages_fn->data.seq, &stages_reader ) );
    for( i = 0; i < n; ++i )
    {
        CvFileNode* stage_fn;
        CvFileNode* trees_fn;
        CvSeqReader trees_reader;

        stage_fn = (CvFileNode*) stages_reader.ptr;
        if( !CV_NODE_IS_MAP( stage_fn->tag ) )
        {
            sprintf( buf, "Invalid stage %d", i );
            CV_ERROR( CV_StsError, buf );
        }

        CV_CALL( trees_fn = cvGetFileNodeByName( fs, stage_fn, ICV_HAAR_TREES_NAME ) );
        if( !trees_fn || !CV_NODE_IS_SEQ( trees_fn->tag )
            || trees_fn->data.seq->total <= 0 )
        {
            sprintf( buf, "Trees node is not a valid sequence. (stage %d)", i );
            CV_ERROR( CV_StsError, buf );
        }

        CV_CALL( cascade->stage_classifier[i].classifier =
            (CvHaarClassifier*) cvAlloc( trees_fn->data.seq->total
                * sizeof( cascade->stage_classifier[i].classifier[0] ) ) );
        for( j = 0; j < trees_fn->data.seq->total; ++j )
        {
            cascade->stage_classifier[i].classifier[j].haar_feature = NULL;
        }
        cascade->stage_classifier[i].count = trees_fn->data.seq->total;

        CV_CALL( cvStartReadSeq( trees_fn->data.seq, &trees_reader ) );
        for( j = 0; j < trees_fn->data.seq->total; ++j )
        {
            CvFileNode* tree_fn;
            CvSeqReader tree_reader;
            CvHaarClassifier* classifier;
            int last_idx;

            classifier = &cascade->stage_classifier[i].classifier[j];
            tree_fn = (CvFileNode*) trees_reader.ptr;
            if( !CV_NODE_IS_SEQ( tree_fn->tag ) || tree_fn->data.seq->total <= 0 )
            {
                sprintf( buf, "Tree node is not a valid sequence."
                         " (stage %d, tree %d)", i, j );
                CV_ERROR( CV_StsError, buf );
            }

            classifier->count = tree_fn->data.seq->total;
            CV_CALL( classifier->haar_feature = (CvHaarFeature*) cvAlloc( 
                classifier->count * ( sizeof( *classifier->haar_feature ) +
                                      sizeof( *classifier->threshold ) +
                                      sizeof( *classifier->left ) +
                                      sizeof( *classifier->right ) ) +
                (classifier->count + 1) * sizeof( *classifier->alpha ) ) );
            classifier->threshold = (int*) (classifier->haar_feature+classifier->count);
            classifier->left = (int*) (classifier->threshold + classifier->count);
            classifier->right = (int*) (classifier->left + classifier->count);
            classifier->alpha = (int*) (classifier->right + classifier->count);

            CV_CALL( cvStartReadSeq( tree_fn->data.seq, &tree_reader ) );
            for( k = 0, last_idx = 0; k < tree_fn->data.seq->total; ++k )
            {
                CvFileNode* node_fn;
                CvFileNode* feature_fn;
                CvFileNode* rects_fn;
                CvSeqReader rects_reader;

                node_fn = (CvFileNode*) tree_reader.ptr;
                if( !CV_NODE_IS_MAP( node_fn->tag ) )
                {
                    sprintf( buf, "Tree node %d is not a valid map. (stage %d, tree %d)",
                             k, i, j );
                    CV_ERROR( CV_StsError, buf );
                }
                CV_CALL( feature_fn = cvGetFileNodeByName( fs, node_fn,
                    ICV_HAAR_FEATURE_NAME ) );
                if( !feature_fn || !CV_NODE_IS_MAP( feature_fn->tag ) )
                {
                    sprintf( buf, "Feature node is not a valid map. "
                             "(stage %d, tree %d, node %d)", i, j, k );
                    CV_ERROR( CV_StsError, buf );
                }
                CV_CALL( rects_fn = cvGetFileNodeByName( fs, feature_fn,
                    ICV_HAAR_RECTS_NAME ) );
                if( !rects_fn || !CV_NODE_IS_SEQ( rects_fn->tag )
                    || rects_fn->data.seq->total < 1
                    || rects_fn->data.seq->total > CV_HAAR_FEATURE_MAX )
                {
                    sprintf( buf, "Rects node is not a valid sequence. "
                             "(stage %d, tree %d, node %d)", i, j, k );
                    CV_ERROR( CV_StsError, buf );
                }
                CV_CALL( cvStartReadSeq( rects_fn->data.seq, &rects_reader ) );
                for( l = 0; l < rects_fn->data.seq->total; ++l )
                {
                    CvFileNode* rect_fn;
                    CvRect r;

                    rect_fn = (CvFileNode*) rects_reader.ptr;
                    if( !CV_NODE_IS_SEQ( rect_fn->tag ) || rect_fn->data.seq->total != 5 )
                    {
                        sprintf( buf, "Rect %d is not a valid sequence. "
                                 "(stage %d, tree %d, node %d)", l, i, j, k );
                        CV_ERROR( CV_StsError, buf );
                    }
                    
                    fn = CV_SEQ_ELEM( rect_fn->data.seq, CvFileNode, 0 );
                    if( !CV_NODE_IS_INT( fn->tag ) || fn->data.i < 0 )
                    {
                        sprintf( buf, "x coordinate must be non-negative integer. "
                                 "(stage %d, tree %d, node %d, rect %d)", i, j, k, l );
                        CV_ERROR( CV_StsError, buf );
                    }
                    r.x = fn->data.i;
                    fn = CV_SEQ_ELEM( rect_fn->data.seq, CvFileNode, 1 );
                    if( !CV_NODE_IS_INT( fn->tag ) || fn->data.i < 0 )
                    {
                        sprintf( buf, "y coordinate must be non-negative integer. "
                                 "(stage %d, tree %d, node %d, rect %d)", i, j, k, l );
                        CV_ERROR( CV_StsError, buf );
                    }
                    r.y = fn->data.i;
                    fn = CV_SEQ_ELEM( rect_fn->data.seq, CvFileNode, 2 );
                    if( !CV_NODE_IS_INT( fn->tag ) || fn->data.i <= 0
                        || r.x + fn->data.i > cascade->orig_window_size.width )
                    {
                        sprintf( buf, "width must be positive integer and "
                                 "(x + width) must not exceed window width. "
                                 "(stage %d, tree %d, node %d, rect %d)", i, j, k, l );
                        CV_ERROR( CV_StsError, buf );
                    }
                    r.width = fn->data.i;
                    fn = CV_SEQ_ELEM( rect_fn->data.seq, CvFileNode, 3 );
                    if( !CV_NODE_IS_INT( fn->tag ) || fn->data.i <= 0
                        || r.y + fn->data.i > cascade->orig_window_size.height )
                    {
                        sprintf( buf, "height must be positive integer and "
                                 "(y + height) must not exceed window height. "
                                 "(stage %d, tree %d, node %d, rect %d)", i, j, k, l );
                        CV_ERROR( CV_StsError, buf );
                    }
                    r.height = fn->data.i;
                    fn = CV_SEQ_ELEM( rect_fn->data.seq, CvFileNode, 4 );
                    if( !CV_NODE_IS_REAL( fn->tag ) )
                    {
                        sprintf( buf, "weight must be real number. "
                                 "(stage %d, tree %d, node %d, rect %d)", i, j, k, l );
                        CV_ERROR( CV_StsError, buf );
                    }

                    classifier->haar_feature[k].rect[l].weight = (int) (fn->data.f*65536);
                    classifier->haar_feature[k].rect[l].r = r;

                    CV_NEXT_SEQ_ELEM( sizeof( *rect_fn ), rects_reader );
                } /* for each rect */
                for( l = rects_fn->data.seq->total; l < CV_HAAR_FEATURE_MAX; ++l )
                {
                    classifier->haar_feature[k].rect[l].weight = 0;
                    classifier->haar_feature[k].rect[l].r = cvRect( 0, 0, 0, 0 );
                }

                CV_CALL( fn = cvGetFileNodeByName( fs, feature_fn, ICV_HAAR_TILTED_NAME));
                if( !fn || !CV_NODE_IS_INT( fn->tag ) )
                {
                    sprintf( buf, "tilted must be 0 or 1. "
                             "(stage %d, tree %d, node %d)", i, j, k );
                    CV_ERROR( CV_StsError, buf );
                }
                classifier->haar_feature[k].tilted = ( fn->data.i != 0 );
                CV_CALL( fn = cvGetFileNodeByName( fs, node_fn, ICV_HAAR_THRESHOLD_NAME));
                if( !fn || !CV_NODE_IS_REAL( fn->tag ) )
                {
                    sprintf( buf, "threshold must be real number. "
                             "(stage %d, tree %d, node %d)", i, j, k );
                    CV_ERROR( CV_StsError, buf );
                }
                classifier->threshold[k] = (int) (fn->data.f*65536);
                CV_CALL( fn = cvGetFileNodeByName( fs, node_fn, ICV_HAAR_LEFT_NODE_NAME));
                if( fn )
                {
                    if( !CV_NODE_IS_INT( fn->tag ) || fn->data.i <= k
                        || fn->data.i >= tree_fn->data.seq->total )
                    {
                        sprintf( buf, "left node must be valid node number. "
                                 "(stage %d, tree %d, node %d)", i, j, k );
                        CV_ERROR( CV_StsError, buf );
                    }
                    /* left node */
                    classifier->left[k] = fn->data.i;
                }
                else
                {
                    CV_CALL( fn = cvGetFileNodeByName( fs, node_fn,
                        ICV_HAAR_LEFT_VAL_NAME ) );
                    if( !fn )
                    {
                        sprintf( buf, "left node or left value must be specified. "
                                 "(stage %d, tree %d, node %d)", i, j, k );
                        CV_ERROR( CV_StsError, buf );
                    }
                    if( !CV_NODE_IS_REAL( fn->tag ) )
                    {
                        sprintf( buf, "left value must be real number. "
                                 "(stage %d, tree %d, node %d)", i, j, k );
                        CV_ERROR( CV_StsError, buf );
                    }
                    /* left value */
                    if( last_idx >= classifier->count + 1 )
                    {
                        sprintf( buf, "Tree structure is broken: too many values. "
                                 "(stage %d, tree %d, node %d)", i, j, k );
                        CV_ERROR( CV_StsError, buf );
                    }
                    classifier->left[k] = -last_idx;
                    classifier->alpha[last_idx++] = (int) (fn->data.f*65536);
                }
                CV_CALL( fn = cvGetFileNodeByName( fs, node_fn,ICV_HAAR_RIGHT_NODE_NAME));
                if( fn )
                {
                    if( !CV_NODE_IS_INT( fn->tag ) || fn->data.i <= k
                        || fn->data.i >= tree_fn->data.seq->total )
                    {
                        sprintf( buf, "right node must be valid node number. "
                                 "(stage %d, tree %d, node %d)", i, j, k );
                        CV_ERROR( CV_StsError, buf );
                    }
                    /* right node */
                    classifier->right[k] = fn->data.i;
                }
                else
                {
                    CV_CALL( fn = cvGetFileNodeByName( fs, node_fn,
                        ICV_HAAR_RIGHT_VAL_NAME ) );
                    if( !fn )
                    {
                        sprintf( buf, "right node or right value must be specified. "
                                 "(stage %d, tree %d, node %d)", i, j, k );
                        CV_ERROR( CV_StsError, buf );
                    }
                    if( !CV_NODE_IS_REAL( fn->tag ) )
                    {
                        sprintf( buf, "right value must be real number. "
                                 "(stage %d, tree %d, node %d)", i, j, k );
                        CV_ERROR( CV_StsError, buf );
                    }
                    /* right value */
                    if( last_idx >= classifier->count + 1 )
                    {
                        sprintf( buf, "Tree structure is broken: too many values. "
                                 "(stage %d, tree %d, node %d)", i, j, k );
                        CV_ERROR( CV_StsError, buf );
                    }
                    classifier->right[k] = -last_idx;
                    classifier->alpha[last_idx++] = (int) (fn->data.f*65536);
                }

                CV_NEXT_SEQ_ELEM( sizeof( *node_fn ), tree_reader );
            } /* for each node */
            if( last_idx != classifier->count + 1 )
            {
                sprintf( buf, "Tree structure is broken: too few values. "
                         "(stage %d, tree %d)", i, j );
                CV_ERROR( CV_StsError, buf );
            }

            CV_NEXT_SEQ_ELEM( sizeof( *tree_fn ), trees_reader );
        } /* for each tree */

        CV_CALL( fn = cvGetFileNodeByName( fs, stage_fn, ICV_HAAR_STAGE_THRESHOLD_NAME));
        if( !fn || !CV_NODE_IS_REAL( fn->tag ) )
        {
            sprintf( buf, "stage threshold must be real number. (stage %d)", i );
            CV_ERROR( CV_StsError, buf );
        }
        cascade->stage_classifier[i].threshold = (int) (fn->data.f*65536);

        parent = i - 1;
        next = -1;

        CV_CALL( fn = cvGetFileNodeByName( fs, stage_fn, ICV_HAAR_PARENT_NAME ) );
        if( !fn || !CV_NODE_IS_INT( fn->tag )
            || fn->data.i < -1 || fn->data.i >= cascade->count )
        {
            sprintf( buf, "parent must be integer number. (stage %d)", i );
            CV_ERROR( CV_StsError, buf );
        }
        parent = fn->data.i;
        CV_CALL( fn = cvGetFileNodeByName( fs, stage_fn, ICV_HAAR_NEXT_NAME ) );
        if( !fn || !CV_NODE_IS_INT( fn->tag )
            || fn->data.i < -1 || fn->data.i >= cascade->count )
        {
            sprintf( buf, "next must be integer number. (stage %d)", i );
            CV_ERROR( CV_StsError, buf );
        }
        next = fn->data.i;

        cascade->stage_classifier[i].parent = parent;
        cascade->stage_classifier[i].next = next;
        cascade->stage_classifier[i].child = -1;

        if( parent != -1 && cascade->stage_classifier[parent].child == -1 )
        {
            cascade->stage_classifier[parent].child = i;
        }
        
        CV_NEXT_SEQ_ELEM( sizeof( *stage_fn ), stages_reader );
    } /* for each stage */

#ifdef CREATE_HAAR_H_FILE
	{
	FILE *fp = fopen("haarcascade_frontalface_alt.h", "w");
	if(!fp)
        CV_ERROR( CV_StsError, "create haarcascade_frontalface_alt.h file fail" );	

	for( i = 0; i < n; ++i ) {
		for( j = 0; j < cascade->stage_classifier[i].count; ++j ) {
			fprintf(fp, "CvHaarClassifier0 stage%d_cassifier%d =\n", i, j);
			fprintf(fp, "{{%d, {{{%d,%d,%d,%d}, %d}, {{%d,%d,%d,%d}, %d}, {{%d,%d,%d,%d}, %d}}}, %d, %d, %d, {%d, %d}};\n",
					cascade->stage_classifier[i].classifier[j].haar_feature[0].tilted,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[0].r.x,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[0].r.y,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[0].r.width,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[0].r.height,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[0].weight,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[1].r.x,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[1].r.y,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[1].r.width,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[1].r.height,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[1].weight,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[2].r.x,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[2].r.y,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[2].r.width,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[2].r.height,
					cascade->stage_classifier[i].classifier[j].haar_feature[0].rect[2].weight,
					cascade->stage_classifier[i].classifier[j].threshold[0],
					cascade->stage_classifier[i].classifier[j].left[0],
					cascade->stage_classifier[i].classifier[j].right[0],
					cascade->stage_classifier[i].classifier[j].alpha[0],
					cascade->stage_classifier[i].classifier[j].alpha[1]);
		}
		fprintf(fp,"\n");

		fprintf(fp, "CvHaarClassifier classifier%d[%d] = {\n", 
			i, cascade->stage_classifier[i].count);
		for( j = 0; j < cascade->stage_classifier[i].count; ++j ) {
			fprintf(fp, "{ %d, &stage%d_cassifier%d.haar_feature, "
					         "&stage%d_cassifier%d.threshold, "
					         "&stage%d_cassifier%d.left, "
					         "&stage%d_cassifier%d.right, "
							 "&stage%d_cassifier%d.alpha[0]},\n", 
						 cascade->stage_classifier[i].classifier[j].count, i, j,
						 i, j,
						 i, j,
						 i, j,
						 i, j);
		}	                 
		fprintf(fp,"};\n\n");
	}
	fprintf(fp, "CvHaarStageClassifier stage_classifier[%d] = {\n", n);
	for( i = 0; i < n; ++i ) {
		fprintf(fp, "{ %3d, %7d, classifier%d, %d, %d, %d},\n",
			cascade->stage_classifier[i].count,
			cascade->stage_classifier[i].threshold,
			i,
			cascade->stage_classifier[i].next,
			cascade->stage_classifier[i].parent,
			cascade->stage_classifier[i].child);
	}
	fprintf(fp, "};\n");
	fprintf(fp, "\n");
	fprintf(fp, "CvHaarClassifierCascade haarcascade_frontalface_alt =\n");
	fprintf(fp, "{ CV_HAAR_MAGIC_VAL, %d,  {%d, %d}, {0, 0}, stage_classifier, NULL};\n", 
		n, cascade->orig_window_size.width, cascade->orig_window_size.height);
	fclose(fp);
	}
#endif

    __END__;

    if( cvGetErrStatus() < 0 )
    {
        cvReleaseHaarClassifierCascade( &cascade );
        cascade = NULL;
    }

    return cascade;
}

static void
icvWriteHaarClassifier( CvFileStorage* fs, const char* name, const void* struct_ptr,
                        CvAttrList attributes )
{
    CV_FUNCNAME( "cvWriteHaarClassifier" );

    __BEGIN__;

    int i, j, k, l;
    char buf[256];
    const CvHaarClassifierCascade* cascade = (const CvHaarClassifierCascade*) struct_ptr;

    /* TODO: parameters check */

    CV_CALL( cvStartWriteStruct( fs, name, CV_NODE_MAP, CV_TYPE_NAME_HAAR, attributes ) );
    
    CV_CALL( cvStartWriteStruct( fs, ICV_HAAR_SIZE_NAME, CV_NODE_SEQ | CV_NODE_FLOW ) );
    CV_CALL( cvWriteInt( fs, NULL, cascade->orig_window_size.width ) );
    CV_CALL( cvWriteInt( fs, NULL, cascade->orig_window_size.height ) );
    CV_CALL( cvEndWriteStruct( fs ) ); /* size */
    
    CV_CALL( cvStartWriteStruct( fs, ICV_HAAR_STAGES_NAME, CV_NODE_SEQ ) );    
    for( i = 0; i < cascade->count; ++i )
    {
        CV_CALL( cvStartWriteStruct( fs, NULL, CV_NODE_MAP ) );
        sprintf( buf, "stage %d", i );
        CV_CALL( cvWriteComment( fs, buf, 1 ) );
        
        CV_CALL( cvStartWriteStruct( fs, ICV_HAAR_TREES_NAME, CV_NODE_SEQ ) );

        for( j = 0; j < cascade->stage_classifier[i].count; ++j )
        {
            CvHaarClassifier* tree = &cascade->stage_classifier[i].classifier[j];

            CV_CALL( cvStartWriteStruct( fs, NULL, CV_NODE_SEQ ) );
            sprintf( buf, "tree %d", j );
            CV_CALL( cvWriteComment( fs, buf, 1 ) );

            for( k = 0; k < tree->count; ++k )
            {
                CvHaarFeature* feature = &tree->haar_feature[k];

                CV_CALL( cvStartWriteStruct( fs, NULL, CV_NODE_MAP ) );
                if( k )
                {
                    sprintf( buf, "node %d", k );
                }
                else
                {
                    sprintf( buf, "root node" );
                }
                CV_CALL( cvWriteComment( fs, buf, 1 ) );

                CV_CALL( cvStartWriteStruct( fs, ICV_HAAR_FEATURE_NAME, CV_NODE_MAP ) );
                
                CV_CALL( cvStartWriteStruct( fs, ICV_HAAR_RECTS_NAME, CV_NODE_SEQ ) );
                for( l = 0; l < CV_HAAR_FEATURE_MAX && feature->rect[l].r.width != 0; ++l )
                {
                    CV_CALL( cvStartWriteStruct( fs, NULL, CV_NODE_SEQ | CV_NODE_FLOW ) );
                    CV_CALL( cvWriteInt(  fs, NULL, feature->rect[l].r.x ) );
                    CV_CALL( cvWriteInt(  fs, NULL, feature->rect[l].r.y ) );
                    CV_CALL( cvWriteInt(  fs, NULL, feature->rect[l].r.width ) );
                    CV_CALL( cvWriteInt(  fs, NULL, feature->rect[l].r.height ) );
                    CV_CALL( cvWriteReal( fs, NULL, feature->rect[l].weight ) );
                    CV_CALL( cvEndWriteStruct( fs ) ); /* rect */
                }
                CV_CALL( cvEndWriteStruct( fs ) ); /* rects */
                CV_CALL( cvWriteInt( fs, ICV_HAAR_TILTED_NAME, feature->tilted ) );
                CV_CALL( cvEndWriteStruct( fs ) ); /* feature */
                
                CV_CALL( cvWriteReal( fs, ICV_HAAR_THRESHOLD_NAME, tree->threshold[k]) );

                if( tree->left[k] > 0 )
                {
                    CV_CALL( cvWriteInt( fs, ICV_HAAR_LEFT_NODE_NAME, tree->left[k] ) );
                }
                else
                {
                    CV_CALL( cvWriteReal( fs, ICV_HAAR_LEFT_VAL_NAME,
                        tree->alpha[-tree->left[k]] ) );
                }

                if( tree->right[k] > 0 )
                {
                    CV_CALL( cvWriteInt( fs, ICV_HAAR_RIGHT_NODE_NAME, tree->right[k] ) );
                }
                else
                {
                    CV_CALL( cvWriteReal( fs, ICV_HAAR_RIGHT_VAL_NAME,
                        tree->alpha[-tree->right[k]] ) );
                }

                CV_CALL( cvEndWriteStruct( fs ) ); /* split */
            }

            CV_CALL( cvEndWriteStruct( fs ) ); /* tree */
        }

        CV_CALL( cvEndWriteStruct( fs ) ); /* trees */

        CV_CALL( cvWriteReal( fs, ICV_HAAR_STAGE_THRESHOLD_NAME,
                              cascade->stage_classifier[i].threshold) );

        CV_CALL( cvWriteInt( fs, ICV_HAAR_PARENT_NAME,
                              cascade->stage_classifier[i].parent ) );
        CV_CALL( cvWriteInt( fs, ICV_HAAR_NEXT_NAME,
                              cascade->stage_classifier[i].next ) );

        CV_CALL( cvEndWriteStruct( fs ) ); /* stage */
    } /* for each stage */
    
    CV_CALL( cvEndWriteStruct( fs ) ); /* stages */
    CV_CALL( cvEndWriteStruct( fs ) ); /* root */

    __END__;
}

static void*
icvCloneHaarClassifier( const void* struct_ptr )
{
    CvHaarClassifierCascade* cascade = NULL;

    CV_FUNCNAME( "cvCloneHaarClassifier" );

    __BEGIN__;

    int i, j, k, n;
    const CvHaarClassifierCascade* cascade_src =
        (const CvHaarClassifierCascade*) struct_ptr;

    n = cascade_src->count;
    CV_CALL( cascade = icvCreateHaarClassifierCascade(n) );
    cascade->orig_window_size = cascade_src->orig_window_size;

    for( i = 0; i < n; ++i )
    {
        cascade->stage_classifier[i].parent = cascade_src->stage_classifier[i].parent;
        cascade->stage_classifier[i].next = cascade_src->stage_classifier[i].next;
        cascade->stage_classifier[i].child = cascade_src->stage_classifier[i].child;
        cascade->stage_classifier[i].threshold = cascade_src->stage_classifier[i].threshold;

        cascade->stage_classifier[i].count = 0;
        CV_CALL( cascade->stage_classifier[i].classifier =
            (CvHaarClassifier*) cvAlloc( cascade_src->stage_classifier[i].count
                * sizeof( cascade->stage_classifier[i].classifier[0] ) ) );
        
        cascade->stage_classifier[i].count = cascade_src->stage_classifier[i].count;

        for( j = 0; j < cascade->stage_classifier[i].count; ++j )
        {
            cascade->stage_classifier[i].classifier[j].haar_feature = NULL;
        }

        for( j = 0; j < cascade->stage_classifier[i].count; ++j )
        {
            const CvHaarClassifier* classifier_src = 
                &cascade_src->stage_classifier[i].classifier[j];
            CvHaarClassifier* classifier = 
                &cascade->stage_classifier[i].classifier[j];

            classifier->count = classifier_src->count;
            CV_CALL( classifier->haar_feature = (CvHaarFeature*) cvAlloc( 
                classifier->count * ( sizeof( *classifier->haar_feature ) +
                                      sizeof( *classifier->threshold ) +
                                      sizeof( *classifier->left ) +
                                      sizeof( *classifier->right ) ) +
                (classifier->count + 1) * sizeof( *classifier->alpha ) ) );
            classifier->threshold = (int*) (classifier->haar_feature+classifier->count);
            classifier->left = (int*) (classifier->threshold + classifier->count);
            classifier->right = (int*) (classifier->left + classifier->count);
            classifier->alpha = (int*) (classifier->right + classifier->count);
            for( k = 0; k < classifier->count; ++k )
            {
                classifier->haar_feature[k] = classifier_src->haar_feature[k];
                classifier->threshold[k] = classifier_src->threshold[k];
                classifier->left[k] = classifier_src->left[k];
                classifier->right[k] = classifier_src->right[k];
                classifier->alpha[k] = classifier_src->alpha[k];
            }
            classifier->alpha[classifier->count] = 
                classifier_src->alpha[classifier->count];
        }
    }

    __END__;

    return cascade;
}


CvType haar_type( CV_TYPE_NAME_HAAR, icvIsHaarClassifier,
                  (CvReleaseFunc)cvReleaseHaarClassifierCascade,
                  icvReadHaarClassifier, icvWriteHaarClassifier,
                  icvCloneHaarClassifier );

/* End of file. */
