#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#include "video_encoder/video_encoder.h"
#include "tsi/mmp_tsi.h"
#include "xcpu_master/itx.h"
//=============================================================================
//                Constant Definition
//=============================================================================

#define es_ring_size         1024000
#ifdef CFG_DUAL_STREAM
    #define VIDEO_STREAM_NUM 2
    #define VIDEO_BUF_NUM    3
#else
    #define VIDEO_STREAM_NUM 1
    #define VIDEO_BUF_NUM    1
#endif

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

static bool            gbVideoInit      = false;
static bool            gbVideoEnable    = false;
static sem_t           gpEncoderSem;
static pthread_mutex_t VideoEngineMutex = PTHREAD_MUTEX_INITIALIZER;
//static FILE      *fp = 0;
static uint8_t         *es_ring_buf     = NULL;
static uint32_t        available_size   = 0;
static uint32_t        turnaround_size  = 0;
static uint8_t         *rd_ptr          = NULL;
static uint8_t         *turnaround_ptr  = NULL;
static bool            bturnaround      = false;
static uint8_t         *gpJPEGAddr      = NULL;
static uint32_t        gJPEGSize;
static uint32_t        gVideoWriteIdx[VIDEO_STREAM_NUM];
static uint32_t        gVideoReadIdx[VIDEO_STREAM_NUM];
static uint8_t         gVideoUserNum[VIDEO_STREAM_NUM];
static VIDEO_SAMPLE    *gptVideoSample[VIDEO_STREAM_NUM][VIDEO_BUF_NUM];
static bool            gStartgetVideo[VIDEO_STREAM_NUM];
static uint32_t        gTsiPort = 0;
//=============================================================================
//                Public Function Definition
//=============================================================================

void
VideoEncoder_Init(void) //(VIDEO_ENCODE_PARAMETER* enPar)
{
    uint8_t      i, k;
    VIDEO_SAMPLE *prev = NULL, *curr = NULL;

    for (k = 0; k < VIDEO_STREAM_NUM; k++)
    {
        gVideoWriteIdx[k] = 0;
        gVideoReadIdx[k]  = 0;
        gStartgetVideo[k] = false;
		gVideoUserNum[k] = 0;
        for (i = 0; i < VIDEO_BUF_NUM; i++)
        {
            curr = gptVideoSample[k][i];
            while (curr != NULL)
            {
                prev = curr;
                curr = curr->next;
                free(prev);
                prev = NULL;
            }
            gptVideoSample[k][i] = NULL;
        }
    }

    if (ithGetDeviceId() == 0x9070)    gTsiPort = 1;

    mmpTsiInitialize(gTsiPort);
    
    gbVideoEnable = false;
    gbVideoInit   = true;
    sem_init(&gpEncoderSem, 0, 0);
}

void
VideoEncoder_Open(uint8_t streamId)
{
    if (!gbVideoInit)
        return;
	
    pthread_mutex_lock(&VideoEngineMutex);

    if (!gbVideoEnable)
    {
        mmpTsiEnable(gTsiPort);

        es_ring_buf     = malloc(es_ring_size);

        available_size  = 0;
        turnaround_size = 0;
        bturnaround     = false;
        rd_ptr          = NULL;
    }

    mmpSpiSetControlMode(SPI_CONTROL_SLAVE);	
    //itxCamPWon();
    if (!gbVideoEnable)
        itxEncStart();
    itxEncSetStream(streamId);
    mmpSpiResetControl();	
    gbVideoEnable = true;
    pthread_mutex_unlock(&VideoEngineMutex);
}

void
VideoEncoder_Close(void)
{
    uint8_t      i, k;
    VIDEO_SAMPLE *prev = NULL, *curr = NULL;

    if (!gbVideoInit)
        return;

    pthread_mutex_lock(&VideoEngineMutex);

    gbVideoEnable = false;	
    mmpTsiDisable(gTsiPort);      
    mmpSpiSetControlMode(SPI_CONTROL_SLAVE);
    itxEncStop();
    //itxCamStandBy();   
    mmpSpiResetControl();
	
    if (es_ring_buf != NULL)
    {
        free(es_ring_buf);
        es_ring_buf = NULL;
    }

    for (k = 0; k < VIDEO_STREAM_NUM; k++)
    {
        gVideoWriteIdx[k] = 0;
        gVideoReadIdx[k]  = 0;
        gStartgetVideo[k] = false;
        for (i = 0; i < VIDEO_BUF_NUM; i++)
        {
            curr = gptVideoSample[k][i];
            while (curr != NULL)
            {
                prev = curr;
                curr = curr->next;
                free(prev);
                prev = NULL;
            }
            gptVideoSample[k][i] = NULL;
        }
    }
	mmpTsiReset(gTsiPort);
    pthread_mutex_unlock(&VideoEngineMutex);
}

void
JPEGEncodeFrame(JPEG_ENCODE_PARAMETER *enPara)
{
    uint32_t valid_size       = 0;
    uint8_t  *pTsi_buf        = 0;
    uint32_t j_available_size = 0;
    uint8_t  *j_rd_ptr        = NULL;
    uint32_t nal_size         = 0;
    uint32_t stuff_nalsize    = 0;
	uint32_t timeOut		  = 0;

    if (!gbVideoInit)
        return;
	
    if (gbVideoEnable)
    {
        pthread_mutex_lock(&VideoEngineMutex);
        mmpSpiSetControlMode(SPI_CONTROL_SLAVE);
        itxJPGRecord(enPara->quality);
        mmpSpiResetControl();
        pthread_mutex_unlock(&VideoEngineMutex);

        if (itpSemWaitTimeout(&gpEncoderSem, 2000) == 0)
        {
            //enPara->strmBuf = malloc(gJPEGSize);
            enPara->enSize = gJPEGSize;
            memcpy(enPara->strmBuf, gpJPEGAddr, gJPEGSize);
        }else
        {
        	printf("itpSemWaitTimeout!!!\n");
        }
    }
    else
    {
        pthread_mutex_lock(&VideoEngineMutex);
		mmpTsiEnable(gTsiPort);
        mmpSpiSetControlMode(SPI_CONTROL_SLAVE);
        itxJPGRecord(enPara->quality);
        mmpSpiResetControl();

        do
        {
            mmpTsiReceive(gTsiPort, &pTsi_buf, &valid_size);

            if (j_available_size == 0)
            {
                j_rd_ptr = pTsi_buf;
            }			
            j_available_size += valid_size;

			timeOut++;
			if(timeOut > 2000)
			{
				 enPara->enSize = 0;
				 printf("Jpeg Encoder Timeout!\n");
				 break;
			}

            if (j_available_size >= 16)
            {
                if ((*j_rd_ptr == 0x00) &&
                    (*(j_rd_ptr + 1) == 0x00) &&
                    (*(j_rd_ptr + 2) == 0x01) &&
                    (*(j_rd_ptr + 3) == 0xfe))
                {
                    nal_size      = (uint32_t)((*(j_rd_ptr + 4) << 24) + (*(j_rd_ptr + 5) << 16) + (*(j_rd_ptr + 6) << 8) + (*(j_rd_ptr + 7)));
                    stuff_nalsize = (uint32_t)((*(j_rd_ptr + 8) << 24) + (*(j_rd_ptr + 9) << 16) + (*(j_rd_ptr + 10) << 8) + (*(j_rd_ptr + 11)));

                    if ((j_available_size - 16) >= stuff_nalsize)
                    {
                        mmpSpiSetControlMode(SPI_CONTROL_SLAVE);
                        itxJPGStop();
                        mmpSpiResetControl();

                        enPara->enSize = nal_size;
						j_rd_ptr+=16;
                        memcpy(enPara->strmBuf, j_rd_ptr, nal_size);
                        break;
                    }
                }
                else
                {
                    j_rd_ptr++;
                    j_available_size--;
                    printf(" Search Sync code %x %d\n", j_rd_ptr, j_available_size);
                }
            }
            usleep(1000);
        } while (1);

        mmpTsiDisable(gTsiPort);
		mmpTsiReset(gTsiPort);
        pthread_mutex_unlock(&VideoEngineMutex);
    }
}

void
VideoEncoder_GetSample(VIDEO_SAMPLE **pVidSample, uint8_t get_id)
{
    uint32_t     valid_size = 0;
    uint8_t      *pTsi_buf  = NULL;
    uint8_t      *data_buf  = NULL;
    bool         bjpeg      = false;
    uint8_t      streamId   = 0, i = 0;
    VIDEO_SAMPLE *current   = NULL;
    VIDEO_SAMPLE *head0     = NULL, *head1 = NULL;
    VIDEO_SAMPLE *prev0     = NULL, *prev1 = NULL;

    if (!gbVideoInit || !gbVideoEnable)
        return;

    pthread_mutex_lock(&VideoEngineMutex);

    mmpTsiReceive(gTsiPort, &pTsi_buf, &valid_size);

    if (valid_size)
    {
        uint32_t nal_size      = 0;
        uint32_t stuff_nalsize = 0;
        uint32_t ms_timestamp  = 0;

        if (available_size == 0)
        {
            rd_ptr = pTsi_buf;
        }

        if (pTsi_buf < rd_ptr)
        {
            if (bturnaround == false)
            {
                turnaround_ptr = pTsi_buf;
                bturnaround    = true;
            }
            turnaround_size += valid_size;

            //printf("Turn around %x %d %d\n", turnaround_ptr, turnaround_size, available_size);
        }
        else
        {
            available_size += valid_size;
        }
		
        //while ((available_size + turnaround_size) >= 16)
        while ((available_size) >= 16)
        {
            //TODO : TSO & TSI shift case
            if ((*rd_ptr == 0x00)
                && (*(rd_ptr + 1) == 0x00)
                && (*(rd_ptr + 2) == 0x01)
                && ((*(rd_ptr + 3) & 0xfc) == 0xfc))
            {
                nal_size      = (uint32_t)((*(rd_ptr + 4) << 24) + (*(rd_ptr + 5) << 16) + (*(rd_ptr + 6) << 8) + (*(rd_ptr + 7)));
                stuff_nalsize = (uint32_t)((*(rd_ptr + 8) << 24) + (*(rd_ptr + 9) << 16) + (*(rd_ptr + 10) << 8) + (*(rd_ptr + 11)));
                ms_timestamp  = (uint32_t)((*(rd_ptr + 12) << 24) + (*(rd_ptr + 13) << 16) + (*(rd_ptr + 14) << 8) + (*(rd_ptr + 15)));

                if (*(rd_ptr + 3) == 0xfe)
                    bjpeg = true;
                else
                    bjpeg = false;

                if (*(rd_ptr + 3) == 0xff)
                    streamId = 0;
                else
                    streamId = 1;

                if (bjpeg)
                    stuff_nalsize += 256;

                if (stuff_nalsize <= (available_size + turnaround_size - 16))
                {
                    //save every completed NAL with matrix
                    available_size -= 16;
                    rd_ptr         += 16;
                    if (stuff_nalsize <= available_size)
                    {
                        data_buf        = rd_ptr;
                        available_size -= stuff_nalsize;

                        if (available_size == 0 && turnaround_size != 0)
                        {
                            rd_ptr          = turnaround_ptr;
                            available_size  = turnaround_size;
                            bturnaround     = false;
                            turnaround_size = 0;
                        }
                        else
                            rd_ptr += stuff_nalsize;
                    }
                    else
                    {
                        memcpy(es_ring_buf, rd_ptr, available_size);
                        memcpy(&es_ring_buf[available_size], turnaround_ptr, stuff_nalsize - available_size);
                        //printf("Ring0 %d %d %d\n", available_size, turnaround_size, stuff_nalsize);
                        data_buf        = es_ring_buf;

                        rd_ptr          = turnaround_ptr + (stuff_nalsize - available_size);
                        available_size  = turnaround_size - (stuff_nalsize - available_size);
                        bturnaround     = false;
                        turnaround_size = 0;
                        //printf("Ring1 %d %d %d\n", available_size, turnaround_size, stuff_nalsize);
                    }

                    if (bjpeg) // JPEG
                    {
                        gJPEGSize  = nal_size;
                        gpJPEGAddr = data_buf;
                        //itpSemPostFromISR(&gpEncoderSem);
                        sem_post(&gpEncoderSem);
                    }
                    else
                    {
                        current = (VIDEO_SAMPLE *)malloc(sizeof(VIDEO_SAMPLE));
						//memset(current, 0, sizeof(VIDEO_SAMPLE));
                        if (current == NULL)
                            printf("Error allocate list\n");

                        current->addr      = data_buf;
                        current->next      = NULL;
                        current->size      = nal_size;
                        current->timestamp = ms_timestamp;
                        current->streamId  = streamId;

                        if (current->streamId == 0)
                        {
                            if (gStartgetVideo[current->streamId] == true)
                            {
                                if (gptVideoSample[current->streamId][gVideoWriteIdx[current->streamId]] == NULL)
                                {
                                    gptVideoSample[current->streamId][gVideoWriteIdx[current->streamId]] = current;
                                    gVideoWriteIdx[current->streamId]                                    = (gVideoWriteIdx[current->streamId] + 1) % VIDEO_BUF_NUM;
                                }
                                else
                                    prev0->next = current;

                                prev0 = current;
                            }
                            else
                                free(current);
                        }
                        else if (current->streamId == 1)
                        {
                            if (gStartgetVideo[current->streamId] == true)
                            {
                                if (gptVideoSample[current->streamId][gVideoWriteIdx[current->streamId]] == NULL)
                                {
                                    gptVideoSample[current->streamId][gVideoWriteIdx[current->streamId]] = current;
                                    gVideoWriteIdx[current->streamId]                                    = (gVideoWriteIdx[current->streamId] + 1) % VIDEO_BUF_NUM;
                                }
                                else
                                    prev1->next = current;

                                prev1 = current;
                            }
                            else
                                free(current);
                        }
                    }
                }
                else
                {
                    break;
                }
            }
            else
            {
                rd_ptr++;
                available_size--;
                printf(" Search Sync code %x %d\n", rd_ptr, available_size);
            }
        }
    }
    pthread_mutex_unlock(&VideoEngineMutex);

    //printf("k=%d\n", get_id);
    if (gptVideoSample[get_id][gVideoReadIdx[get_id]] != NULL)
    {
        *(pVidSample)                                 = gptVideoSample[get_id][gVideoReadIdx[get_id]];
        gptVideoSample[get_id][gVideoReadIdx[get_id]] = NULL;
        gVideoReadIdx[get_id]                         = (gVideoReadIdx[get_id] + 1) % VIDEO_BUF_NUM;
    }
    return;
}

void
VideoEncoder_SetSreamstate(uint8_t stream_id, bool state)
{
    printf(">>>>> %d, %d\n", stream_id, state);
    gStartgetVideo[stream_id] = state;
}

bool
VideoEncoder_GetSreamstate(uint8_t stream_id)
{
    return gStartgetVideo[stream_id];  
}

void
VideoEncoder_SetSreamUserNum(uint8_t stream_id, bool state)
{
    //printf(">>>>>%s, %d, %d\n", __FUNCTION__, stream_id, state);
	if(state)
		gVideoUserNum[stream_id]++;
	else
		gVideoUserNum[stream_id]--;
}

uint8_t
VideoEncoder_GetSreamUserNum(uint8_t stream_id)
{
    //printf(">>>>>%s, %d, %d\n", __FUNCTION__, stream_id, gVideoUserNum[stream_id]);
	return gVideoUserNum[stream_id];
}