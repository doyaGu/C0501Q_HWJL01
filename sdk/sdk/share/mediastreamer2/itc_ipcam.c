#include <pthread.h>
#include <semaphore.h>
#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/mscommon.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "itc_ipcam.h"

bool_t isIPCAM_FirstVideoGetted = FALSE;

typedef struct PacketQueue {
    AVPacketList    *first_pkt, *last_pkt;
    int             nb_packets;
    int             size;
    int             abort_request;
    int64_t         lastPts;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} PacketQueue;

typedef struct IPCamInstance {
    PacketQueue     videoq;
    ms_mutex_t      ipcam_mutex;
    sem_t           ipcam_sem;
} IPCamInstance;


IPCamInstance *global_ipcam_Inst = NULL;
AVPacket flush_pkt;


///////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
//
///////////////////////////////////////////////////////////////////////////////////////////
static int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
    AVPacketList *pkt1;

    /* duplicate the packet */
    if (q->mutex)
    {
#ifdef ENABLE_GENERAL_PLAYER    
        if (pkt != &flush_pkt && av_dup_packet(pkt) < 0)
            return -1;

        pkt1       = av_malloc(sizeof(AVPacketList));
#endif		
        if (!pkt1)
            return -1;
        pkt1->pkt  = *pkt;
        pkt1->next = NULL;

        pthread_mutex_lock(&q->mutex);

        if (!q->last_pkt)
            q->first_pkt = pkt1;
        else
            q->last_pkt->next = pkt1;
        q->last_pkt = pkt1;
        q->nb_packets++;
        q->size    += pkt1->pkt.size + sizeof(*pkt1);
        q->lastPts  = pkt->pts;
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
static void packet_queue_init(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    //packet_queue_put(q, &flush_pkt);
}

static void packet_queue_flush(PacketQueue *q)
{
    AVPacketList *pkt, *pkt1;
    if (q->mutex)
    {
        pthread_mutex_lock(&q->mutex);
        for (pkt = q->first_pkt; pkt != NULL; pkt = pkt1)
        {
            pkt1 = pkt->next;
#ifdef ENABLE_GENERAL_PLAYER			
            av_free_packet(&pkt->pkt);
            av_freep(&pkt);
#endif			
        }
        q->last_pkt   = NULL;
        q->first_pkt  = NULL;
        q->nb_packets = 0;
        q->size       = 0;
        pthread_mutex_unlock(&q->mutex);
    }
}

static void packet_queue_end(PacketQueue *q)
{
    if (q->mutex)
    {
        packet_queue_flush(q);
        pthread_mutex_destroy(&q->mutex);
        pthread_cond_destroy(&q->cond);
        memset(q, 0, sizeof(PacketQueue));
    }
}

static void packet_queue_abort(PacketQueue *q)
{
    if (q->mutex)
    {
        pthread_mutex_lock(&q->mutex);
        q->abort_request = 1;
        pthread_cond_signal(&q->cond);
        pthread_mutex_unlock(&q->mutex);
    }
}

/* return < 0 if aborted, 0 if no packet and > 0 if packet received. */
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
    AVPacketList *pkt1;
    int          ret;
    if (q->mutex)
    {
        pthread_mutex_lock(&q->mutex);

        for (;;)
        {
            if (q->abort_request)
            {
                ret = -1;
                break;
            }

            pkt1 = q->first_pkt;
            if (pkt1)
            {
                q->first_pkt = pkt1->next;
                if (!q->first_pkt)
                    q->last_pkt = NULL;
                q->nb_packets--;
                q->size -= pkt1->pkt.size + sizeof(*pkt1);
                *pkt     = pkt1->pkt;
#ifdef ENABLE_GENERAL_PLAYER				
                av_free(pkt1);
#endif
                ret      = 1;
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

int PutIntoPacketQueue(unsigned char* inputbuf, int inputbuf_size, double timestamp)
{
    IPCamInstance *pInst = global_ipcam_Inst;
    AVPacket packet;

    if(!pInst) return -1;
    ms_mutex_lock(&pInst->ipcam_mutex);
#ifdef ENABLE_GENERAL_PLAYER	
    av_init_packet(&packet);
#endif
    packet.data = inputbuf ;
    packet.size = inputbuf_size;
    packet.timestamp = timestamp;
#ifdef ENABLE_GENERAL_PLAYER	
    packet.destruct = av_destruct_packet;
#endif
    packet_queue_put(&pInst->videoq, &packet);
    ms_mutex_unlock(&pInst->ipcam_mutex);
    
    return 0;
}

static void itc_ipcam_init(MSFilter *f){
    IPCamInstance *pInst = (IPCamInstance *)ms_new(IPCamInstance, 1);
    printf("[MSItcIPCAM] init\n");
    do
    {
        if (!f || !pInst)
        {
            break;
        }
        memset(pInst, 0x0, sizeof(IPCamInstance));
        packet_queue_init(&pInst->videoq);
        ms_mutex_init(&pInst->ipcam_mutex, NULL);
        sem_init(&pInst->ipcam_sem, 0, 0);
        global_ipcam_Inst = pInst;
        isIPCAM_FirstVideoGetted = FALSE;
    } while (0);

    return;
}

static void itc_ipcam_process(MSFilter *f){
    IPCamInstance *pInst = global_ipcam_Inst;
    int ret;
    mblk_t *m = NULL;
    if (!f || !pInst) return;
    
    for (;;)
    {
        AVPacket pkt;
        ret = packet_queue_get(&pInst->videoq, &pkt, 0);
        if (ret < 0)
        {
            printf("Packet_queue_get error\n");
        }
        else if (ret > 0)
        {
            //printf("YC: %s, %d, pkt.size = %d\n", __FUNCTION__, __LINE__, pkt.size);
            isIPCAM_FirstVideoGetted = TRUE;
            m = allocb(pkt.size + 10, 0);
            if (m != NULL)
            {
                memcpy(m->b_wptr, pkt.data, pkt.size);
                m->b_wptr += pkt.size;
                m->b_rptr += 4;
                mblk_set_timestamp_info(m, (uint32_t)(pkt.timestamp*90000));
                ms_queue_put(f->outputs[0], m);
            }    
#ifdef ENABLE_GENERAL_PLAYER			
            av_free_packet(&pkt);
#endif
        }
        else
        {
            break;
        }    
    }

}

static void itc_ipcam_postprocess(MSFilter *f){
    IPCamInstance *pInst = global_ipcam_Inst;
    if (!f || !pInst) return;

    packet_queue_end(&pInst->videoq);
    
}

static void itc_ipcam_uninit(MSFilter *f){
    IPCamInstance *pInst = global_ipcam_Inst;
    if (!f || !pInst) return;
    ms_mutex_destroy(&pInst->ipcam_mutex);
    sem_destroy(&pInst->ipcam_sem);
    ms_free(pInst);    
}


#ifdef _MSC_VER
MSFilterDesc ms_itc_ipcam_desc={
    MS_ITC_IPCAM_ID,
    "MSItcIPCAM",
    N_("Inter ticker communication filter."),
    MS_FILTER_OTHER,
    NULL,
    0,
    1,
    itc_ipcam_init,
    NULL,
    itc_ipcam_process,
    itc_ipcam_postprocess,
    itc_ipcam_uninit,
    NULL
};
#else
MSFilterDesc ms_itc_ipcam_desc={
    .id=MS_ITC_IPCAM_ID,
    .name="MSItcIPCAM",
    .text=N_("Inter ticker communication filter."),
    .category=MS_FILTER_OTHER,
    .ninputs=0,
    .noutputs=1,
    .init=itc_ipcam_init,
    .process=itc_ipcam_process,
    .postprocess=itc_ipcam_postprocess,
    .uninit=itc_ipcam_uninit
};
#endif

MS_FILTER_DESC_EXPORT(ms_itc_ipcam_desc)

