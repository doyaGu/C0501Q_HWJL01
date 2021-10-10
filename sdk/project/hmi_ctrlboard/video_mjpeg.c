#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include "ite/itu.h"
#include "itu_private.h"
#include "castor3player.h"
#include "jpg/ite_jpg.h"
#include "ite/itv.h"

#define BOOT_VIDEO CFG_BOOT_MJPEG_FILE_NAME
#define InputQueueNumber 60

#define RIFF		0x46464952      /* "RIFF" */
#define WAVE		0x45564157      /* "WAVE" */
#define FACT		0x74636166      /* "fact" */
#define LIST		0x5453494c      /* "LIST" */
#define JUNK		0x4B4E554A      /* "JUNK" */
#define MOVI		0x69766f6d		 /* "MOVI" */
#define VIDS00DC	0x63643030		 /* "00DC" */
#define AUDS01WB	0x62773130		 /* "01WB" */
#define IDX1		0x31786469		 /* "IDX1" */
#define HDRL		0x6C726468		 /* "HDRL" */
#define AVIH		0x68697661		 /* "AVIH" */
#define fourChar(x,y,z,w) ( ((w)<<24)|((z)<<16)|((y)<<8)|(x) )/*little-endian*/

typedef struct
{
    uint32_t             dwMicroSecPerFrame;    
    uint32_t             dwMaxBytesPerSec;      
    uint32_t             dwPaddingGranularity;	
	uint32_t             dwFlags;     
    uint32_t             dwTotalFrames;           
    uint32_t             dwInitialFrames;  
    uint32_t             dwStreams;  
	uint32_t             dwSuggestedBufferSize;
    uint32_t             dwWidth;
    uint32_t             dwHeight;
    uint32_t             dwReserved[4];

} MainAVIHeader;

typedef void (*pfPktReleaseCb) (void *pkt);

typedef struct JpegInputPkt {
    uint8_t *pInputBuffer;
    uint32_t bufferSize;
} JpegInputPkt;

typedef struct QueuePktList {
    void* pkt;
    struct QueuePktList *next;
} QueuePktList;

typedef struct PktQueue {
    QueuePktList *firstPkt, *lastPkt;
    int numPackets;
    int maxPackets;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pfPktReleaseCb pfPktRelease;
} PktQueue;

typedef struct JPG_DECODER_TAG
{
    uint32_t	framePitchY;
    uint32_t	framePitchUV;
    uint32_t	frameWidth;
    uint32_t	frameHeight;
    uint32_t	frameBufCount;
	uint32_t    currDisplayFrameBufIndex;
    uint32_t	OutAddrY[2];
    uint32_t	OutAddrU[2];
    uint32_t	OutAddrV[2];
	uint8_t     *DisplayAddrY;
    uint8_t     *DisplayAddrU;
    uint8_t     *DisplayAddrV;
	JPG_COLOR_SPACE     colorFmt;
} JPG_DECODER;

typedef struct _VideoWindow {
    ITUSurface* lcdSurf;
    int x_pos;
    int y_pos;
    int width;
    int height;
    int lcdBgColor;
} VideoWindow;

static VideoWindow *vwindow = NULL; 
static pthread_t tid , tidDeMjpeg , tidAVIOpen;
static bool 	 videoPlayerIsFileEOF = false;
extern bool 	 bPlayingBootVideo;
extern bool      isOtherVideoPlaying;

static PktQueue        gMJpegInputQueue = { 0 };
static PktQueue        gMJpegOutputQueue = { 0 };
static int             gbAbortFlag = 0;

// ite H/W jpg
static HJPG			 *pHJpeg = 0;
static JPG_DECODER    	*gptJPG_DECODER     = NULL;
static uint32_t Jbuf_vram_addr  	= 0;
static uint8_t* Jbuf_sys_addr   	= NULL;
static uint32_t int_count =0 , dec_count=0;
static pthread_mutex_t JDecMutex  = PTHREAD_MUTEX_INITIALIZER;
static MainAVIHeader MainAVIH = {0};

//BG
extern ITUSurface      *VideoSurf[2];
int gx,  gy,  gwidth, gheight,  gbgColor;

static pfPktReleaseCb
_jpegInputPktRelease(
    void* pkt)
{
    JpegInputPkt *ptJpegInputPkt = (JpegInputPkt*)pkt;
    if (ptJpegInputPkt && ptJpegInputPkt->pInputBuffer && ptJpegInputPkt->bufferSize)
    {
        free(ptJpegInputPkt->pInputBuffer);
    }
    free(pkt);
}

static int
_packetQueuePut(
    PktQueue *q,
    void *pkt)
{
    QueuePktList *pkt1;

    /* duplicate the packet */
	
    if (q->mutex)
    {
    	while(q->numPackets > q->maxPackets)
		{
			usleep(1000*100);
		}
        pkt1 = malloc(sizeof(QueuePktList));
        if (!pkt1) return -1;
        pkt1->pkt =  pkt;
        pkt1->next = NULL;

        pthread_mutex_lock(&q->mutex);

        if (!q->lastPkt)
            q->firstPkt = pkt1;
        else
            q->lastPkt->next = pkt1;
        q->lastPkt = pkt1;
        q->numPackets++;

        /* XXX: should duplicate packet data in DV case */
        pthread_cond_signal(&q->cond);
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }
    else
    {
        return -1;
    }
}

/* packet queue handling */
static void
_packetQueueInit(
    PktQueue *q,
    void* pfPktRelease,
    int maxPackets)
{
    memset(q, 0, sizeof(PktQueue));
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    q->maxPackets = maxPackets;
    q->pfPktRelease = pfPktRelease;
}

static void
_packetQueueFlush(
    PktQueue *q)
{
    QueuePktList *pkt, *pkt1;
    if (q->mutex)
    {
        pthread_mutex_lock(&q->mutex);
        for (pkt = q->firstPkt; pkt != NULL; pkt = pkt1) {
            pkt1 = pkt->next;
            if (q->pfPktRelease)
            {
                q->pfPktRelease(pkt->pkt);
            }
            free(pkt);
        }
        q->lastPkt = NULL;
        q->firstPkt = NULL;
        q->numPackets = 0;
        pthread_mutex_unlock(&q->mutex);
    }
}

static void
_packetQueueEnd(
    PktQueue *q)
{
    if (q->mutex)
    {
        _packetQueueFlush(q);
        pthread_mutex_destroy(&q->mutex);
        pthread_cond_destroy(&q->cond);
        memset(q, 0, sizeof(PktQueue));
    }
}

static int
_packetQueueGet(
    PktQueue *q,
    void **pkt,
    int block)
{
    QueuePktList *pkt1;
    int ret;
    if (q->mutex)
    {
        pthread_mutex_lock(&q->mutex);
		
        for (;;)
        {
            pkt1 = q->firstPkt;
            if (pkt1)
            {
            
                q->firstPkt = pkt1->next;
                if (!q->firstPkt)
                    q->lastPkt = NULL;
                q->numPackets--;
                *pkt = pkt1->pkt;
                free(pkt1);
                ret = 1;
                break;
            }
            else if (!block)
            {
                ret = 0;
                break;
            }
            else
            {
                pthread_cond_wait(&q->cond, &q->mutex);
            }
        }
        pthread_mutex_unlock(&q->mutex);
        return ret;
    }
    else
    {
        return -1;
    }
}

static void
AVIFileStreamFree(
uint8_t* data)
{
	if(data) free(data);
}

static void
AVIFileStreamRead(
uint8_t* data,uint32_t size, uint32_t count, void *fp)
{
	int Result = 0;
	
	if(data)
	{   
		Result = fread(data, size, count, fp);
	}else printf("Mem open Fail!\n");

	return Result;
}

static uint8_t* 
AVIFileStreamMalloc(
unsigned int size)
{
	uint8_t *data =NULL;
	data = malloc(size); 
	
	return data;
}

static void
_mjpeg_decode_display(
	void)
{
	uint32_t	   frame_width, frame_height, frame_PitchY, frame_PitchUV;
	
	frame_width   = gptJPG_DECODER->frameWidth;
	frame_height  = gptJPG_DECODER->frameHeight;
	frame_PitchY  = gptJPG_DECODER->framePitchY;
	frame_PitchUV = gptJPG_DECODER->framePitchUV;

	if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 &&  ithIsTilingModeOn())
	{
		uint32_t real_height_ForTile;

		real_height_ForTile = (frame_height  + 0x7f)/ 0x80;

		if (!Jbuf_sys_addr)
		{
			Jbuf_vram_addr = itpVmemAlignedAlloc(128*1024,(frame_PitchY * 128 * real_height_ForTile * 3 ) ); //for YUV420
			if(!Jbuf_vram_addr) printf("Jbuf_sys_addr Alloc Buffer Fail!!\n");
			
			Jbuf_sys_addr = (uint8_t*) ithMapVram(Jbuf_vram_addr,(frame_PitchY * 128 * real_height_ForTile * 3 ) , ITH_VRAM_WRITE);
			gptJPG_DECODER->frameBufCount = 0;
			gptJPG_DECODER->currDisplayFrameBufIndex = 0;
		}

		if(!gptJPG_DECODER->frameBufCount)
		{
			gptJPG_DECODER->OutAddrY[0] = Jbuf_sys_addr;
			gptJPG_DECODER->OutAddrU[0] = gptJPG_DECODER->OutAddrY[0]  + (frame_PitchY * 128 * real_height_ForTile); 
			gptJPG_DECODER->OutAddrV[0] = gptJPG_DECODER->OutAddrU[0]  + (frame_PitchY * 128 * real_height_ForTile);

			gptJPG_DECODER->frameBufCount = 1;
		}
	}else
	{
		if (!Jbuf_sys_addr)
		{
			Jbuf_vram_addr = itpVmemAlignedAlloc(32,(frame_PitchY * frame_height * 4 ) ); //for YUV422
			if(!Jbuf_vram_addr) printf("Jbuf_sys_addr Alloc Buffer Fail!!\n");
			
			Jbuf_sys_addr = (uint8_t*) ithMapVram(Jbuf_vram_addr,(frame_PitchY * frame_height * 4 ) , ITH_VRAM_WRITE);
			gptJPG_DECODER->frameBufCount = 0;
			gptJPG_DECODER->currDisplayFrameBufIndex = 0;
		}

		if(!gptJPG_DECODER->frameBufCount)
		{
			gptJPG_DECODER->OutAddrY[0] = Jbuf_sys_addr;
			gptJPG_DECODER->OutAddrU[0] = gptJPG_DECODER->OutAddrY[0]  + (frame_PitchY  * frame_height); 
			gptJPG_DECODER->OutAddrV[0] = gptJPG_DECODER->OutAddrU[0]  + (frame_PitchUV * frame_height);

			gptJPG_DECODER->OutAddrY[1] = gptJPG_DECODER->OutAddrV[0]  + (frame_PitchY  * frame_height);
			gptJPG_DECODER->OutAddrU[1] = gptJPG_DECODER->OutAddrY[1]  + (frame_PitchUV * frame_height);
			gptJPG_DECODER->OutAddrV[1] = gptJPG_DECODER->OutAddrU[1]  + (frame_PitchUV * frame_height);
			gptJPG_DECODER->frameBufCount = 2;
		}
	}

	if(gptJPG_DECODER->frameBufCount ==1)
			gptJPG_DECODER->currDisplayFrameBufIndex = 0;

	switch (gptJPG_DECODER->currDisplayFrameBufIndex)
    {
	    case 0:
	        gptJPG_DECODER->DisplayAddrY = gptJPG_DECODER->OutAddrY[0];
	        gptJPG_DECODER->DisplayAddrU = gptJPG_DECODER->OutAddrU[0];
	        gptJPG_DECODER->DisplayAddrV = gptJPG_DECODER->OutAddrV[0];
	        break;

	    case 1:
	        gptJPG_DECODER->DisplayAddrY = gptJPG_DECODER->OutAddrY[1];
	        gptJPG_DECODER->DisplayAddrU = gptJPG_DECODER->OutAddrU[1];
	        gptJPG_DECODER->DisplayAddrV = gptJPG_DECODER->OutAddrV[1];
	        break;
    }

	if(gptJPG_DECODER->currDisplayFrameBufIndex >= 1) 	gptJPG_DECODER->currDisplayFrameBufIndex = 0;
		else gptJPG_DECODER->currDisplayFrameBufIndex++;	


}

static int
_mjpeg_decode_frame(
	unsigned char	*jpegStream,
	unsigned int	streamLength)
{
    int                result     = 0;
    JPG_ERR            jpgRst     = JPG_ERR_OK;
	JPG_RECT	       destRect   = {0};
	ITUColor           black      = { 0, 0, 0, 0 };
	ITUSurface		   *surf	  = NULL;
	uint8_t 		   *dest	  = NULL;

	
	 //pthread_mutex_lock(&JDecMutex);

    do
    {
        JPG_STREAM_INFO inStreamInfo   = {0};
        JPG_STREAM_INFO outStreamInfo  = {0};
        JPG_BUF_INFO    entropyBufInfo = {0};
        JPG_USER_INFO   jpgUserInfo    = {0};
		
		uint8_t            *dbuf       = NULL; 
    	ITV_DBUF_PROPERTY  dbufprop    = {0};
	
		
        // ------------------------------------
        // set src type
        inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
        inStreamInfo.streamType            = JPG_STREAM_MEM;
        inStreamInfo.jstream.mem[0].pAddr  = jpegStream;
        inStreamInfo.jstream.mem[0].length = streamLength;
        inStreamInfo.validCompCnt          = 1;
        iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);

        // ------------------------------------
        // parsing Header
        jpgRst = iteJpg_Parsing(pHJpeg, &entropyBufInfo,(void *)&destRect);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
            break;
        }

        // ----------------------------------------
        // get output YUV plan buffer
        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
     
		gptJPG_DECODER->frameHeight =  jpgUserInfo.real_height;
		gptJPG_DECODER->frameWidth  =  jpgUserInfo.real_width;
		gptJPG_DECODER->framePitchY  = jpgUserInfo.comp1Pitch;
		gptJPG_DECODER->framePitchUV = jpgUserInfo.comp23Pitch;
		gptJPG_DECODER->colorFmt 	 = jpgUserInfo.colorFormate;
		
        outStreamInfo.streamIOType         = JPG_STREAM_IO_WRITE;
        outStreamInfo.streamType           = JPG_STREAM_MEM;

		JPG_INIT_PARAM      *pJInitParam = (JPG_INIT_PARAM*)pHJpeg;
		if(pJInitParam->outColorSpace == JPG_COLOR_SPACE_RGB565)
		{
			int new_index = itv_get_vidSurf_index();
			while( new_index == -1)
			{
				//printf("wait to get new_index!\n");
				usleep(1000);
                new_index = itv_get_vidSurf_index();
			}

            switch(new_index)
            {
               case  0:
                    new_index =1;
               break;
               
               case  1:
               case -2:
                    new_index =0;
               break;
            }

			surf = VideoSurf[new_index];
			dest = (uint8_t *)ituLockSurface(surf, gx, gy, gwidth, gheight);
			ituColorFill(surf, gx, gy, gwidth, gheight, &black);
			outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)dest;     // get output buf;
	        outStreamInfo.jstream.mem[0].pitch  =  surf->pitch;
	        outStreamInfo.jstream.mem[0].length =  surf->pitch * surf->height; 
	        outStreamInfo.validCompCnt          = 1;
		}
		else
		{
			_mjpeg_decode_display();

			if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0 &&  ithIsTilingModeOn())
			{
				uint32_t real_height_ForTile;
				real_height_ForTile = (gptJPG_DECODER->frameHeight  + 0x7f)/ 0x80;

				outStreamInfo.jstream.mem[0].pAddr  = gptJPG_DECODER->DisplayAddrY; // get output buf;
		        outStreamInfo.jstream.mem[0].pitch  = gptJPG_DECODER->framePitchY;
		        outStreamInfo.jstream.mem[0].length = gptJPG_DECODER->framePitchY * 128 * real_height_ForTile;
		        // U
		        outStreamInfo.jstream.mem[1].pAddr  = gptJPG_DECODER->DisplayAddrU;
		        outStreamInfo.jstream.mem[1].pitch  = gptJPG_DECODER->framePitchUV;
		        outStreamInfo.jstream.mem[1].length = gptJPG_DECODER->framePitchUV * 128 * real_height_ForTile;
		        // V
		        outStreamInfo.jstream.mem[2].pAddr  = gptJPG_DECODER->DisplayAddrV;
		        outStreamInfo.jstream.mem[2].pitch  = gptJPG_DECODER->framePitchUV;
		        outStreamInfo.jstream.mem[1].length = gptJPG_DECODER->framePitchUV * 128 * real_height_ForTile;
			}else
			{
				outStreamInfo.jstream.mem[0].pAddr  = gptJPG_DECODER->DisplayAddrY; // get output buf;
		        outStreamInfo.jstream.mem[0].pitch  = gptJPG_DECODER->framePitchY;
		        outStreamInfo.jstream.mem[0].length = gptJPG_DECODER->framePitchY * gptJPG_DECODER->frameHeight;
		        // U
		        outStreamInfo.jstream.mem[1].pAddr  = gptJPG_DECODER->DisplayAddrU;
		        outStreamInfo.jstream.mem[1].pitch  = gptJPG_DECODER->framePitchUV;
		        outStreamInfo.jstream.mem[1].length = gptJPG_DECODER->framePitchUV * gptJPG_DECODER->frameHeight;
		        // V
		        outStreamInfo.jstream.mem[2].pAddr  = gptJPG_DECODER->DisplayAddrV;
		        outStreamInfo.jstream.mem[2].pitch  = gptJPG_DECODER->framePitchUV;
		        outStreamInfo.jstream.mem[2].length = gptJPG_DECODER->framePitchUV * gptJPG_DECODER->frameHeight;
			}
		}
		
		//printf("\n\tY=0x%x, u=0x%x, v=0x%x\n",
		//			outStreamInfo.jstream.mem[0].pAddr,
		//			outStreamInfo.jstream.mem[1].pAddr,
		//			outStreamInfo.jstream.mem[2].pAddr);

        outStreamInfo.validCompCnt         = 3;
        jpgRst                             = iteJpg_SetStreamInfo(pHJpeg, 0, &outStreamInfo, 0);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
            break;
        }

        // ------------------------------
        // setup jpg
        jpgRst = iteJpg_Setup(pHJpeg, 0);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
            break;
        }

        // ------------------------------
        // fire H/W jpg
        jpgRst = iteJpg_Process(pHJpeg, &entropyBufInfo, 0, 0);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
            break;
        }
		
		iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0); 
	   	//printf("\n\tresult = %d\n", jpgUserInfo.status); 

        jpgRst = iteJpg_WaitIdle(pHJpeg, 0);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
            break;
        }
		
		jpgRst = iteJpg_Reset(pHJpeg,0);
		if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
            result = -1;
            break;
        }
		
		while( (dbuf = itv_get_dbuf_anchor()) == NULL)
		{
			//printf("wait to get dbuf!\n");
			usleep(1000);
		}
		if(pJInitParam->outColorSpace == JPG_COLOR_SPACE_RGB565)
		{
			// ------------------------------------
			// Flip LCD for handshake mode.
			dbufprop.src_w    = 0;
			dbufprop.src_h    = 0;
			dbufprop.pitch_y  = 0;
			dbufprop.pitch_uv = 0;
			dbufprop.format   = MMP_ISP_IN_RGB565;
			dbufprop.ya       = 0;
			dbufprop.ua       = 0;
			dbufprop.va       = 0;
			dbufprop.bidx     = 0;
			//printf("dbufprop.ya=0x%x,dbufprop.ua=0x%x,dbufprop.va=0x%x,dbufprop.src_w=%d,dbufprop.src_h=%d,dbufprop.pitch_y=%d,dbufprop.pitch_uv=%d,dbufprop.format=%d\n",dbufprop.ya,dbufprop.ua,dbufprop.va,dbufprop.src_w,dbufprop.src_h,dbufprop.pitch_y,dbufprop.pitch_uv,dbufprop.format);
			itv_update_dbuf_anchor(&dbufprop);
		}
		else
		{
			// ------------------------------------
			// Flip LCD for command trigger mode.
			dbufprop.src_w    = gptJPG_DECODER->frameWidth;
			dbufprop.src_h    = gptJPG_DECODER->frameHeight;
			dbufprop.pitch_y  = outStreamInfo.jstream.mem[0].pitch;
			dbufprop.pitch_uv = outStreamInfo.jstream.mem[1].pitch;
			dbufprop.format   = MMP_ISP_IN_YUV422;//(ithIsTilingModeOn()) ? MMP_ISP_IN_YUV422 : MMP_ISP_IN_YUV420;
			dbufprop.ya       = outStreamInfo.jstream.mem[0].pAddr;
			dbufprop.ua       = outStreamInfo.jstream.mem[1].pAddr;
			dbufprop.va       = outStreamInfo.jstream.mem[2].pAddr;
			dbufprop.bidx     = 0;
			//printf("dbufprop.ya=0x%x,dbufprop.ua=0x%x,dbufprop.va=0x%x,dbufprop.src_w=%d,dbufprop.src_h=%d,dbufprop.pitch_y=%d,dbufprop.pitch_uv=%d,dbufprop.format=%d\n",dbufprop.ya,dbufprop.ua,dbufprop.va,dbufprop.src_w,dbufprop.src_h,dbufprop.pitch_y,dbufprop.pitch_uv,dbufprop.format);
			itv_update_dbuf_anchor(&dbufprop);
		}
		dec_count++;  
    } while (0);

    if (result < 0)
    {
        // reset H/W jpg
        printf("result =%d\n");
        iteJpg_Reset(pHJpeg, 0);
    }

	if(surf)
	{
		ituUnlockSurface(surf);
	}

	//pthread_mutex_unlock(&JDecMutex);
    return streamLength;
}


static int
_mjpeg_decode_init(
    void)
{
    int                rst        = 0;
    JPG_ERR            jpgRst     = JPG_ERR_OK;
    JPG_INIT_PARAM     initParam  = {0};

	if (NULL == gptJPG_DECODER)
	gptJPG_DECODER = (JPG_DECODER *)calloc(sizeof(char), sizeof(JPG_DECODER)); // for jpg engine

    initParam.codecType     = JPG_CODEC_DEC_MJPG;
    initParam.decType       = JPG_DEC_PRIMARY;
    //initParam.outColorSpace = JPG_COLOR_SPACE_YUV420;
	initParam.outColorSpace = JPG_COLOR_SPACE_RGB565;

#if !defined (CFG_ENABLE_ROTATE) && !defined (CFG_BOOT_VIDEO_ENABLE_WINDOW_MODE)
	initParam.dispMode      = JPG_DISP_CENTER; 
#else
	initParam.dispMode      = JPG_DISP_FIT;
#endif 
	initParam.width         = gwidth;//ithLcdGetWidth();
    initParam.height        = gheight;//ithLcdGetHeight();

    iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

    if (!pHJpeg)
    {
        printf(" err ! create mjpg dec handle fail ! %s [%d]\n", __FILE__, __LINE__);
        rst = -1;
    }

    return rst;
}

static int
_mjpeg_decode_end(
    void)
{
    JPG_ERR            result     = JPG_ERR_OK;

    iteJpg_DestroyHandle(&pHJpeg, 0);
	pHJpeg =0;
	if (Jbuf_sys_addr)
    {
        itpVmemFree(Jbuf_vram_addr);
        Jbuf_sys_addr  = NULL;
        Jbuf_vram_addr = 0;
    }
	if (gptJPG_DECODER)
	{
        free(gptJPG_DECODER);
    	gptJPG_DECODER = NULL;
	}

    return 0;
}

static void*
AVIFileOpen(
 void *arg)
{
	uint8_t 		*data_Chunk = 0 ,*data_stream =0 ,*pCur = 0;
	uint32_t        CurCount       = 0, FourCC = 0, LIST_Size = 0 , LIST_Container=0 ,Marker_Size=0;
	uint32_t		frame_DataSize	=0, AVIH_Size=0 ,timeout_ms = 1000;
	FILE 			*f = 0;
	char 			*file_path = (char*)arg;
	
	f = fopen(file_path, "rb");
	//printf("file_path=%s\n",file_path);
	
	if(f)
	{  
		data_Chunk = AVIFileStreamMalloc(88); //RIFF AVI file first header 88 Bytes
		AVIFileStreamRead(data_Chunk , 1 ,88 ,f);

		pCur   = data_Chunk;
		FourCC = fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3));
		if(FourCC != RIFF)
		{
			printf("This is not AVI File !!\n");
			AVIFileStreamFree(data_Chunk);
			pCur = NULL;
			goto end;
		}
		pCur += 20;	
		
	}else printf("AVI file open fail!!!\n");

while(!videoPlayerIsFileEOF )
{
	while(pCur)
	{
		FourCC = fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3));

		switch(FourCC)
		{
			case RIFF:
		 		pCur    += 12;
			break;
			case LIST:
				pCur	+= 4;
				LIST_Size			= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3));
				pCur	+= 4;
				
				AVIFileStreamFree(data_Chunk);
				data_Chunk = AVIFileStreamMalloc(8); //LIST fourcc name and 4 bytes Size
				AVIFileStreamRead(data_Chunk , 1 , 8 ,f);
				pCur   = data_Chunk;				
				
				LIST_Container 		= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3));
				if(LIST_Container != MOVI && LIST_Container != HDRL)
				{
					fseek(f ,LIST_Size-8 , SEEK_CUR);
	
					AVIFileStreamFree(data_Chunk);
					data_Chunk = AVIFileStreamMalloc(8);  //LIST fourcc name and 4 bytes Size
					AVIFileStreamRead(data_Chunk , 1 , 8 ,f);
					pCur   = data_Chunk;								
				}
			break;
			case AVIH:
				pCur	+= 4;
				
				AVIH_Size	= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3));
				pCur	+= 4;

				MainAVIH.dwMicroSecPerFrame 	= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;
				MainAVIH.dwMaxBytesPerSec   	= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;
				MainAVIH.dwPaddingGranularity   = fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;
				MainAVIH.dwFlags   				= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;
				MainAVIH.dwTotalFrames   		= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;
				MainAVIH.dwInitialFrames   		= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;
				MainAVIH.dwStreams   			= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;
				MainAVIH.dwSuggestedBufferSize  = fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;
				MainAVIH.dwWidth   				= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;
				MainAVIH.dwHeight   			= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;
				MainAVIH.dwReserved[0]   		= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;				
				MainAVIH.dwReserved[1]   		= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;	
				MainAVIH.dwReserved[2]   		= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;	
				MainAVIH.dwReserved[3]   		= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3)); pCur	+= 4;	

				//printf("dwMicroSecPerFrame=0x%x,dwMaxBytesPerSec=0x%x\n",MainAVIH.dwMicroSecPerFrame,MainAVIH.dwMaxBytesPerSec );
				//printf("dwPaddingGranularity=0x%x,dwFlags=0x%x\n",MainAVIH.dwPaddingGranularity,MainAVIH.dwFlags );
				//printf("dwTotalFrames=0x%x,dwInitialFrames=0x%x\n",MainAVIH.dwTotalFrames,MainAVIH.dwInitialFrames );
				//printf("dwStreams=0x%x,dwSuggestedBufferSize=0x%x\n",MainAVIH.dwStreams,MainAVIH.dwSuggestedBufferSize );
				//printf("dwWidth=0x%x,dwHeight=0x%x\n",MainAVIH.dwWidth,MainAVIH.dwHeight );
				//printf("dwReserved[0]=0x%x,dwReserved[1]=0x%x\n",MainAVIH.dwReserved[0],MainAVIH.dwReserved[1] );
				//printf("dwReserved[2]=0x%x,dwReserved[3]=0x%x\n",MainAVIH.dwReserved[2],MainAVIH.dwReserved[3] );
			
				AVIFileStreamFree(data_Chunk);
				data_Chunk = AVIFileStreamMalloc(8); //LIST fourcc name and 4 bytes Size
				AVIFileStreamRead(data_Chunk , 1 , 8 ,f);
				pCur   = data_Chunk;			
				
			break;
			case HDRL:
			case MOVI:
				pCur += 4;
			break;
			case VIDS00DC:
				pCur	+= 4;
				
				AVIFileStreamFree(data_Chunk);
				data_Chunk = AVIFileStreamMalloc(4); //Read 4bytes Size
				AVIFileStreamRead(data_Chunk , 1 , 4 ,f);
				pCur   = data_Chunk;			

				frame_DataSize	= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3));
				pCur	+= 4;

				data_stream = malloc(frame_DataSize);
				AVIFileStreamRead(data_stream , 1 , frame_DataSize ,f);

				//printf("pcur(input_buffer) =0x%x,frame_DataSize=%d,data_stream[0]=0x%x,data_stream[1]=0x%x\n",data_stream,frame_DataSize, data_stream[0] ,data_stream[1]);
				JpegInputPkt *jpegPkt = (JpegInputPkt*) malloc(sizeof(JpegInputPkt));
		        if (jpegPkt)
		        {
		            jpegPkt->pInputBuffer = data_stream;
		            jpegPkt->bufferSize = frame_DataSize;
		            _packetQueuePut(&gMJpegInputQueue, jpegPkt);
					int_count++;
		        }

				AVIFileStreamFree(data_Chunk);
				data_Chunk = AVIFileStreamMalloc(4); //Read 4Bytes Size
				AVIFileStreamRead(data_Chunk , 1 , 4 ,f);
				pCur   = data_Chunk;			
			break;
			case JUNK:
				pCur	+= 4;
				Marker_Size			= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3));
				pCur	+= 4;
				
				fseek(f ,Marker_Size, SEEK_CUR);

				AVIFileStreamFree(data_Chunk);
				data_Chunk = AVIFileStreamMalloc(8);  //LIST fourcc name and 4 bytes Size
				AVIFileStreamRead(data_Chunk , 1 , 8 ,f);
				pCur   = data_Chunk;	
				break;
			case AUDS01WB:
				pCur	+= 4;

				AVIFileStreamFree(data_Chunk);
				data_Chunk = AVIFileStreamMalloc(4); //Read 4 Bytes Size
				AVIFileStreamRead(data_Chunk , 1 , 4 ,f);
				pCur   = data_Chunk;			
			
				frame_DataSize	= fourChar(*pCur,*(pCur+1) ,*(pCur+2),*(pCur+3));
				fseek(f , frame_DataSize, SEEK_CUR);

				AVIFileStreamFree(data_Chunk);
				data_Chunk = AVIFileStreamMalloc(4); //Read 4 Bytes Size
				AVIFileStreamRead(data_Chunk , 1 , 4 ,f);
				pCur   = data_Chunk;			
				
				break;
			case IDX1:
				AVIFileStreamFree(data_Chunk);
				pCur = NULL;
			break;
			default:
				AVIFileStreamFree(data_Chunk);
				fseek(f , -4, SEEK_CUR);
				data_Chunk = AVIFileStreamMalloc(5); //sometimes have  0x00 marker.
				AVIFileStreamRead(data_Chunk , 1 , 5 ,f);
				pCur   = data_Chunk;			
				pCur++;
            break;
		}
	}

	while( (int_count != dec_count) && --timeout_ms)
	{
		usleep(1000*10);
	}

	end:
	if(f)
	{
		fclose(f);
		videoPlayerIsFileEOF =true;
	}
	int_count =0 ; dec_count=0;
	usleep(3*1000);
}
pthread_exit(NULL);

}

static void FillBgColor(ITUSurface* lcdSurf, int bgColor)
{
    ITUColor fillColor;
	ITURotation rot = itv_get_rotation();
    
    fillColor.alpha = 0;
    fillColor.red = (uint8_t)((bgColor&0xff0000) >> 16);
    fillColor.green = (uint8_t)((bgColor&0x00ff00) >> 8);
    fillColor.blue = (uint8_t)(bgColor&0x0000ff);

#ifndef CFG_ENABLE_ROTATE
    ituColorFill(lcdSurf, 0, 0, ithLcdGetWidth(), ithLcdGetHeight(), &fillColor);
#else
    if (rot == ITU_ROT_90 || rot == ITU_ROT_270)
        ituColorFill(lcdSurf, 0, 0, ithLcdGetHeight(), ithLcdGetWidth(), &fillColor);
    else
    	ituColorFill(lcdSurf, 0, 0, ithLcdGetWidth(), ithLcdGetHeight(), &fillColor);
#endif
}

static void*
_DeMjpegThread(
    void *arg)
{
    int ret;
	VideoWindow *vwindow = (VideoWindow *)arg;
	struct  timeval    Tstart , Tend;
	unsigned  long     TimeDiff;

	while(!videoPlayerIsFileEOF)
    {
        JpegInputPkt* ptInputPkt = 0;

        if (gbAbortFlag)
        {
            break;
        }
		
        if (_packetQueueGet(&gMJpegInputQueue, (void**) &ptInputPkt, 0) > 0)
        {
	        gettimeofday(&Tstart,NULL);
		   	if (_mjpeg_decode_frame(ptInputPkt->pInputBuffer, ptInputPkt->bufferSize) >= 0)
		   	{
#ifdef CFG_BOOT_VIDEO_ENABLE_WINDOW_MODE		   	
	   			FillBgColor(vwindow->lcdSurf, vwindow->lcdBgColor);
#endif
                ituDrawVideoSurface(vwindow->lcdSurf, vwindow->x_pos, vwindow->y_pos, vwindow->width, vwindow->height);
        		ituFlip(vwindow->lcdSurf);	
				
				gettimeofday(&Tend,NULL);
				TimeDiff = 1000000 * (Tend.tv_sec-Tstart.tv_sec)+ Tend.tv_usec-Tstart.tv_usec;
				usleep(MainAVIH.dwMicroSecPerFrame - TimeDiff);
				//printf("the difference is %ld ,delay time = %ld\n",TimeDiff ,(MainAVIH.dwMicroSecPerFrame - TimeDiff) );  
		   	}
		    else printf("decode fail!\n");

            _jpegInputPktRelease(ptInputPkt);
        }
        else
        {
            usleep(1000);
        }
    }
    pthread_exit(NULL);
}

void PlayMjpeg(int x, int y, int width, int height, int bgColor, int volume)
{
    vwindow = (VideoWindow *)malloc(sizeof(VideoWindow));
 
    vwindow->lcdSurf = ituGetDisplaySurface();
    vwindow->x_pos = x;
    vwindow->y_pos = y;
    vwindow->width = width;
    vwindow->height = height;
    vwindow->lcdBgColor = bgColor;

	gx = x;
	gy = y;
	gwidth = width;
	gheight = height;

    itv_set_video_window(x, y, width, height);
    bPlayingBootVideo = true;
    isOtherVideoPlaying = true;

	printf("PlayMjpeg\n");
	_mjpeg_decode_init();

#if defined(CFG_BUILD_ITV) && !defined(CFG_TEST_VIDEO) 
	itv_set_pb_mode(1);
#endif
	printf("x=%d,y=%d,width=%d,height=%d\n",x,y,width,height);

	_packetQueueInit(&gMJpegInputQueue, _jpegInputPktRelease, InputQueueNumber);
	pthread_create(&tidAVIOpen, NULL, AVIFileOpen, (void *)CFG_PRIVATE_DRIVE ":/media/" BOOT_VIDEO);
	usleep(1000*100);

	//create jpeg decode thread for jpeg decode.
	pthread_create(&tidDeMjpeg	, NULL, _DeMjpegThread,  (void *)vwindow);

}

void WaitPlayMjpegFinish(void)
{
    if(tidDeMjpeg)
	{
		pthread_join(tidDeMjpeg, NULL);

#if (CFG_BUILD_ITV) && (CFG_CHIP_FAMILY == 9850)  
        itv_stop_vidSurf_anchor();
#endif            	
    /* mtal routine */
#ifdef CFG_BUILD_ITV            
		itv_flush_dbuf();
#ifndef CFG_TEST_VIDEO
		itv_set_pb_mode(0);
#endif
#endif 
		_mjpeg_decode_end();

	    _packetQueueEnd(&gMJpegInputQueue);

		pthread_join(tidAVIOpen, NULL);

		if(vwindow)
			free(vwindow);
        videoPlayerIsFileEOF = false;
        bPlayingBootVideo = false;
        isOtherVideoPlaying = false;
		tid 	   = 0;
		tidDeMjpeg = 0;
		tidAVIOpen = 0;
	}    
}

