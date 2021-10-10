#ifdef HAVE_CONFIG_H
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/msfilewriter.h"
#include "mediastreamer2/msvideo.h"
#include "ffmpeg-priv.h"

//for H264 data //Benson
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
//for H264 data. //Benson

#ifdef WIN32
#include <malloc.h>
#endif
#include "stdio.h"
#include "ite/itp.h"
#include "fat/fat.h"
#include "ite/ite_sd.h"

#ifndef FALSE
#   define FALSE 0
#endif
#ifndef TRUE
#   define TRUE 1
#endif

static void FileWriter_callback_default()
{
}
void (*FileWriter_callback)(void *arg) = FileWriter_callback_default;

typedef struct {
	FILE *file;
	AVCodec *codec;
    mblk_t  *compressed_data;
}FileWriter;


static void Snapshot_success(void *arg)
{
	if(FileWriter_callback)
    	FileWriter_callback(arg);
}

static void file_writer_init(MSFilter  *f)
{
    FileWriter *s = ms_new0(FileWriter, 1);
    printf("file_writer_init\n");
}

static void file_writer_uninit(MSFilter *f)
{
    FileWriter *s = (FileWriter *)f->data;
    return ;
}

static void file_writer_preprocess(MSFilter *f)
{
    return ;
}

static void file_writer_process(MSFilter *f)
{
    FileWriter *d = (FileWriter *)f->data;
    mblk_t  *im;
    uint8_t *GetStream = NULL;  
    char    *Filename  = NULL;
    uint32_t  GetFilesize =0;
    FILE     *fout = 0;

    if ((im = ms_queue_get(f->inputs[0])) != NULL)
    {
        Filename  = im->b_rptr;
        im->b_rptr += DEF_FileStream_Name_LENGTH;

        GetStream    = im->b_rptr;
        GetFilesize  = im->b_wptr - im->b_rptr ;

        if( (fout = fopen(Filename, "wb")))
        {
            fwrite(GetStream, 1 ,GetFilesize, fout);
            fclose(fout);
			
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
    ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
#endif
            printf("file_write success\n");
            Snapshot_success((void *)Filename);
        }
        else
            printf("open savefile fail !!\n");

        if(im) freemsg(im);
    }
    else
    {
        if(im) freemsg(im);
    }
    return ;
}


static MSFilterMethod methods[] = {
    {   0, NULL}
};

#ifdef _MSC_VER

MSFilterDesc ms_file_writer_desc = {
    MS_FILE_WRITER_ID,
    "MSFileWriter",
    N_("Encode jpeg stream to jpg file"),
    MS_FILTER_OTHER,
    NULL,
    1,
    0,
    file_writer_init,
    file_writer_preprocess,
    file_writer_process,
    NULL,
    file_writer_uninit,
    methods
};

#else

MSFilterDesc ms_file_writer_desc = {
    .id         = MS_FILE_WRITER_ID,
    .name       = "MSFileWriter",
    .text       = N_("Encode jpeg stream to jpg file"),
    .category   = MS_FILTER_OTHER,
    .ninputs    = 1,
    .noutputs   = 0,
    .init       = file_writer_init,
    .preprocess = file_writer_preprocess,
    .process    = file_writer_process,
    .uninit     = file_writer_uninit,
    .methods    = methods
};

#endif

MS_FILTER_DESC_EXPORT(ms_file_writer_desc)
