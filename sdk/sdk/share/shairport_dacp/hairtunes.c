/*
 * HairTunes - RAOP packet handler and slave-clocked replay engine
 * Copyright (c) James Laird 2011
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <openssl/aes.h>
#include <math.h>
#include <sys/stat.h>

#include "hairtunes.h"
#include "shairport.h"
#include <sys/signal.h>
#include <fcntl.h>
#include <ao/ao.h>

#ifdef FANCY_RESAMPLING
#include <samplerate.h>
#endif

#include <assert.h>
static int debug = 0;

#include "alac.h"

#ifdef __OPENRTOS__
#include "i2s/i2s.h"
#include "ite/itp.h"

#define I2S_BUFFER_LENGTH  16*64*1408 //64*1024
static char gOutBuffer[I2S_BUFFER_LENGTH]__attribute__ ((aligned(16)));
static unsigned int gOutReadPointer=0;
static unsigned int gOutWritePointer=0;
#endif

//#define TEMP_SOLUTION
#define DEMO_SOLUTION

//#define PCM_DUMP
#ifdef PCM_DUMP
char inFileName[256] = "A:\\dump_data.pcm";
int   gPCM = 0;
static FILE *fin  = NULL;

#endif

static int gDecode = 0;
static unsigned int gAlacStart,gAlacStop;
static int gDec=0;

#undef AF_INET6

// and how full it needs to be to begin (must be <BUFFER_FRAMES)
#define START_FILL    282

#define MAX_PACKET      2048

typedef unsigned short seq_t;

// global options (constant after init)
static unsigned char aeskey[16], aesiv[16];
static AES_KEY aes;
static char *rtphost = 0;
static int dataport = 0, controlport = 0, timingport = 0;
static int fmtp[32];
static int sampling_rate;
static int frame_size;
static unsigned int gRTPaddress;
static int buffer_start_fill;

static char *libao_driver = NULL;
static char *libao_devicename = NULL;
static char *libao_deviceid = NULL; // ao_options expects "char*"

// FIFO name and file handle
static char *pipename = NULL;
static int pipe_handle = -1;

#define FRAME_BYTES (4*frame_size)
// maximal resampling shift - conservative
#define OUTFRAME_BYTES (4*(frame_size+4))


static alac_file *decoder_info;

extern SHAIRPORT_PARAM gShairport;

#ifdef FANCY_RESAMPLING
static int fancy_resampling = 1;
static SRC_STATE *src;
#endif

static int  init_rtp(void);
static void init_buffer(void);
static int  init_output(void);
static void rtp_request_resend(seq_t first, seq_t last);
static void ab_resync(void);

// interthread variables
// stdin->decoder
static double volume = 1.0;
static int fix_volume = 0x10000;
static pthread_mutex_t vol_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct audio_buffer_entry {   // decoded audio packets
    int ready;
    signed short *data;
} abuf_t;
static abuf_t audio_buffer[BUFFER_FRAMES];
#define BUFIDX(seqno) ((seq_t)(seqno) % BUFFER_FRAMES)

// mutex-protected variables
static seq_t ab_read, ab_write;
static int ab_buffering = 1, ab_synced = 0;
static pthread_mutex_t ab_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ab_buffer_ready = PTHREAD_COND_INITIALIZER;
static int ab_init =0;

static int gnSameLatePacketNumber = 0;
static seq_t gnSameLatePacketId = 0;

// check buffering get late packetid 
static int gnBufferingLatePacketNumber = 0;
static int gnReceiveFlush =0;
#if 0
static seq_t gnLatePacketId = 0;
static int gnLatePacketNumber = 0;
#endif

static int gnWakeUp = 0;
static int gnMissingFrame = 0;
static int gnMissingFramePrint = 0;
static struct timeval tWakeUp1 = {0, 0}, tWakeUp2 = {0, 0};
// avoid iOS device 100~200 ms mute problem
static int gnPause = 0;

#define HAIRTUNE_WAKE_MSEC   8*(60 * 1000)

static struct timeval tvO = {0, 0};
static struct timeval tv1 = {0, 0};



static void die(char *why) {
    fprintf(stderr, "FATAL: %s\n", why);
    exit(1);
}

#ifdef HAIRTUNES_STANDALONE
static int hex2bin(unsigned char *buf, char *hex) {
    int i, j;
    if (strlen(hex) != 0x20)
        return 1;
    for (i=0; i<0x10; i++) {
        if (!sscanf(hex, "%2X", &j))
           return 1;
        hex += 2;
        *buf++ = j;
    }
    return 0;
}
#endif

static int init_decoder(void) {
    alac_file *alac;
    int sample_size = fmtp[3];

    frame_size = fmtp[1]; // stereo samples
    sampling_rate = fmtp[11];

    if (sample_size != 16)
        die("only 16-bit samples supported!");

    alac = create_alac(sample_size, 2);
    if (!alac)
        return 1;
    decoder_info = alac;

    alac->setinfo_max_samples_per_frame = frame_size;
    alac->setinfo_7a =      fmtp[2];
    alac->setinfo_sample_size = sample_size;
    alac->setinfo_rice_historymult = fmtp[4];
    alac->setinfo_rice_initialhistory = fmtp[5];
    alac->setinfo_rice_kmodifier = fmtp[6];
    alac->setinfo_7f =      fmtp[7];
    alac->setinfo_80 =      fmtp[8];
    alac->setinfo_82 =      fmtp[9];
    alac->setinfo_86 =      fmtp[10];
    alac->setinfo_8a_rate = fmtp[11];
    allocate_buffers(alac);
    return 0;
}

// return 1, can not sleep
static int hairtunesAudioOutputWake()
{
    if (gnWakeUp){
        gettimeofday(&tWakeUp2, NULL);
        if (itpTimevalDiff(&tWakeUp1, &tWakeUp2) >= HAIRTUNE_WAKE_MSEC) {
            printf("[Hairtune] stop wake up %d #line %d \n",HAIRTUNE_WAKE_MSEC,__LINE__);
            hairtunesSetAudioOutputWakeUp(0);
        }
        if (ab_write-ab_read < 100){
            return 0;
        }
    }
    return gnWakeUp;
}

int hairtunesSetAudioOutputWakeUp(int nWakeUp)
{
    if (nWakeUp && gnWakeUp==0){
        printf("[Hairtune] start Hairtune wake timer #line %d \n",__LINE__);
        gettimeofday(&tWakeUp1, NULL);        
    }
    gnWakeUp = nWakeUp;    
    return 0;    
}

int hairtunes_init(char *pAeskey, char *pAesiv, char *fmtpstr, int pCtrlPort, int pTimingPort,
         int pDataPort, char *pRtpHost, char*pPipeName, char *pLibaoDriver, char *pLibaoDeviceName, char *pLibaoDeviceId,
         int bufStartFill,int nInit)
{
    int i = 0;
    char *arg;
    char line[128];
    int in_line = 0;
    int n;
    double f;
    fprintf(stderr, "hairtunes_init\n");

    if(pAeskey != NULL)    
        memcpy(aeskey, pAeskey, sizeof(aeskey));
    if(pAesiv != NULL)
        memcpy(aesiv, pAesiv, sizeof(aesiv));
    if(pRtpHost != NULL)
        rtphost = pRtpHost;
    if(pPipeName != NULL)
        pipename = pPipeName;
    if(pLibaoDriver != NULL)
        libao_driver = pLibaoDriver;
    if(pLibaoDeviceName != NULL)
        libao_devicename = pLibaoDeviceName;
    if(pLibaoDeviceId != NULL)
        libao_deviceid = pLibaoDeviceId;
    
    controlport = pCtrlPort;
    timingport = pTimingPort;
    dataport = pDataPort;
    gnSameLatePacketNumber = 0;
    gnBufferingLatePacketNumber = 0;
    if(bufStartFill < 0)
        bufStartFill = START_FILL;
//    buffer_start_fill = bufStartFill+50;

    buffer_start_fill = bufStartFill;
    
    if (nInit){
        AES_set_decrypt_key(aeskey, 128, &aes);
        
#if defined(__OPENRTOS__)
        fprintf(stderr, "hairtunes_init init_output_i2s\n");
        init_output_i2s();
#endif
        if (shairportGetException()){
            printf("[Hairtunes] init exception %d, set default \n",shairportGetException());
            shairportSetException(0);
        }
        //ab_resync();
        hairtunes_flush(0);
        
        //gOutReadPointer=0;
        //gOutWritePointer=0;
        //I2S_DA32_SET_WP(gOutWritePointer);
    }else{
        AES_set_decrypt_key(aeskey, 128, &aes);

        memset(fmtp, 0, sizeof(fmtp));
        while ( (arg = strsep(&fmtpstr, " \t")) )
            fmtp[i++] = atoi(arg);

        init_decoder();
        init_buffer();
        init_rtp();      // open a UDP listen port and start a listener; decode into ring buffer
        fflush(stdout);
        init_output();              // resample and output from ring buffer
#if defined(__OPENRTOS__)
        fprintf(stderr, "hairtunes_init init_output_i2s\n");
        init_output_i2s();
#endif        
    
    }
    
#if 0
    while (fgets(line + in_line, sizeof(line) - in_line, stdin)) {
        n = strlen(line);
        if (line[n-1] != '\n') {
            in_line = strlen(line) - 1;
            if (n == sizeof(line)-1)
                in_line = 0;
            continue;
        }
        if (sscanf(line, "vol: %lf\n", &f)) {
            assert(f<=0);
            if (debug)
                fprintf(stderr, "VOL: %lf\n", f);
            pthread_mutex_lock(&vol_mutex);
            volume = pow(10.0,0.05*f);
            fix_volume = 65536.0 * volume;
            pthread_mutex_unlock(&vol_mutex);
            continue;
        }
        if (!strcmp(line, "exit\n")) {
            exit(0);
        }
        if (!strcmp(line, "flush\n")) {
            pthread_mutex_lock(&ab_mutex);
            ab_resync();
            pthread_mutex_unlock(&ab_mutex);
            if (debug)
                fprintf(stderr, "FLUSH\n");
        }
    }
    fprintf(stderr, "bye!\n");
    fflush(stderr);
#endif // 0
    gettimeofday(&tv1, NULL);
    gettimeofday(&tvO, NULL);

    return EXIT_SUCCESS;
}

void hairtunes_flush(int nFlushI2S)
{
    pthread_mutex_lock(&ab_mutex);
    ab_resync();
    pthread_mutex_unlock(&ab_mutex);
    if (debug)
        fprintf(stderr, "FLUSH\n");
    else
        printf("hairtunes_flush %d\n",gnReceiveFlush);
    // 1 means flush continue , 0 means 1 flush
    if (gnReceiveFlush==1){
        gnReceiveFlush = 0;
    } else if (gnReceiveFlush==0){
        gnReceiveFlush = 1;
    } 


    if (nFlushI2S && gnReceiveFlush)
        init_output_i2s();
    gOutReadPointer=0;
    gOutWritePointer=0;
    gnReceiveFlush = 1;

    I2S_DA32_SET_WP(gOutWritePointer);
    
}

void hairtunes_vol(double f)
{
    assert(f<=0);
    if (debug)
        fprintf(stderr, "VOL: %lf\n", f);
    pthread_mutex_lock(&vol_mutex);
    volume = pow(10.0,0.05*f);
    fix_volume = 65536.0 * volume;
    pthread_mutex_unlock(&vol_mutex);
}

#ifdef HAIRTUNES_STANDALONE
int main(int argc, char **argv) {
    char *hexaeskey = 0, *hexaesiv = 0;
    char *fmtpstr = 0;
    char *arg;
    assert(RAND_MAX >= 0x10000);    // XXX move this to compile time
    while ( (arg = *++argv) ) {
        if (!strcasecmp(arg, "iv")) {
            hexaesiv = *++argv;
            argc--;
        } else
        if (!strcasecmp(arg, "key")) {
            hexaeskey = *++argv;
            argc--;
        } else
        if (!strcasecmp(arg, "fmtp")) {
            fmtpstr = *++argv;
        } else
        if (!strcasecmp(arg, "cport")) {
            controlport = atoi(*++argv);
        } else
        if (!strcasecmp(arg, "tport")) {
            timingport = atoi(*++argv);
        } else
        if (!strcasecmp(arg, "dport")) {
            dataport = atoi(*++argv);
        } else
        if (!strcasecmp(arg, "host")) {
            rtphost = *++argv;
        } else
        if (!strcasecmp(arg, "pipe")) {
            if (libao_driver || libao_devicename || libao_deviceid ) {
                die("Option 'pipe' may not be combined with 'ao_driver', 'ao_devicename' or 'ao_deviceid'");
            }

            pipename = *++argv;
        } else
        if (!strcasecmp(arg, "ao_driver")) {
            if (pipename) {
                die("Option 'ao_driver' may not be combined with 'pipe'");
            }

            libao_driver = *++argv;
        } else
        if (!strcasecmp(arg, "ao_devicename")) {
            if (pipename || libao_deviceid ) {
                die("Option 'ao_devicename' may not be combined with 'pipe' or 'ao_deviceid'");
            }

            libao_devicename = *++argv;
        } else
        if (!strcasecmp(arg, "ao_deviceid")) {
            if (pipename || libao_devicename) {
                die("Option 'ao_deviceid' may not be combined with 'pipe' or 'ao_devicename'");
            }

            libao_deviceid = *++argv;
        }
#ifdef FANCY_RESAMPLING
        else
        if (!strcasecmp(arg, "resamp")) {
            fancy_resampling = atoi(*++argv);
        }
#endif
    }

    if (!hexaeskey || !hexaesiv)
        die("Must supply AES key and IV!");

    if (hex2bin(aesiv, hexaesiv))
        die("can't understand IV");
    if (hex2bin(aeskey, hexaeskey))
        die("can't understand key");
    return hairtunes_init(NULL, NULL, fmtpstr, controlport, timingport, dataport,
                    NULL, NULL, NULL, NULL, NULL, START_FILL);
}
#endif

static void init_buffer(void) {
    int i;
    for (i=0; i<BUFFER_FRAMES; i++)
        audio_buffer[i].data = malloc(OUTFRAME_BYTES);
    ab_resync();
}

static void free_buffer(){
    int i;
    for (i=0; i<BUFFER_FRAMES; i++){
        if (audio_buffer[i].data)
            free(audio_buffer[i].data);
    }
}

static void ab_resync(void) {
    int i;
    for (i=0; i<BUFFER_FRAMES; i++){
        audio_buffer[i].ready = 0;
        memset(audio_buffer[i].data,0,OUTFRAME_BYTES);
    }
    ab_synced = 0;
    ab_buffering = 1;

}

// the sequence numbers will wrap pretty often.
// this returns true if the second arg is after the first
static inline int seq_order(seq_t a, seq_t b) {
    signed short d = b - a;
    return d > 0;
}

#ifdef __OPENRTOS__
static ITP_DPU_INFO gDpuInfo;

static const char* DpuMode[] =
{
	":dpu:aes",
	":dpu:des",
	":dpu:des3",
	":dpu:csa",
	":dpu:crc",
};

static const char* DpuCipherTable[] =
{
    "aes",
    "cbc",
    "cfb",
    "ofb",
    "ctr",
};


void HwDpuEncrypt(int len,unsigned char *in, unsigned char *out)
{
    int fd;
    unsigned char iv[16];
    ITP_DPU_INFO *info;
    int i;
    
    info = &gDpuInfo;
    info->dpuMode = ITP_DPU_AES_MODE;
    info->cipher= ITP_DPU_CIPHER_CBC;
    info->descrypt =1;
    info->vctLen =sizeof(iv)*8;
    info->vctBuf=aesiv;
    info->keyLen = 128;
    info->keyBuf = aeskey;
    info->srcBuf = in;
    info->dstBuf = out;
    info->dpuLen= (uint32_t)len;
    //printf("HwDpu.01\n");
#if 0
printf("vctLen %d  iv\n",info->vctLen);
    for (i=0;i<16;i++)
        printf("0x%x ",info->vctBuf[i]);
    printf("\n");
    printf("key\n");

    for (i=0;i<16;i++)
        printf("0x%x ",info->keyBuf[i]);
    printf("\n");
#endif    
    fd = open(DpuMode[info->dpuMode], 0);

    //printf("HwDpu.02 %d\n",fd);

    //if have not any IOCTL operation, then do default DPU encryption(i.e. descrypt=0, cipher=ecb, crc_master=true, )
    //if(dpuMode==ITP_DPU_CSA_MODE) ==> descrypt=1, cipher=no_use, crc_master=no_use)
    //if(dpuMode==ITP_DPU_CRC_MODE) ==> descrypt=no_use, cipher=no_use, crc_master=1)
    //if(dpuMode==AES/DES/DES3) ==> descrypt=0, cipher=ecb, crc_master=no_use)

    if(info->dpuMode==ITP_DPU_CRC_MODE)
    {
        ioctl(fd, ITP_IOCTL_SET_CRC_MASTER, &info->crc_master); //0:crc_slave 1:crc_master
        //printf("HwDpu.03\n");
    }
    else
    {
        ioctl(fd, ITP_IOCTL_SET_KEY_LENGTH, &info->keyLen); //key.len, key.ptr
        ioctl(fd, ITP_IOCTL_SET_KEY_BUFFER, info->keyBuf); //key.len, key.ptr

        //printf("HwDpu.04[%x,%x]\n",&info->descrypt,&info->cipher);

        if(info->dpuMode!=ITP_DPU_CSA_MODE)
        {
            ioctl(fd, ITP_IOCTL_SET_DESCRYPT, &info->descrypt); //0:encrypt 1:descrypt
            //printf("HwDpu.05\n");
            ioctl(fd, ITP_IOCTL_SET_CIPHER, &info->cipher); //0:ecb, 1:cbc, 2:cfb, 3:ofb, 4:ctr
            //printf("HwDpu.06\n");
            if(info->cipher!=ITP_DPU_CIPHER_ECB)	//ECB mode has no initial vector
            {
                ioctl(fd, ITP_IOCTL_SET_VECTOR_LENGTH, &info->vctLen);
                ioctl(fd, ITP_IOCTL_SET_VECTOR_BUFFER, info->vctBuf);
            }
        //printf("HwDpu.07\n");
        }
    }
    //printf("HwDpu.08\n");

    write(fd, info->srcBuf, info->dpuLen);
    //printf("HwDpu.09\n");
#if 0    
    for (i=0;i<info->dpuLen;i++){
        printf("%d ",info->srcBuf[i]);
        if (i%8==0 && i)
            printf("\n");
    }
#endif    
    read(fd, info->dstBuf, info->dpuLen);
    //printf("HwDpu.10\n");
#if 0    
    for (i=0;i<info->dpuLen;i++){
        printf("%d ",info->dstBuf[i]);
        if (i%8==0 && i)
            printf("\n");
    }
printf("\n");
#endif

    close(fd);
}
#endif

static void alac_decode(short *dest, char *buf, int len) {
    unsigned char packet[MAX_PACKET];
    unsigned char iv[16];
    int aeslen = len & ~0xf;
    int outsize;
    int i;
    assert(len<=MAX_PACKET);
#if 0
    gAlacStart = ithTimerGetTime(ITH_TIMER4);
#endif
    memcpy(iv, aesiv, sizeof(iv));
#if 0 //def __OPENRTOS__
    HwDpuEncrypt(aeslen,buf,packet);

#else
    #if 0
    printf("vctLen %d  iv\n",sizeof(iv));
    for (i=0;i<16;i++)
        printf("0x%x ",iv[i]);
    printf("\n");
    printf("openssl len %d \n",aeslen);    
    #endif


    AES_cbc_encrypt((unsigned char*)buf, packet, aeslen, &aes, iv, AES_DECRYPT);

    #if 0
    printf("openssl src data\n");    
    for (i=0;i<aeslen;i++){
        printf("0x%x ",buf[i]);
        if (i%8==0 && i)
            printf("\n");
    }

    printf("openssl dest data\n");
    for (i=0;i<aeslen;i++){
        printf("0x%x ",packet[i]);
        if (i%8==0 && i)
            printf("\n");
    }
    printf("\n");
    printf("HwDpuEncrypt len %d \n",aeslen);    
    HwDpuEncrypt(aeslen,buf,packet2);
    printf("HwDpuEncrypt src data\n");    
    for (i=0;i<aeslen;i++){
        printf("0x%x ",buf[i]);
        if (i%8==0 && i)
            printf("\n");
    }

    printf("HwDpuEncrypt dest data\n");
    for (i=0;i<aeslen;i++){
        printf("0x%x ",packet2[i]);
        if (i%8==0 && i)
            printf("\n");
    }
    printf("\n");    
    while(1);
    #endif
#endif

    memcpy(packet+aeslen, buf+aeslen, len-aeslen);

#if 0
    gAlacStop = ithTimerGetTime(ITH_TIMER4);   
    gDec += gAlacStop-gAlacStart;
    if (gDecode%1000 == 0){
        printf("[Hairtune] sw gDecode %d %d \n",gDecode,gDec);            
        gDec = 0;
    }
#endif
    decode_frame(decoder_info, packet, dest, &outsize);
    if (outsize == FRAME_BYTES){

    }else{
        fprintf(stderr, "outsize: %d 0x%x %d \n",outsize,dest,len);
        usleep(30*1000);
    //assert(outsize == FRAME_BYTES);
    }
    
}

static void buffer_put_packet(seq_t seqno, char *data, int len) {
    abuf_t *abuf = 0;
    short buf_fill;
    int nTemp;
    pthread_mutex_lock(&ab_mutex);
    if (!ab_synced) {
        ab_write = seqno;
        ab_read = seqno-1;
        ab_synced = 1;
    }
#if defined(__OPENRTOS__)   
    gOutReadPointer = I2S_DA32_GET_RP();
    if (gOutReadPointer <= gOutWritePointer) {
        nTemp = gOutWritePointer - gOutReadPointer;
    } else {
        nTemp = I2S_BUFFER_LENGTH - (gOutReadPointer - gOutWritePointer);        
    }
#endif
    if (seqno == ab_write+1) {                  // expected packet
        abuf = audio_buffer + BUFIDX(seqno);
        ab_write = seqno;
    } else if (seq_order(ab_write, seqno)) {    // newer than expected
        //fprintf(stderr, "\nrtp_request_resend %04X (%04X:%04X) output %d \n", seqno, ab_read, ab_write,nTemp);
        rtp_request_resend(ab_write+1, seqno-1);
        abuf = audio_buffer + BUFIDX(seqno);
        ab_write = seqno;
    } else if (seq_order(ab_read, seqno)) {     // late but not yet played
        abuf = audio_buffer + BUFIDX(seqno);
        //fprintf(stderr, "\nlate  %04X (%04X:%04X)\n", seqno, ab_read, ab_write);
    } else {    // too late.
     //   fprintf(stderr, "\nlate packet %04X (%04X:%04X) (%d) (%d)\n", seqno, ab_read, ab_write,gShairport.nCurrState,ab_buffering);
        if (gnSameLatePacketId == seqno){
            gnSameLatePacketNumber++;
            if (gnSameLatePacketNumber%100==0)
                printf("[Hairtune]get same late packets %d %04X \n",gnSameLatePacketNumber,seqno);
        } else {
            gnSameLatePacketId = seqno;
            gnSameLatePacketNumber = 0;
        }
        if (gnSameLatePacketNumber>=2000){
            gnSameLatePacketNumber = 0;
            shairportSetException(3);
        }
        if (ab_buffering) {
            if ( (seqno > ab_read) && (seqno - ab_read >50) ){
                gnBufferingLatePacketNumber++;
                printf("[Hairtune] buffering get late packet %d \n",gnBufferingLatePacketNumber);
            } else if  ( (ab_read > seqno) && (ab_read-seqno>50) ){
                gnBufferingLatePacketNumber++;
                printf("[Hairtune] buffering get late packet %d \n",gnBufferingLatePacketNumber);
            } 
            if (gnBufferingLatePacketNumber>15){
                ab_write = seqno;
                ab_read = seqno-1;
                printf("[Hairtune] buffering get late packet ab_read = seqno %04X %04X \n",ab_read,seqno);                
                gnBufferingLatePacketNumber = 0;
            }            
        }
    }
    buf_fill = ab_write - ab_read;
    pthread_mutex_unlock(&ab_mutex);

   // fprintf(stderr, "\n alac_decode %04X (%04X:%04X)\n", seqno, ab_read, ab_write);

    if (abuf && abuf->ready==0) {
        if (abuf->ready==1)
            fprintf(stderr, "\abuf->ready==1 %04X \n",seqno);    
        alac_decode(abuf->data, data, len);
        abuf->ready = 1;
    }
    pthread_mutex_lock(&ab_mutex);
#ifdef TEMP_SOLUTION    
    #if defined(__OPENRTOS__)
        fprintf(stderr, "[Hairtune] seqno %d\n",seqno);
        audio_output(abuf->data, FRAME_BYTES/4);
    #endif
    #ifdef PCM_DUMP
        if (fin)
            fwrite(abuf->data, 1, FRAME_BYTES, fin);

        if (gPCM == 1000){
            fclose(fin);
        }
        gPCM++;
        
    #endif
#endif
    if (ab_buffering && buf_fill >= buffer_start_fill) {
        ab_buffering = 0;
        if (gnReceiveFlush==1){
            gnReceiveFlush = 0;
            printf("gnReceiveFlush = 0 \n");
        }

        fprintf(stderr, "\n pthread_cond_signal ~~~~~~~~~~~~~~~~~%d    ~~~~~~~~~~~~~~~%d    ~~~~~~~~~\n",gDecode,buffer_start_fill);

        shairportSetException(4);
        pthread_cond_signal(&ab_buffer_ready);
    }else{
        //fprintf(stderr, "\n pthread_cond_signal ~~~~%d   ~~~~~~~~~~~~~~~~~~~%d  ~~~~~~~~%d   ~~~~~~~~~~\n",ab_buffering,buf_fill,gDecode);
    }
    pthread_mutex_unlock(&ab_mutex);
}

static int rtp_sockets[2];  // data, control
#ifdef AF_INET6
static struct sockaddr_in6 rtp_client;
#else
static struct sockaddr_in rtp_client;
#endif

static void *rtp_thread_func(void *arg) {
    socklen_t si_len = sizeof(rtp_client);
    char packet[MAX_PACKET];
    char *pktp;
    seq_t seqno;
    ssize_t plen;
    int sock = rtp_sockets[0], csock = rtp_sockets[1];
    int readsock;
    char type;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    FD_SET(csock, &fds);

    while (select(csock>sock ? csock+1 : sock+1, &fds, 0, 0, 0)!=-1) {
        if (FD_ISSET(sock, &fds)) {
            readsock = sock;
        } else {
            readsock = csock;
        }
        FD_SET(sock, &fds);
        FD_SET(csock, &fds);

        plen = recvfrom(readsock, packet, sizeof(packet), 0, (struct sockaddr*)&rtp_client, &si_len);

        //printf("readsock %d 0x%x 0x%x \n",readsock,getShairportsaddr(),rtp_client.sin_addr);
        if (plen < 12)
            continue;
        assert(plen<=MAX_PACKET);
        #if 1
        memcpy(&gRTPaddress,&rtp_client.sin_addr,4);
        if (getShairportsaddr() != gRTPaddress){
            //printf("readsock %d 0x%x 0x%x \n",readsock,getShairportsaddr(),rtp_client.sin_addr);            
            //gRTPaddress = rtp_client.sin_addr;
            continue;            
        }
        #endif
            
        type = packet[1] & ~0x80;
        if (type == 0x60 || type == 0x56) {   // audio data / resend
            pktp = packet;
            if (type==0x56) {
                pktp += 4;
                plen -= 4;
            }
            seqno = ntohs(*(unsigned short *)(pktp+2));
#if defined(__OPENRTOS__)
            if (gShairport.nCurrState == SHAIRPORT_PLAY && seqno % 30 ==0 && (shairportGetALStatus()==0 || shairportgetOtherProtocolPlaying())){
                // stop hairtune
                gShairport.nCurrState = HAIRTUNE_STOP;
                fprintf(stderr, "\Hairtune set status = stop \n");
                usleep(3000);
                i2s_deinit_DAC();

            }
            if (gShairport.nCurrState != SHAIRPORT_PLAY){
                usleep(15*1000);
                continue;
            }
            if (seqno % 1500 ==0){
                gettimeofday(&tv1, NULL);
                fprintf(stderr,"Airplay %ld %ld %ld %ld \n",tv1.tv_sec,shairportGetALStatus(),shairportGetException(),(ab_write-ab_read));
                if (shairportGetException()==1){
                    // get new data, do not restart
                    fprintf(stderr,"Airplay get new data, do not restart \n");
                    shairportSetException(0);
                }
                
            }
            
            if (shairportGetException()==2){
                // get new client, do not play
                usleep(15*1000);
                continue;
            }
            
            if (hairtunesAudioOutputWake()){
                if (ab_write-ab_read > BUFFER_FRAMES/2){
                    usleep(3000);                
                }
            }
            if ( (ab_write-ab_read > BUFFER_FRAMES-100) && gnMissingFrame!=1){
                usleep(3000);
          // printf("[Hairtuen] rtp_sleep %d \n",ab_write-ab_read);
            }
            if (gnPause){
                usleep(3000);
            }
            
#endif
////
#if 0
            gnLatePacketNumber++;
            if (gnLatePacketNumber%100==0){
                gnLatePacketId = seqno;
                printf("[Hairtune]packets %d %04X \n",gnLatePacketNumber,seqno);
            }            
            if (gnLatePacketNumber>=5000){
                seqno = gnLatePacketId;
                gnLatePacketNumber = 5000;
            }
#endif            
////
            buffer_put_packet(seqno, pktp+12, plen-12);
        }
    }

    return 0;
}

static void rtp_request_resend(seq_t first, seq_t last) {
    char req[8];    // *not* a standard RTCP NACK
    if (seq_order(last, first))
        return;

    if (last-first+1>5)
        fprintf(stderr, "requesting resend on %d packets (port %d)\n", last-first+1, controlport);

    req[0] = 0x80;
    req[1] = 0x55|0x80;  // Apple 'resend'
    *(unsigned short *)(req+2) = htons(1);  // our seqnum
    *(unsigned short *)(req+4) = htons(first);  // missed seqnum
    *(unsigned short *)(req+6) = htons(last-first+1);  // count

#ifdef AF_INET6
    rtp_client.sin6_port = htons(controlport);
#else
    rtp_client.sin_port = htons(controlport);
#endif
    sendto(rtp_sockets[1], req, sizeof(req), 0, (struct sockaddr *)&rtp_client, sizeof(rtp_client));
}


static int init_rtp(void) {
    struct sockaddr_in si;
    int type = AF_INET;
    struct sockaddr* si_p = (struct sockaddr*)&si;
    socklen_t si_len = sizeof(si);
    unsigned short *sin_port = &si.sin_port;
    int sock = -1, csock = -1;    // data and control (we treat the streams the same here)
    unsigned short port = 6000;
    pthread_t rtp_thread;

    pthread_attr_t attr;

    	struct sched_param param;
    	pthread_attr_init(&attr);
    	pthread_attr_setstacksize(&attr, 102400);
        param.sched_priority = 3;//sched_get_priority_max(1);
    	pthread_attr_setschedparam(&attr, &param);
    	//pthread_create(&task, &attr, iteMacThreadFunc, NULL);
    
    memset(&si, 0, sizeof(si));
#ifdef AF_INET6
    struct sockaddr_in6 si6;
    type = AF_INET6;
    si_p = (struct sockaddr*)&si6;
    si_len = sizeof(si6);
    sin_port = &si6.sin6_port;
    memset(&si6, 0, sizeof(si6));
#endif

    si.sin_family = AF_INET;
#ifdef SIN_LEN
    si.sin_len = sizeof(si);
#endif
    si.sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef AF_INET6
    si6.sin6_family = AF_INET6;
    #ifdef SIN6_LEN
        si6.sin6_len = sizeof(si);
    #endif
    si6.sin6_addr = in6addr_any;
    si6.sin6_flowinfo = 0;
#endif

    while(1) {
        int bind1;
        int bind2;
        if(sock < 0)
            sock = socket(type, SOCK_DGRAM, IPPROTO_UDP);
#ifdef AF_INET6
        if(sock==-1 && type == AF_INET6) {
            // try fallback to IPv4
            type = AF_INET;
            si_p = (struct sockaddr*)&si;
            si_len = sizeof(si);
            sin_port = &si.sin_port;
            continue;
        }
#endif
        if (sock==-1)
            die("Can't create data socket!");

        if(csock < 0)
            csock = socket(type, SOCK_DGRAM, IPPROTO_UDP);
        if (csock==-1)
            die("Can't create control socket!");

        *sin_port = htons(port);
        bind1 = bind(sock, si_p, si_len);
        *sin_port = htons(port + 1);
        bind2 = bind(csock, si_p, si_len);

        if(bind1 != -1 && bind2 != -1) break;
        if(bind1 != -1) { close(sock); sock = -1; }
        if(bind2 != -1) { close(csock); csock = -1; }

        port += 3;
    }

    printf("port: %d\n", port); // let our handler know where we end up listening
    printf("cport: %d\n", port+1);

    rtp_sockets[0] = sock;
    rtp_sockets[1] = csock;

    pthread_create(&rtp_thread, &attr, rtp_thread_func, (void *)rtp_sockets);
    
    //pthread_create(&rtp_thread, NULL, rtp_thread_func, (void *)rtp_sockets);

    return port;
}

static short lcg_rand(void) {
	static unsigned long lcg_prev = 12345;
	lcg_prev = lcg_prev * 69069 + 3;
	return lcg_prev & 0xffff;
}

static inline short dithered_vol(short sample) {
    static short rand_a, rand_b;
    long out;

    out = (long)sample * fix_volume;
    if (fix_volume < 0x10000) {
        rand_b = rand_a;
        rand_a = lcg_rand();
        out += rand_a;
        out -= rand_b;
    }
    return out>>16;
}

typedef struct {
    double hist[2];
    double a[2];
    double b[3];
} biquad_t;

static void biquad_init(biquad_t *bq, double a[], double b[]) {
    bq->hist[0] = bq->hist[1] = 0.0;
    memcpy(bq->a, a, 2*sizeof(double));
    memcpy(bq->b, b, 3*sizeof(double));
}

static void biquad_lpf(biquad_t *bq, double freq, double Q) {
    double w0 = 2.0 * M_PI * freq * frame_size / (double)sampling_rate;
    double alpha = sin(w0)/(2.0*Q);

    double a_0 = 1.0 + alpha;
    double b[3], a[2];
    b[0] = (1.0-cos(w0))/(2.0*a_0);
    b[1] = (1.0-cos(w0))/a_0;
    b[2] = b[0];
    a[0] = -2.0*cos(w0)/a_0;
    a[1] = (1-alpha)/a_0;

    biquad_init(bq, a, b);
}

static double biquad_filt(biquad_t *bq, double in) {
    double w = in - bq->a[0]*bq->hist[0] - bq->a[1]*bq->hist[1];
    double out = bq->b[1]*bq->hist[0] + bq->b[2]*bq->hist[1] + bq->b[0]*w;
    bq->hist[1] = bq->hist[0];
    bq->hist[0] = w;

    return out;
}

static double bf_playback_rate = 1.0;

static double bf_est_drift = 0.0;   // local clock is slower by
static biquad_t bf_drift_lpf;
static double bf_est_err = 0.0, bf_last_err;
static biquad_t bf_err_lpf, bf_err_deriv_lpf;
static double desired_fill;
static int fill_count;

static void bf_est_reset(short fill) {
    biquad_lpf(&bf_drift_lpf, 1.0/180.0, 0.3);
    biquad_lpf(&bf_err_lpf, 1.0/10.0, 0.25);
    biquad_lpf(&bf_err_deriv_lpf, 1.0/2.0, 0.2);
    fill_count = 0;
    bf_playback_rate = 1.0;
    bf_est_err = bf_last_err = 0;
    desired_fill = fill_count = 0;
}

static void bf_est_update(short fill) {
    double buf_delta;
    double err_deriv;
    double adj_error;
    if (fill_count < 1000) {
        desired_fill += (double)fill/1000.0;
        fill_count++;
        return;
    }

#define CONTROL_A   (1e-4)
#define CONTROL_B   (1e-1)

    buf_delta = fill - desired_fill;
    bf_est_err = biquad_filt(&bf_err_lpf, buf_delta);
    err_deriv = biquad_filt(&bf_err_deriv_lpf, bf_est_err - bf_last_err);
    adj_error = CONTROL_A * bf_est_err;

    bf_est_drift = biquad_filt(&bf_drift_lpf, CONTROL_B*(adj_error + err_deriv) + bf_est_drift);

    if (debug)
        fprintf(stderr, "bf %d err %f drift %f desiring %f ed %f estd %f\n",
                fill, bf_est_err, bf_est_drift, desired_fill, err_deriv, err_deriv + adj_error);
    bf_playback_rate = 1.0 + adj_error + bf_est_drift;

    bf_last_err = bf_est_err;
}

// get the next frame, when available. return 0 if underrun/stream reset.
static short *buffer_get_frame(void) {
    short buf_fill;
    seq_t read;
    abuf_t *abuf = 0;
    unsigned short next;
    int i;
    abuf_t *curframe;
#ifdef TEMP_SOLUTION    
// temp return
    return 0;
#endif

    pthread_mutex_lock(&ab_mutex);
    buf_fill = ab_write - ab_read;
#ifdef DEMO_SOLUTION    
    if (buf_fill < 1 || !ab_synced || ab_buffering/* || ab_init==0*/) {    // init or underrun. stop and wait
        if (ab_synced){
            fprintf(stderr, "\nunderrun %d %d %d %d write %d ab_read %d %d \n",buf_fill,ab_synced,ab_buffering,gDecode,ab_write,ab_read,gShairport.nCurrState);
        #if defined(__OPENRTOS__)
            if (gShairport.nCurrState==SHAIRPORT_PLAY){
                shairportSetException(1);
            }
        #endif
        }

        ab_buffering = 1;
        pthread_cond_wait(&ab_buffer_ready, &ab_mutex);
        ab_read++;
        buf_fill = ab_write - ab_read;
        //bf_est_reset(buf_fill);
        fprintf(stderr, "\n pthread_cond_wait ~~~~%d   ~~~~~~~~~~~~~~~~~~~%d  ~~~~~~~~%d   ~~~~~~~~~~\n",ab_buffering,buf_fill,gDecode);
        if (I2S_DA32_GET_RP() ==0 ){
            printf("%d ,i2s rd ptr %d wr %d  \n",shairportGetException(),I2S_DA32_GET_RP(),I2S_DA32_GET_RP());                    
            gnPause = 1;
            i2s_pause_DAC(gnPause);
        }
        pthread_mutex_unlock(&ab_mutex);

        return 0;
    }
#else
    if (buf_fill < 1 || !ab_synced || ab_buffering) {    // init or underrun. stop and wait
        if (ab_synced)
            fprintf(stderr, "\nunderrun.\n");

        ab_buffering = 1;
        pthread_cond_wait(&ab_buffer_ready, &ab_mutex);
        ab_read++;
        buf_fill = ab_write - ab_read;
        //bf_est_reset(buf_fill);
        fprintf(stderr, "\n pthread_cond_wait ~~~~%d   ~~~~~~~~~~~~~~~~~~~%d  ~~~~~~~~%d   ~~~~~~~~~~\n",ab_buffering,buf_fill,gDecode);
        pthread_mutex_unlock(&ab_mutex);

        return 0;
    }
    
#endif
    if (ab_read % 1100 == 0){
        gettimeofday(&tvO, NULL);
        fprintf(stderr,"Airplay output %ld %ld \n",tvO.tv_sec,ab_read);
    }

    if (buf_fill >= BUFFER_FRAMES) {   // overrunning! uh-oh. restart at a sane distance
        fprintf(stderr, "\noverrun.\n");
        ab_read = ab_write - buffer_start_fill;
    }
    read = ab_read;
    ab_read++;
    buf_fill = ab_write - ab_read;
#ifndef DEMO_SOLUTION        
    bf_est_update(buf_fill);
#endif

    // check if t+16, t+32, t+64, t+128, ... (START_FILL / 2)
    // packets have arrived... last-chance resend
    if (!ab_buffering) {
        for (i = 16; i < (START_FILL / 2); i = (i * 2)) {
            next = ab_read + i;
            abuf = audio_buffer + BUFIDX(next);
            if (!abuf->ready && next < ab_write) {
                //printf("[Hairtune] resend %d read %d wirte %d \n",next,ab_read,ab_write);
                rtp_request_resend(next, next+1);
            }
        }
    }


    curframe = audio_buffer + BUFIDX(read);
    if (!curframe->ready && gShairport.nCurrState == SHAIRPORT_PLAY) {
        if (gnMissingFramePrint%20 ==0) {
            fprintf(stderr, "\n missing frame  %d %04X \n",BUFIDX(read),read);
            gnMissingFramePrint = 0;
        }
        memset(curframe->data, 0, FRAME_BYTES);
        gnMissingFrame = 1;
        gnMissingFramePrint++;
        // content switch
        usleep(5);
    } else {
        //fprintf(stderr, " read %d \n",ab_read);
        gnMissingFrame = 0;
    }
    curframe->ready = 0;
    pthread_mutex_unlock(&ab_mutex);

    return curframe->data;
}

static int stuff_buffer(double playback_rate, short *inptr, short *outptr) {
    int i;
    int stuffsamp = frame_size;
    int stuff = 0;
    double p_stuff;

    p_stuff = 1.0 - pow(1.0 - fabs(playback_rate-1.0), frame_size);

    if (rand() < p_stuff * RAND_MAX) {
        stuff = playback_rate > 1.0 ? -1 : 1;
        stuffsamp = rand() % (frame_size - 1);
    }

    pthread_mutex_lock(&vol_mutex);
    for (i=0; i<stuffsamp; i++) {   // the whole frame, if no stuffing
        *outptr++ = dithered_vol(*inptr++);
        *outptr++ = dithered_vol(*inptr++);
    };
    if (stuff) {
        if (stuff==1) {
            if (debug)
                fprintf(stderr, "+++++++++\n");
            // interpolate one sample
            *outptr++ = dithered_vol(((long)inptr[-2] + (long)inptr[0]) >> 1);
            *outptr++ = dithered_vol(((long)inptr[-1] + (long)inptr[1]) >> 1);
        } else if (stuff==-1) {
            if (debug)
                fprintf(stderr, "---------\n");
            inptr++;
            inptr++;
        }
        for (i=stuffsamp; i<frame_size + stuff; i++) {
            *outptr++ = dithered_vol(*inptr++);
            *outptr++ = dithered_vol(*inptr++);
        }
    }
    pthread_mutex_unlock(&vol_mutex);
    return frame_size + stuff;
}

void audio_output(signed short *outbuf , int samples)
{
    int tmp;
    int nTemp;

#if defined(__OPENRTOS__)

    // Wait output buffer avaliable
    do {       
        gOutReadPointer = I2S_DA32_GET_RP();
        if (gOutReadPointer <= gOutWritePointer) {
            nTemp = I2S_BUFFER_LENGTH - (gOutWritePointer - gOutReadPointer);
        } else {
            nTemp = gOutReadPointer - gOutWritePointer;
        }
        if ((nTemp-2) < FRAME_BYTES*2) {
            usleep(200000);
        } else {
            break;
        }
    } while(1);
    if (!hairtunesAudioOutputWake() && nTemp < FRAME_BYTES*750){
        //fprintf(stderr, " sleep %d \n",ab_read);
        usleep(200000);
    }

    // avoid iOS device 100~200 ms mute problem
    if (gnPause){
        if (gOutWritePointer>=FRAME_BYTES*15){
            gnPause = 0;
            memset(gOutBuffer+I2S_DA32_GET_RP(),0,FRAME_BYTES*15);
            //ithInvalidateDCache();
            //printf("i2s rd ptr %d wr %d \n",I2S_DA32_GET_RP(),I2S_DA32_GET_WP());            
            i2s_pause_DAC(gnPause);
        }
    }
    
    //fprintf(stderr, " sleep %d \n",nTemp);
    if (gOutWritePointer+(samples*4) < I2S_BUFFER_LENGTH){
        memcpy(gOutBuffer+ gOutWritePointer, outbuf, samples*4);
        gOutWritePointer += samples*4;
        if(gOutWritePointer >= I2S_BUFFER_LENGTH) 
        {
            gOutWritePointer = 0;
        }
        I2S_DA32_SET_WP(gOutWritePointer);
    }else{
        nTemp = I2S_BUFFER_LENGTH - gOutWritePointer;
        memcpy(gOutBuffer + gOutWritePointer, outbuf, nTemp);
        gOutWritePointer += nTemp;
        I2S_DA32_SET_WP(gOutWritePointer);
        tmp = samples*4 - nTemp;
        memcpy(gOutBuffer, &outbuf[nTemp/2], tmp);
        gOutWritePointer = tmp;
        I2S_DA32_SET_WP(gOutWritePointer);
    }
 // fprintf(stderr, "[Hairtune] write %d %d\n",samples*4,gOutWritePointer);
#endif

}

static void *audio_thread_func(void *arg) {
    ao_device* dev = arg;
    int play_samples;
    int tmp;
    int nTemp;
    signed short buf_fill __attribute__((unused));
    signed short *inbuf, *outbuf;
    outbuf = malloc(OUTFRAME_BYTES);

#ifdef FANCY_RESAMPLING
    float *frame, *outframe;
    SRC_DATA srcdat;
    if (fancy_resampling) {
        frame = malloc(frame_size*2*sizeof(float));
        outframe = malloc(2*frame_size*2*sizeof(float));

        srcdat.data_in = frame;
        srcdat.data_out = outframe;
        srcdat.input_frames = FRAME_BYTES;
        srcdat.output_frames = 2*FRAME_BYTES;
        srcdat.src_ratio = 1.0;
        srcdat.end_of_input = 0;
    }
#endif

    while (1) {
        if (shairportGetException()==2){
            // get new client, do not play
            usleep(15*1000);
            continue;
        }
        
        do {
            inbuf = buffer_get_frame();
            if (!hairtunesAudioOutputWake())
                usleep(2000);
        } while (!inbuf);

#ifdef FANCY_RESAMPLING
        if (fancy_resampling) {
            int i;
            pthread_mutex_lock(&vol_mutex);
            for (i=0; i<2*FRAME_BYTES; i++) {
                frame[i] = (float)inbuf[i] / 32768.0;
                frame[i] *= volume;
            }
            pthread_mutex_unlock(&vol_mutex);
            srcdat.src_ratio = bf_playback_rate;
            src_process(src, &srcdat);
            assert(srcdat.input_frames_used == FRAME_BYTES);
            src_float_to_short_array(outframe, outbuf, FRAME_BYTES*2);
            play_samples = srcdat.output_frames_gen;
        } else
#endif


#if 0 //def DEMO_SOLUTION
            outbuf = inbuf;
            play_samples = FRAME_BYTES/4;
#else
            play_samples = stuff_buffer(bf_playback_rate, inbuf, outbuf);
#endif
#ifdef WIN32
        if (pipename) {
            if (pipe_handle == -1) {
                // attempt to open pipe - block if there are no readers
                pipe_handle = open(pipename, O_WRONLY);
            }

            // only write if pipe open (there's a reader)
            if (pipe_handle != -1) {
                 if (write(pipe_handle, outbuf, play_samples*4) == -1) {
                    // write failed - do anything here?
                    // SIGPIPE is handled elsewhere...
                 }
            }
        } else {
            ao_play(dev, (char *)outbuf, play_samples*4);
        }
#else
    #ifdef TEMP_SOLUTION

    #else
        gOutReadPointer = I2S_DA32_GET_RP();
        if (gOutReadPointer <= gOutWritePointer) {
            nTemp = gOutWritePointer - gOutReadPointer;
        } else {
            nTemp = I2S_BUFFER_LENGTH - (gOutReadPointer - gOutWritePointer);        
        }
        if (!hairtunesAudioOutputWake() && nTemp>64*FRAME_BYTES){
            usleep(20000);
        }
    
        if (gShairport.nCurrState == SHAIRPORT_PLAY)
            audio_output(outbuf,play_samples);
        //else
         //   usleep(20000);
    #endif
#endif
      //fprintf(stderr, "[Hairtune] write %d \n",play_samples*4);

    }

    return 0;
}

#define NUM_CHANNELS 2

static void handle_broken_fifo() {
    close(pipe_handle);
    pipe_handle = -1;
}

static void init_pipe(const char* pipe) {
    // make the FIFO and catch the broken pipe signal
    mknod(pipe, S_IFIFO | 0644, 0);
    signal(SIGPIPE, handle_broken_fifo);
}

static void* init_ao(void) {
    int driver;
#ifdef WIN32
    ao_sample_format fmt;
    ao_option *ao_opts = NULL;
    ao_device *dev;
    ao_initialize();

    if (libao_driver) {
        // if a libao driver is specified on the command line, use that
        driver = ao_driver_id(libao_driver);
        if (driver == -1) {
            die("Could not find requested ao driver");
        }
    } else {
        // otherwise choose the default
        driver = ao_default_driver_id();
    }

    memset(&fmt, 0, sizeof(fmt));

    fmt.bits = 16;
    fmt.rate = sampling_rate;
    fmt.channels = NUM_CHANNELS;
    fmt.byte_format = AO_FMT_NATIVE;

    if(libao_deviceid) {
        ao_append_option(&ao_opts, "id", libao_deviceid);
    } else if(libao_devicename){
        ao_append_option(&ao_opts, "dev", libao_devicename);
        // Old libao versions (for example, 0.8.8) only support
        // "dsp" instead of "dev".
        ao_append_option(&ao_opts, "dsp", libao_devicename);
    }

    dev = ao_open_live(driver, &fmt, ao_opts);
    if (dev == NULL) {
        die("Could not open ao device");
    }

    return dev;
#endif    
}

#if defined(__OPENRTOS__)
void init_output_i2s(void){

    unsigned char* I2SBuf;
    int nChannels = 2;
    int nSampeRate = 44100;
    int nBufferLength;
    int nTemp = 1;
    unsigned int* pBuf;
    unsigned int nLength;
    STRC_I2S_SPEC spec;
    
    /* init I2S */
    //if (getShairportWifiMode()==1){

    //} else {
        i2s_deinit_DAC();
    //}
    
    memset(&spec,0,sizeof(spec));
    memset(gOutBuffer,0,sizeof(gOutBuffer));
    spec.channels           = nChannels;
    spec.sample_rate      = nSampeRate;
    spec.buffer_size        = I2S_BUFFER_LENGTH;
    spec.is_big_endian    = 0;
    spec.base_i2s           = gOutBuffer;

    spec.sample_size      = 16;
    spec.num_hdmi_audio_buffer    = 1;
    spec.is_dac_spdif_same_buffer = 1;
    spec.base_hdmi[0]    = gOutBuffer;
    spec.base_hdmi[1]    = gOutBuffer;
    spec.base_hdmi[2]    = gOutBuffer;
    spec.base_hdmi[3]    = gOutBuffer;
    spec.base_spdif         = gOutBuffer;

    i2s_init_DAC(&spec);
}
#endif

static int init_output(void) {
    void* arg = 0;
    pthread_attr_t attr;
    pthread_t audio_thread;
    int nTmp;

    struct sched_param param;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 102400);
    param.sched_priority = 2;//sched_get_priority_max(1);
    pthread_attr_setschedparam(&attr, &param);
    //pthread_create(&task, &attr, iteMacThreadFunc, NULL);
    

#ifdef WIN32
    if (pipename) {
        init_pipe(pipename);
    } else {
        arg = init_ao();
    }
#else
    //init_output_i2s();
#endif

#ifdef PCM_DUMP
    if (fin==NULL && (fin = fopen(inFileName, "wb")) == NULL){
        fprintf(stderr, "[Hairtune] Can not open '%s' \n",inFileName);
    }

#endif

#ifdef FANCY_RESAMPLING
    int err;
    if (fancy_resampling)
        src = src_new(SRC_SINC_MEDIUM_QUALITY, 2, &err);
    else
        src = 0;
#endif
    nTmp = pthread_mutex_init(&ab_mutex, NULL);
    if (nTmp != 0){
        fprintf(stderr,"[Hairtune] Can not open ab_mutex %d \n",nTmp);
    }
    pthread_cond_init (&ab_buffer_ready, NULL);

//    pthread_create(&audio_thread, &attr, audio_thread_func, arg);
    pthread_create(&audio_thread, NULL, audio_thread_func, arg);

    return 0;
}

