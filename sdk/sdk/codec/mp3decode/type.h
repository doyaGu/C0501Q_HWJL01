
#ifndef __TYPE_H__
#define __TYPE_H__

typedef struct _STREAMBUF
{
    unsigned char *buf;
    int rdptr, wtptr;
    int length;
    int eof;
} STREAMBUF;

typedef struct _SAMPBUF
{
    int *buf;
    int nch;
    int nsamples;
} SAMPBUF;

typedef struct _param_eq_struct{
    int enable;
    short int bandcenter[16];
    short int dbtab[16];
} param_eq_struct;

typedef struct _param_reverb_struct{
    int enable;
    int src_gain;
    int reverb_gain;
    int delay[3];
    int gain[3];
} param_reverb_struct;

typedef struct _param_drc_struct{
    int enable;
    int digital_gain;       // range from [0 ~ 1000]%, ex. 100% : volume unchanged
} param_drc_struct;

typedef struct _param_voff_struct{
    int enable;
} param_voff_struct;

typedef struct _param_struct{
    param_eq_struct         param_eq;
    param_reverb_struct     param_reverb;
    param_drc_struct        param_drc;
    param_voff_struct       param_voff;
} param_struct;

#endif // __TYPE_H__

