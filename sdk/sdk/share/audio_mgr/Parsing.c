#ifdef CFG_AUDIO_MGR_PARSING_MP3

// Mp3    
const static int MP3_HEADER_SIZE = 4;
/* indexing = [version][samplerate index]
 * sample rate of frame (Hz)
 */
const static int samplerateTab[3][3] = 
{
    {44100, 48000, 32000},        /* MPEG-1 */
    {22050, 24000, 16000},        /* MPEG-2 */
    {11025, 12000,  8000},        /* MPEG-2.5 */
};

const static int channelTab[4] = 
{
    2,      /* STEREO TAG = 0 */
    2,      /* JOIN TAG = 1 */
    2,      /* DUAL TAG = 2 */
    1       /* MONO TAG = 3 */
};

/* indexing = [version][layer][bitrate index]
 * bitrate (kbps) of frame
 *   - bitrate index == 0 is "free" mode (bitrate determined on the fly by
 *       counting bits between successive sync words)
 */
const static int bitrateTab[3][3][15] = 
{
    {
        /* MPEG-1 */
        {  0, 32, 64, 96,128,160,192,224,256,288,320,352,384,416,448}, /* Layer 1 */
        {  0, 32, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,384}, /* Layer 2 */
        {  0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320}, /* Layer 3 */
    },
    {
        /* MPEG-2 */
        {  0, 32, 48, 56, 64, 80, 96,112,128,144,160,176,192,224,256}, /* Layer 1 */
        {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160}, /* Layer 2 */
        {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160}, /* Layer 3 */
    },
    {
        /* MPEG-2.5 */
        {  0, 32, 48, 56, 64, 80, 96,112,128,144,160,176,192,224,256}, /* Layer 1 */
        {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160}, /* Layer 2 */
        {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160}, /* Layer 3 */
    },
};

/* indexing = [version][layer]
 * number of samples in one frame (per channel)
 */
const static int samplesPerFrameTab[3][3] = 
{
    {384, 1152, 1152 }, /* MPEG1 */
    {384, 1152,  576 }, /* MPEG2 */
    {384, 1152,  576 }, /* MPEG2.5 */
};

/* indexing = [version][sampleRate][bitRate]
 * for layer3, nSlots = floor(samps/frame * bitRate / sampleRate / 8)
 *   - add one pad slot if necessary
 */
const static int slotTab[3][3][15] = 
{
    {
        /* MPEG-1 */
        { 0, 104, 130, 156, 182, 208, 261, 313, 365, 417, 522, 626, 731, 835,1044 },    /* 44 kHz */
        { 0,  96, 120, 144, 168, 192, 240, 288, 336, 384, 480, 576, 672, 768, 960 },    /* 48 kHz */
        { 0, 144, 180, 216, 252, 288, 360, 432, 504, 576, 720, 864,1008,1152,1440 },    /* 32 kHz */
    },
    {
        /* MPEG-2 */
        { 0,  26,  52,  78, 104, 130, 156, 182, 208, 261, 313, 365, 417, 470, 522 },    /* 22 kHz */
        { 0,  24,  48,  72,  96, 120, 144, 168, 192, 240, 288, 336, 384, 432, 480 },    /* 24 kHz */
        { 0,  36,  72, 108, 144, 180, 216, 252, 288, 360, 432, 504, 576, 648, 720 },    /* 16 kHz */
    },
    {
        /* MPEG-2.5 */
        { 0,  52, 104, 156, 208, 261, 313, 365, 417, 522, 626, 731, 835, 940,1044 },    /* 11 kHz */
        { 0,  48,  96, 144, 192, 240, 288, 336, 384, 480, 576, 672, 768, 864, 960 },    /* 12 kHz */
        { 0,  72, 144, 216, 288, 360, 432, 504, 576, 720, 864,1008,1152,1296,1440 },    /*  8 kHz */
    },
};

#define PARSING_BUFFER_SIZE 80*1024

// parsing buffer read index
static int parsingRIdx=0;
// parsing buffer write index
static int parsingWIdx=0;
static char parsingBuffer[PARSING_BUFFER_SIZE];

static int gnCh = 0;
static int gnSampleRate = 0;
static int gnSampleRateIndex = 0;
static int gnInitSampleRate =0;
static int gnSamplePerFrame = 0;
static int gnType =0;
static int gnFrames = 0;

static int sampleRateHistory[4][2];
static int samplePerFrameHistory[3];

// pBuf is pointer to buf, nBufSize is buffer size,pParsingSize is the number of paring size
// return audio frame count 
int parsing_mp3_data(char* pBuf,int nBufSize,int* pParsingSize);

int parsing_aac_data(char* pBuf,int nBufSize,int* pParsingSize);

int parsing_audio_init();

int parsing_mp3(char* parsing_buf,int size,int* pParsingSize);

int parsing_aac(char* parsing_buf,int size,int* pParsingSize);



int parsing_mp3_data(char* pBuf,int nBufSize,int* pParsingSize)
{
    return parsing_mp3(pBuf,nBufSize,pParsingSize);
}

int parsing_aac_data(char* pBuf,int nBufSize,int* pParsingSize)
{
    return parsing_aac(pBuf,nBufSize,pParsingSize);
}

int parsing_data_init()
{
    int nTemp ;
    parsingRIdx = 0;
    parsingWIdx = 0;
    memset(parsingBuffer,0,PARSING_BUFFER_SIZE);
    memset(sampleRateHistory,0,sizeof(sampleRateHistory));
    memset(samplePerFrameHistory,0,sizeof(samplePerFrameHistory));
    gnType = 0;
    gnFrames = 0;
    gnSampleRateIndex = 0;
    nTemp = parsing_audio_init();
    return 0;
}

int parsing_data(char* pBuf,int nBufSize,int nType)
{
    int nParsingSize=0;
    int nFrame =0;
    // prepare parsing buffer
    if (parsingWIdx>sizeof(parsingBuffer)){
        parsingWIdx = 0;
        return 0;
    } 
    if (parsingWIdx+nBufSize > sizeof(parsingBuffer)){
        parsingWIdx = (sizeof(parsingBuffer) - nBufSize);
    
    }
    memcpy(&parsingBuffer[parsingWIdx],pBuf,nBufSize);
    parsingWIdx+=nBufSize;

    if (parsingWIdx>sizeof(parsingBuffer)){
        parsingWIdx = 0;
    }

    //
    switch(nType)
    {
        // parsing mp3
        case 1:
            nFrame = parsing_mp3_data(parsingBuffer,parsingWIdx,&nParsingSize);
            break;

        // parsing aac
        case 2:
            nFrame = parsing_aac_data(parsingBuffer,parsingWIdx,&nParsingSize);
            break;


        default:
            break;
    }
    
    // copy unparsing data to parsing buffer
    if (parsingWIdx-nParsingSize>0){
    memcpy(parsingBuffer,&parsingBuffer[parsingRIdx],parsingWIdx-nParsingSize);
    } else {
        return nFrame;
    }
    // reset read,write pointer
    parsingRIdx = 0;
    parsingWIdx = parsingWIdx-nParsingSize;
    gnType = nType;
    // reset part of parsing buffer
    if (nBufSize-parsingWIdx>0){
    memset(&parsingBuffer[parsingWIdx],0,nBufSize-parsingWIdx);
    } else {
        return nFrame;
    }
    gnFrames += nFrame;
    return nFrame;
}


//384, 1152,  576
int parsing_audio_init()
{

    gnCh = 0;
    gnSampleRate = 0;
    gnSamplePerFrame = 0;

    return 0;
}

int parsing_audio_check(int samplerate)
{
    int i=0,j=0;    
    
    if (384 == gnSamplePerFrame){
        samplePerFrameHistory[0]++;
    } else if (1152 == gnSamplePerFrame){
        samplePerFrameHistory[1]++;
    } else if (576 == gnSamplePerFrame){
        samplePerFrameHistory[2]++;
    }

    if (samplerate==gnSampleRate){
        if (sampleRateHistory[0][0]==0){
            sampleRateHistory[0][0] =gnSampleRate;
            sampleRateHistory[0][1] =1;
            printf("init sample rate %d \n",sampleRateHistory[0][0]);
            gnSampleRateIndex=1;
            gnInitSampleRate = gnSampleRate;
            return 0;
        }
        
         //if (sampleRateHistory[0[0] == gnSampleRate){
         //   sampleRateHistory[0][1]++;
          //  return 0;
      //   }
          
        for (j=0 ;j<4;j++){            
             if (sampleRateHistory[j][0] == gnSampleRate){
                sampleRateHistory[j][1]++;
                break;
            } else if (sampleRateHistory[j][0] == 0){
                sampleRateHistory[gnSampleRateIndex][0] = gnSampleRate; 
                sampleRateHistory[gnSampleRateIndex][1] = 1; 
                printf("add index %d sample rate %d \n",j,sampleRateHistory[gnSampleRateIndex][0]);
                if (gnSampleRateIndex<4)
                    gnSampleRateIndex++;
                break;
            } 
        }
        return 0;
    } else {
        //printf("parsing_audio_check sample rate diff %d,%d \n",samplerate,gnSampleRate);
        return 1;
    }
}

// nFrames is mp3 frame number
int parsing_mp3_current_time(int nFrames){
    int i,j =0;
    for (i=0;i<2;i++){
        if (samplePerFrameHistory[j]<samplePerFrameHistory[i+1])
            j = i+1;
        else 
            j = j;
    }
    //384, 1152,  576
    if (j==0)
        gnSamplePerFrame = 384;
    else if (j==1)
        gnSamplePerFrame = 1152;
    else if (j==2)
        gnSamplePerFrame = 576;
    
    j = 0;
    for (i=0;i<4;i++){
        if (sampleRateHistory[j][1]<sampleRateHistory[i+1][1])
            j = i+1;
        else
            j = j;
    }
    gnSampleRate = sampleRateHistory[j][0];

    if (gnSampleRate< 8000 || gnSampleRate>48000)
        gnSampleRate = smtkAudioMgrSampleRate();
    
    if (gnSampleRate) {
        // seconds
        //printf("mp3_time %d %d %d  %d %d \n",nFrames,gnSamplePerFrame,gnSampleRate,sampleRateHistory[j][1],gnInitSampleRate);
        return (nFrames*gnSamplePerFrame)/gnSampleRate;
    } else {
        printf("[Parsing] Error gnSampleRate %d \n",gnSampleRate);
        return 0;
    }
/*
        #ifdef SMTK_AUDIO_SET_TIME_BY_MILLISECOND
            if ( ((*frameCounts)*mp3Info.samplesperframe/(mp3Info.sampling_rate/10) ) - fileParsingSecond>=1 )
            {
                fileParsingSecond = ((*frameCounts)*mp3Info.samplesperframe)/(mp3Info.sampling_rate/10) ;
                if (fileParsingSecond <SECOND_NUMBER)
                {
                    fileSecPos[fileParsingSecond] = *pos+i -size;                         
                }
                else if (fileParsingSecond >= SECOND_NUMBER)
                {
                    fileSecPosComplete=MMP_TRUE;
                    return -1;                            
                }
            }
        #else                     
             if ( ((*frameCounts)*mp3Info.samplesperframe/mp3Info.sampling_rate) - fileParsingSecond>=1 )
             {
                 fileParsingSecond = ((*frameCounts)*mp3Info.samplesperframe)/mp3Info.sampling_rate ;
                 //dbg_msg(DBG_MSG_TYPE_INFO, "Audio Mgr fileParsingSecond %d \n",fileParsingSecond);
                 if (fileParsingSecond <SECOND_NUMBER)                         
                 {
                     fileSecPos[fileParsingSecond] = *pos+i -size;                         
                 }
                 else if (fileParsingSecond >= SECOND_NUMBER)
                 {
                     fileSecPosComplete=MMP_TRUE;
                     //dbg_msg(DBG_MSG_TYPE_INFO, "audio_parsing_mp3_file size fileSecPosComplete -1 \n");                             
                     return -1;                             
                 }                         
             }
         #endif
*/               
}

// nFrames is aac frame number
int parsing_aac_current_time(int nFrames){
    
    if (gnSampleRate) {
        //printf("aac_time %d %d %d  \n",nFrames,gnSamplePerFrame,gnSampleRate);

        return ((nFrames*gnSamplePerFrame)/gnSampleRate);
    } else {
        printf("[Parsing] Error gnSampleRate %d \n",gnSampleRate);
        return 0;
    }
}

int parsing_data_get_current_time()
{
    int nDuration;
    switch(gnType)
    {
        // parsing mp3
        case 1:
            nDuration = parsing_mp3_current_time(gnFrames);
            nDuration*= 1000;
            break;
        case 2:
            nDuration = parsing_aac_current_time(gnFrames);
            nDuration*= 1000;
            break;

        default:
            break;
    }
    return nDuration;
}

int parsing_mp3(char* parsing_buf,int size,int* pParsingSize)
{
    int i=0;
    int layer;
    int brIdx;
    int srIdx;
    int id;
    int ver;
    int paddingBit;
    int bitRate;
    int sampling_rate;
    int samplesperframe;
    int frameLength=0;
    int frameCounts = 0;

    for(i=0; i+2<size; i++) {
    /*    audioState = smtkAudioMgrGetState();
        if (audioState != SMTK_AUDIO_PLAY && audioState != SMTK_AUDIO_PAUSE) {
            //dbg_msg(DBG_MSG_TYPE_INFO, "[Audio] Leave playing state \n");
            break;
        }        
    */
        if (!( (char)parsing_buf[i] == (char)0xff && ((parsing_buf[i+1] & 0xe0) == 0xe0) )) {    
            continue;
        }        
        
        layer = 4 - ((parsing_buf[i+1] >> 1) & 0x3);
        brIdx = (parsing_buf[i+2] >> 4) & 0xf;
        srIdx = (parsing_buf[i+2] >> 2) & 0x3;

        if (srIdx == 3 || layer == 4 || brIdx == 15){
            continue; // illegal frame
        }

        /* keep frame information */
        
        id  = (parsing_buf[i+1] >> 3) & 0x3;
        ver = (id == 0 ? 2 : (id & 0x1 ? 0 : 1));
        paddingBit = (parsing_buf[i+2]  >> 1) & 0x01;                
        bitRate;
        //int crc = (buffer[i+1] & 0x1);

        sampling_rate = samplerateTab[ver][srIdx];
        samplesperframe = samplesPerFrameTab[ver][layer-1];
        if (layer ==3) {
            frameLength = slotTab[ver][srIdx][brIdx]+ paddingBit;
        } else if (layer ==2) {
            bitRate = bitrateTab[ver][layer-1][brIdx]*1000;
            if (sampling_rate)
                frameLength = (144 * bitRate / sampling_rate) + paddingBit;
        } else if (layer == 1) {
            bitRate = bitrateTab[ver][layer-1][brIdx]*1000;                    
            frameLength = ((12 * bitRate / sampling_rate) + paddingBit)*4;
        }

        if (gnSampleRate == 0) {
            gnSampleRate = sampling_rate;
            gnSamplePerFrame = samplesperframe;
        } else {
            if (parsing_audio_check(sampling_rate)){
                parsing_audio_init();
                continue;
            }
        }
        if (frameLength == 0){
           //(*frameCounts)++;
           continue;
        }

        *pParsingSize = i;
        i += frameLength-1 ;
        
        (frameCounts)++;
        //dbg_msg(DBG_MSG_TYPE_INFO, "[Audio] seek_MP3_header frameLength %d *frameCounts  %d fileParsingSecond %d\n",*frameLength,*frameCounts,fileParsingSecond );

    }
    if (*pParsingSize==0){
        *pParsingSize = size;
    } 
    return frameCounts;
}

#define SYNCWORDH           0xff
#define SYNCWORDL           0xf0

#define NUM_SAMPLE_RATES    12
#define MAX_AAC_FRAME_LENGTH 1500

/* sample rates (table 4.5.1) */
const int sampRateTab[NUM_SAMPLE_RATES] = {
    96000, 88200, 64000, 48000, 44100, 32000,
    24000, 22050, 16000, 12000, 11025,  8000
};

int parsing_aac(char* parsing_buf,int size,int* pParsingSize)
{
    int i=0;
    int sampling_rate;
    int samplesperframe;
    int frameLength=0;
    int frameCounts=0;
    int nTemp=0;

    for(i=0; i+2<size; i++) {

       
        if (!(((parsing_buf[i+0] & SYNCWORDH)== SYNCWORDH)&&((parsing_buf[i+1] & SYNCWORDL) == SYNCWORDL)))
        {
            continue;
        }

        if( ((parsing_buf[i+1] & 0x06) >> 1) != 0x0 )
            continue;

        // Sample rate index
        if( ((parsing_buf[i+2] & 0x3c) >> 2) >= NUM_SAMPLE_RATES ) {
            continue;
        }

        //frameLength = ((parsing_buf[i+3] & 0x03) << 11) + (parsing_buf[i+4] << 3) + ((parsing_buf[i+5] >> 5) /*& 0x7*/);
        frameLength = (((parsing_buf[i+3] << 16) + (parsing_buf[i+4] <<8) + (parsing_buf[i+5])) >> 5) & 0x1fff;
        //frameLength+=7;
        if(frameLength > MAX_AAC_FRAME_LENGTH)
            continue;      
    
        nTemp = (parsing_buf[i+2] & 0x3c) >> 2;
        sampling_rate = sampRateTab[nTemp];

        if (frameLength == 0){
           continue;
        }

        if (gnSampleRate == 0) {
            gnSampleRate = sampling_rate;
            gnSamplePerFrame = 1024;
        } else {
            if (parsing_audio_check(sampling_rate)){
                parsing_audio_init();
                continue;
            }
        }



        *pParsingSize = i;
        i += frameLength-1 ;
        
        (frameCounts)++;
        //dbg_msg(DBG_MSG_TYPE_INFO, "[Audio] seek_MP3_header frameLength %d *frameCounts  %d fileParsingSecond %d\n",*frameLength,*frameCounts,fileParsingSecond );

    }
    if (*pParsingSize==0){
        *pParsingSize = size;
    }
    return frameCounts;
}

#endif

