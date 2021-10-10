/*
 * audio resampling
 * Copyright (c) 2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * audio resampling
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

//#include "avcodec.h"
//#include "dsputil.h"
#include <stdio.h>
#ifdef CFG_AUDIO_MGR_RESAMPLE

//#include <assert.h>
//#include "math.h"
//#include "coder.h"
#include "resample.h"
//#include "resample48k.h"
//#include "resample44k.h"
//#include "resample32k.h"
//#include "resample22k.h"
//#include "resample8k.h"
#include "resample8k_16.h"
#include "resample16k_16.h"

#ifndef CONFIG_RESAMPLE_HP
#define FILTER_SHIFT 15

#ifndef INT16_MIN
#define INT16_MIN       (-0x7fff - 1)
#endif

#ifndef INT16_MAX
#define INT16_MAX       0x7fff
#endif

    #if !defined(WIN32)
        typedef signed long long  int64_t;
        #define FELEM  signed short
        #define FELEM2 signed int 
        #define FELEML signed long long   
        #define FELEM_MAX INT16_MAX
        #define FELEM_MIN INT16_MIN
        #define WINDOW_TYPE 9
    #else
typedef signed __int64       int64_t;

#define FELEM  signed __int16
#define FELEM2 signed __int32
#define FELEML signed __int64 
#define FELEM_MAX INT16_MAX
#define FELEM_MIN INT16_MIN
#define WINDOW_TYPE 9
    #endif

#elif !defined(CONFIG_RESAMPLE_AUDIOPHILE_KIDDY_MODE)
#define FILTER_SHIFT 30

#define FELEM int32_t
#define FELEM2 int64_t
#define FELEML int64_t
#define FELEM_MAX INT32_MAX
#define FELEM_MIN INT32_MIN
#define WINDOW_TYPE 12
#else
#define FILTER_SHIFT 0

#define FELEM double
#define FELEM2 double
#define FELEML double
#define WINDOW_TYPE 24
#endif
//ERROR
#define ENOMEM          12

#define CEILING_POS(X) ((X-(int)(X)) > 0 ? (int)(X+1) : (int)(X))
#define CEILING_NEG(X) ((X-(int)(X)) < 0 ? (int)(X-1) : (int)(X))
#define CEILING(X) ( ((X) > 0) ? CEILING_POS(X) : CEILING_NEG(X) )

#define M_PI       3.14159265358979323846
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))




AVResampleContext gAVResample;
extern FILE *ftable;

#if 1

#else
static __inline__  int lrintf(float flt)
{	
    int intgr;
	_asm
	{	
	    fld flt
		fistp intgr
	};
	return intgr;
}
#endif

/**
 * Clip a signed integer value into the amin-amax range.
 * @param a value to clip
 * @param amin minimum value of the clip range
 * @param amax maximum value of the clip range
 * @return clipped value
 */
static __inline int av_clip(int a, int amin, int amax)
{
    if      (a < amin) return amin;
    else if (a > amax) return amax;
    else               return a;
}

/**
 * 0th order modified bessel function of the first kind.
 */
static double bessel(double x){
    double v=1;
    double lastv=0;
    double t=1;
    int i;

    x= x*x/4;
    for(i=1; v != lastv; i++){
        lastv=v;
        t *= x/(i*i);
        v += t;
    }
    return v;
}

/**
 * builds a polyphase filterbank.
 * @param factor resampling factor
 * @param scale wanted sum of coefficients for each filter
 * @param type 0->cubic, 1->blackman nuttall windowed sinc, 2..16->kaiser windowed sinc beta=2..16
 * @return 0 on success, negative on error
 */
static int build_filter(FELEM *filter, double factor, int tap_count, int phase_count, int scale, int type){
    int ph, i;
    double x, y, w;
    //double *tab = av_malloc(tap_count * sizeof(*tab));
    double tab[22]; //16
    const int center= (tap_count-1)/2;

    if (!tab)
        return -(ENOMEM);

    /* if upsampling, only need to interpolate, no filter */
    if (factor > 1.0)
        factor = 1.0;

    for(ph=0;ph<phase_count;ph++) {
        double norm = 0;
        for(i=0;i<tap_count;i++) {
            x = M_PI * ((double)(i - center) - (double)ph / phase_count) * factor;
            if (x == 0) y = 1.0;
            else        y = sin(x) / x;
            switch(type){
            case 0:{
                const float d= -0.5; //first order derivative = -0.5
                x = fabs(((double)(i - center) - (double)ph / phase_count) * factor);
                if(x<1.0) y= 1 - 3*x*x + 2*x*x*x + d*(            -x*x + x*x*x);
                else      y=                       d*(-4 + 8*x - 5*x*x + x*x*x);
                break;}
            case 1:
                w = 2.0*x / (factor*tap_count) + M_PI;
                y *= 0.3635819 - 0.4891775 * cos(w) + 0.1365995 * cos(2*w) - 0.0106411 * cos(3*w);
                break;
            default:
                w = 2.0*x / (factor*tap_count*M_PI);
                y *= bessel(type*sqrt(FFMAX(1-w*w, 0)));
                break;
            }

            tab[i] = y;
            norm += y;
        }

        /* normalize so that an uniform color remains the same */
        for(i=0;i<tap_count;i++) {
#ifdef CONFIG_RESAMPLE_AUDIOPHILE_KIDDY_MODE
            filter[ph * tap_count + i] = tab[i] / norm;
#else
            filter[ph * tap_count + i] = av_clip(lrintf(tab[i] * scale / norm), FELEM_MIN, FELEM_MAX);
#endif
        }
    }
#if 0
    {
#define LEN 1024
        int j,k;
        double sine[LEN + tap_count];
        double filtered[LEN];
        double maxff=-2, minff=2, maxsf=-2, minsf=2;
        for(i=0; i<LEN; i++){
            double ss=0, sf=0, ff=0;
            for(j=0; j<LEN+tap_count; j++)
                sine[j]= cos(i*j*M_PI/LEN);
            for(j=0; j<LEN; j++){
                double sum=0;
                ph=0;
                for(k=0; k<tap_count; k++)
                    sum += filter[ph * tap_count + k] * sine[k+j];
                filtered[j]= sum / (1<<FILTER_SHIFT);
                ss+= sine[j + center] * sine[j + center];
                ff+= filtered[j] * filtered[j];
                sf+= sine[j + center] * filtered[j];
            }
            ss= sqrt(2*ss/LEN);
            ff= sqrt(2*ff/LEN);
            sf= 2*sf/LEN;
            maxff= FFMAX(maxff, ff);
            minff= FFMIN(minff, ff);
            maxsf= FFMAX(maxsf, sf);
            minsf= FFMIN(minsf, sf);
            if(i%11==0){
                av_log(NULL, AV_LOG_ERROR, "i:%4d ss:%f ff:%13.6e-%13.6e sf:%13.6e-%13.6e\n", i, ss, maxff, minff, maxsf, minsf);
                minff=minsf= 2;
                maxff=maxsf= -2;
            }
        }
    }
#endif

    //av_free(tab);
    return 0;
}
/*
 av_audio_resample_init( 
		2, ctx->channels, 44100, ctx->sample_rate,
        AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16, 16,
        10, 0, 0.8);
s->resample_context = av_resample_init(output_rate, input_rate,
                                           filter_length//16, log2_phase_count//10,
                                           linear//0, cutoff//0.8);
*/
void *av_resample_init(AVResampleContext *avResampleContext,int out_rate, int in_rate, int filter_size, int phase_shift, int linear, double cutoff){
    AVResampleContext *c= avResampleContext;//av_mallocz(sizeof(AVResampleContext));
    double factor= FFMIN(out_rate * cutoff / in_rate, 1.0);
    int phase_count= 1<<phase_shift;
    int i,j;
    if (!c)
        return 0;

    c->phase_shift= phase_shift;
    c->phase_mask= phase_count-1;
    c->linear= linear;
#ifdef WIN32
    c->filter_length= FFMAX((int)ceil(filter_size/factor), 1);
#else
    c->filter_length= FFMAX((int)CEILING(filter_size/factor), 1);
#endif
#ifdef LOOKUP_TABLE
    if (in_rate==44100) {
        // 44100 
//        c->filter_bank = (short*)&filter44k[0];
    } else if (in_rate==32000){
        // 32000 
//        c->filter_bank = (short*)&filter32k[0];
    } else if (in_rate==8000){
        c->filter_bank = (short*)&filter8k[0];
    } else if (in_rate==16000){
        c->filter_bank = (short*)&filter16k[0];
    }   
#else
    c->filter_bank;//= av_mallocz(c->filter_length*(phase_count+1)*sizeof(FELEM));
    if (!c->filter_bank)
        goto error;
    if (build_filter(c->filter_bank, factor, c->filter_length, phase_count, 1<<FILTER_SHIFT, WINDOW_TYPE))
        goto error;
    memcpy(&c->filter_bank[c->filter_length*phase_count+1], c->filter_bank, (c->filter_length-1)*sizeof(FELEM));
    c->filter_bank[c->filter_length*phase_count]= c->filter_bank[c->filter_length - 1];
#endif

#if 0   
    for (i=0;i<11275*2;i+=10)
    {
        fprintf(ftable,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",c->filter_bank[i+0],c->filter_bank[i+1],c->filter_bank[i+2],c->filter_bank[i+3],c->filter_bank[i+4],c->filter_bank[i+5],c->filter_bank[i+6],c->filter_bank[i+7],c->filter_bank[i+8],c->filter_bank[i+9]);
    }
    fclose(ftable);
#endif
    c->src_incr= out_rate;
    c->ideal_dst_incr= c->dst_incr= in_rate * phase_count;
    c->index= -phase_count*((c->filter_length-1)/2);

    return c;
error:
    //av_free(c->filter_bank);
    //av_free(c);
    return 0;
}

void av_resample_close(AVResampleContext *c){
    //av_freep(&c->filter_bank);
    //av_freep(&c);
}

void av_resample_compensate(AVResampleContext *c, int sample_delta, int compensation_distance){
//    sample_delta += (c->ideal_dst_incr - c->dst_incr)*(int64_t)c->compensation_distance / c->ideal_dst_incr;
    c->compensation_distance= compensation_distance;
    c->dst_incr = c->ideal_dst_incr - c->ideal_dst_incr * (signed long long)sample_delta / compensation_distance;
}

int av_resample(AVResampleContext *c, short *dst, short *src, int *consumed, int src_size, int dst_size, int update_ctx){
    int dst_index, i;
    int index= c->index;
    int frac= c->frac;
    int dst_incr_frac= c->dst_incr % c->src_incr;
    int dst_incr=      c->dst_incr / c->src_incr;
    int compensation_distance= c->compensation_distance;

  if(compensation_distance == 0 && c->filter_length == 1 && c->phase_shift==0)
  {
        int64_t index2= ((int64_t)index)<<32;
        int64_t incr= (1LL<<32) * c->dst_incr / c->src_incr;
        dst_size= FFMIN(dst_size, (src_size-1-index) * (int64_t)c->src_incr / c->dst_incr);

        for(dst_index=0; dst_index < dst_size; dst_index++){
            dst[dst_index] = src[index2>>32];
            index2 += incr;
        }
        frac += dst_index * dst_incr_frac;
        index += dst_index * dst_incr;
        index += frac / c->src_incr;
        frac %= c->src_incr;
  }
  else
  {
    for(dst_index=0; dst_index < dst_size; dst_index++)
    {
        FELEM *filter= c->filter_bank + MULSHIFT32(c->filter_length,(index & c->phase_mask));
        int sample_index= index >> c->phase_shift;
        FELEM2 val=0;

        if(sample_index < 0)
        {
#if 1
            for(i=0; i<c->filter_length; i++)
                val += src[FFABS(sample_index + i) % src_size] * filter[i];
#else
            for(i=0; i<c->filter_length; i++)
            {
                val += MULSHIFT(src[FASTABS(sample_index + i) % src_size],filter[i],0);
            }
#endif
        }
        else if(sample_index + c->filter_length > src_size)
        {
            break;
        }
        else if(c->linear)
        {
            FELEM2 v2=0;
            for(i=0; i<c->filter_length; i++){
                val += src[sample_index + i] * (FELEM2)filter[i];
                v2  += src[sample_index + i] * (FELEM2)filter[i + c->filter_length];
            }
            val+=(v2-val)*(FELEML)frac / c->src_incr;
        }
        else
        {
            for(i=0; i<c->filter_length; i++)
            {
                val += src[sample_index + i] * (FELEM2)filter[i];
                //val += MULSHIFT(src[sample_index + i],filter[i],0);
            }
        }

#ifdef CONFIG_RESAMPLE_AUDIOPHILE_KIDDY_MODE
        dst[dst_index] = av_clip_int16(lrintf(val));
#else
        val = (val + (1<<(FILTER_SHIFT-1)))>>FILTER_SHIFT;
        dst[dst_index] = (unsigned)(val + 32768) > 65535 ? (val>>31) ^ 32767 : val;
#endif

        frac += dst_incr_frac;
        index += dst_incr;
        if(frac >= c->src_incr){
            frac -= c->src_incr;
            index++;
        }

        if(dst_index + 1 == compensation_distance){
            compensation_distance= 0;
            dst_incr_frac= c->ideal_dst_incr % c->src_incr;
            dst_incr=      c->ideal_dst_incr / c->src_incr;
        }
    }
  }
    *consumed= FFMAX(index, 0) >> c->phase_shift;
    if(index>=0) index &= c->phase_mask;

    if(compensation_distance){
        compensation_distance -= dst_index;
        //assert(compensation_distance > 0);
    }
    if(update_ctx){
        c->frac= frac;
        c->index= index;
        c->dst_incr= dst_incr_frac + c->src_incr*dst_incr;
        c->compensation_distance= compensation_distance;
    }
#if 0
    if(update_ctx && !c->compensation_distance){
#undef rand
        av_resample_compensate(c, rand() % (8000*2) - 8000, 8000*2);
av_log(NULL, AV_LOG_DEBUG, "%d %d %d\n", c->dst_incr, c->ideal_dst_incr, c->compensation_distance);
    }
#endif

    return dst_index;
}

#endif

