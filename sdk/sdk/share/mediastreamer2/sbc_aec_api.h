#include "mediastreamer2/msqueue.h"


/***
SbcAECState

AEC struct
***/
typedef struct SbcAECState{
    MSBufferizer delayed_ref;
    MSBufferizer ref;
    MSBufferizer echo;
    MSBufferizer hw_ref;
    MSBufferizer hw_echo;
    MSBufferizer hw_clean;
    
        bool_t hw_start;
        ms_thread_t hw_thread;	
        //ms_mutex_t hw_mutex;
    
    int framesize;
    int samplerate;
    int delay_ms;
    int AECframesize;
    int delay_ref_samples;
    bool_t echostarted;
    bool_t echoruning;
    bool_t delayset;
    bool_t bypass_mode;
    bool_t AECrun;

    queue_t echo_copy_q;
    queue_t ref_copy_q;
    queue_t clean_copy_q;

    ms_mutex_t mutex;
}SbcAECState;

/***
AEC_ITE_INIT
ex:
SAMPLINGRATE : 8000
FRAMESIZE    : 320
DELAY_MS     : 120
***/
SbcAECState* AEC_ITE_INIT(int SAMPLINGRATE,int FRAMESIZE,int DELAY_MS,bool_t BYPASS);

/***
AEC_applyTX  (TX buffer control)

TxIn_echo   : MIC in data with echo
TxOut_clean : after aec process clean data 
***/
int AEC_applyTX(SbcAECState *s,mblk_t *TxIn_echo,mblk_t *TxOut_clean,int frame_size);

/***
AEC_applyRX  (RX buffer control)

RxIn_ref    : Far-end data
RxOut_ref   : output SPK data
***/
int AEC_applyRX(SbcAECState *s,mblk_t *RxIn_ref,mblk_t *RxOut_ref,int frame_size);


void AEC_set_delay(SbcAECState *s,int delay,bool_t setdelay_flag);
void AEC_set_delay_flag(SbcAECState *s,bool_t yesno);
/***
AEC_ITE_UNINIT
***/
void AEC_ITE_UNINIT(SbcAECState *s);

/***
sbc_aec_start_hwthread

aec thread start 
***/
static void sbc_aec_start_hwthread(SbcAECState *e);

/***
sbc_aec_stop_hwthread

aec thread stop
***/
static void sbc_aec_stop_hwthread(SbcAECState *e);

/***
hw_engine_thread

main aec thread
***/
static void *sbc_aec_hw_engine_thread(void *arg);

/***
AEC_IN_ARM_INIT

ifndef RUN_IN_RISC (in sbc_aec_api.c)
aec can run in arm with another Algorithm 
it can be better aec effect but need more CPU loading
***/
static void AEC_IN_ARM_INIT(int *AECframesize);