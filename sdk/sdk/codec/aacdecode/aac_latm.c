
#include <string.h>
#include "aac_latm.h"
#include "coder.h"

//#include "win32.h"
#if defined(ENABLE_CODECS_PLUGIN)
#include "plugin.h"
#endif
uint8_t b_latm_cfg;
latm_mux_t tLatm;
uint8_t* gpBuffer;

/****************************************************************************
 * LOAS helpers
 ****************************************************************************/
int LOASSyncInfo( uint8_t ptHeader[LOAS_HEADER_SIZE] )
{
    return ( ( ptHeader[1] & 0x1f ) << 8 ) + ptHeader[2];
}

int Mpeg4GAProgramConfigElement( BitStreamInfo *bsi )
{
    int nTag,nNumFront,nNumSide;
    int nNumBack,nNumLife,nNumAssocData,i_num_valid_cc;
    int i_comment;
    /* TODO compute channels count ? */
    nTag = GetBits(bsi, 4);
    if( nTag != 0x05 )
    {
        return -1;
    }
    GetBits(bsi, 6); // object type + sampling index
    nNumFront = GetBits(bsi, 4);
    nNumSide = GetBits(bsi, 4);
    nNumBack = GetBits(bsi, 4);
    nNumLife = GetBits(bsi, 2);
    nNumAssocData = GetBits(bsi, 3);
    i_num_valid_cc = GetBits(bsi, 4);
    if( GetBits(bsi, 1) ) 
    {
        GetBits(bsi, 4); // mono downmix
    }
    if( GetBits(bsi, 1) )
    {
        GetBits(bsi, 4);// stereo downmix
    }
    if( GetBits(bsi, 1) )  
    {
        GetBits(bsi, 3);// matrix downmix + pseudo_surround
    }
    GetBits(bsi, nNumFront * (1+4));
    GetBits(bsi, nNumSide * (1+4));
    GetBits(bsi, nNumBack * (1+4));
    GetBits(bsi, nNumLife * 4);
    GetBits(bsi, nNumAssocData * 4);
    GetBits(bsi, i_num_valid_cc * 5);
    ByteAlignBitstream( bsi);
    i_comment = GetBits(bsi, 8);
    GetBits(bsi, i_comment*8);

    return 0;
}

int Mpeg4GASpecificConfig( mpeg4_cfg_t *p_cfg, BitStreamInfo *bsi )
{
    int nExtensionFlag;
    p_cfg->nFrameLength = GetBits(bsi, 1)? 960 : 1024; 

    if (GetBits(bsi, 1))     // depend on core coder
    {
        GetBits(bsi, 14);   // core coder delay
    }
    nExtensionFlag = GetBits(bsi, 1);
    if (p_cfg->nChannel == 0)
    {
        Mpeg4GAProgramConfigElement( bsi );
    }
    if (p_cfg->nObjectType == 6 || p_cfg->nObjectType == 20)
    {
        GetBits(bsi, 3); // layer
    }
    if (nExtensionFlag)
    {
        if (p_cfg->nObjectType == 22)
        {
            GetBits(bsi, 5+11);   // numOfSubFrame + layer length
        }
        if (p_cfg->nObjectType == 17 || p_cfg->nObjectType == 19 ||
            p_cfg->nObjectType == 20 || p_cfg->nObjectType == 23)
        {
            GetBits(bsi, 1+1+1);   // ER data : section scale spectral */
        }
        if (GetBits(bsi, 1))     // extension 3
        {
            return ERR_AAC_LATM_PARSING_ERROR;
        }
    }
    return 0;
}

int Mpeg4ReadAudioObjectType( BitStreamInfo *bsi )
{
    int i_type;
    i_type = GetBits(bsi, 5);
    if ( i_type == 31)
    {
        i_type = 32 + GetBits(bsi, 6);
    }
    return i_type;
}

int Mpeg4ReadAudioSamplerate(BitStreamInfo *bsi,AACDecInfo *aacDecInfo, int updateIdx)
{
    int i_index;
    PSInfoBase *psi;
    psi = (PSInfoBase *)(aacDecInfo->psInfoBase);
    i_index = GetBits(bsi, 4);

    if (updateIdx) 
    {
        psi->sampRateIdx = i_index;
    }
    if (i_index >13 || i_index<0 )
    {
        return ERR_AAC_LATM_PARSING_ERROR;
    }
    if ( i_index != 0x0f )
    {
        return Sample_rates[i_index];
    }
    return GetBits(bsi, 24);
}

int Mpeg4ReadAudioSpecificInfo( mpeg4_cfg_t *p_cfg, int *pi_extra, uint8_t *p_extra, BitStreamInfo *bsi, int i_max_size,AACDecInfo *aacDecInfo )
{
#if 0
    static const char *ppsz_otype[] = {
        "NULL",
        "AAC Main", "AAC LC", "AAC SSR", "AAC LTP", "SBR", "AAC Scalable",
        "TwinVQ",
        "CELP", "HVXC",
        "Reserved", "Reserved",
        "TTSI",
        "Main Synthetic", "Wavetables Synthesis", "General MIDI",
        "Algorithmic Synthesis and Audio FX",
        "ER AAC LC",
        "Reserved",
        "ER AAC LTP", "ER AAC Scalable", "ER TwinVQ", "ER BSAC", "ER AAC LD",
        "ER CELP", "ER HVXC", "ER HILN", "ER Parametric",
        "SSC",
        "PS", "Reserved", "Escape",
        "Layer 1", "Layer 2", "Layer 3",
        "DST",
    };
#endif
    
    const int i_pos_start = CalcBitsUsed(bsi, gpBuffer, 0);   
    BitStreamInfo tBsiSave = *bsi;
    int i_bits;
    int i;
    int nResult;   
    memset( p_cfg, 0, sizeof(*p_cfg) );
    *pi_extra = 0;
    p_cfg->nObjectType = Mpeg4ReadAudioObjectType( bsi );
    p_cfg->nSampleRate = Mpeg4ReadAudioSamplerate( bsi ,aacDecInfo,1);
    if (p_cfg->nSampleRate >96000 || p_cfg->nSampleRate<0)
    {
        printf("[AAC] latm parsing sampling rate error %d \n",p_cfg->nSampleRate);
        return ERR_UNKNOWN;
    }
    aacDecInfo->sampRate = p_cfg->nSampleRate;
    p_cfg->nChannel = GetBits(bsi, 4);//bs_read( s, 4 );
    if (p_cfg->nChannel == 7)
    {
        p_cfg->nChannel = 8; // 7.1
    }
    else if (p_cfg->nChannel >= 8)
    {
        p_cfg->nChannel = -1;
    }
    aacDecInfo->nChans = p_cfg->nChannel;
    if (aacDecInfo->nChans >AAC_MAX_NCHANS || aacDecInfo->nChans<0)
    {
        printf("[AAC] latm parsing channel error %d \n",p_cfg->nChannel);
        return ERR_UNKNOWN;
    }        
    p_cfg->nSBR = -1;
    p_cfg->nPs  = -1;
    p_cfg->extension.nObjectType = 0;
    p_cfg->extension.nSampleRate = 0;
    if( p_cfg->nObjectType == 5 || p_cfg->nObjectType == 29 )
    {
        p_cfg->nSBR = 1;
        if( p_cfg->nObjectType == 29 )
        {
           p_cfg->nPs = 1;
        }
        p_cfg->extension.nObjectType = 5;
        p_cfg->extension.nSampleRate = Mpeg4ReadAudioSamplerate( bsi ,aacDecInfo,0);
        if (p_cfg->extension.nSampleRate >96000 || p_cfg->extension.nSampleRate<0)
        {
            printf("[AAC] latm parsing extension sampling rate error %d \n",p_cfg->nSampleRate);
            return ERR_UNKNOWN;
        }        
        p_cfg->nObjectType = Mpeg4ReadAudioObjectType( bsi );
    }

    switch( p_cfg->nObjectType )
    {
        case 1: case 2: case 3: case 4:
        case 6: case 7:
        case 17: case 19: case 20: case 21: case 22: case 23:
            nResult = Mpeg4GASpecificConfig( p_cfg, bsi );
            if (nResult)
            {
                printf("[AAC] latm pMpeg4GASpecificConfig error\n");            
                return ERR_UNKNOWN;
            }
            break;
        case 8:
            //printf("[AAC] CelpSpecificConfig\n");
            // CelpSpecificConfig();
            break;
        case 9:
            //printf("[AAC] HvxcSpecificConfig\n");        
            // HvxcSpecificConfig();
            break;
        case 12:
            //printf("[AAC] TTSSSpecificConfig\n");                    
            // TTSSSpecificConfig();
            break;
        case 13: case 14: case 15: case 16:
            // StructuredAudioSpecificConfig();
            break;
        case 24:
            // ERCelpSpecificConfig();
            break;
        case 25:
            // ERHvxcSpecificConfig();
            break;
        case 26: case 27:
            // ParametricSpecificConfig();
            break;
        case 28:
            // SSCSpecificConfig();
            break;
        case 32: case 33: case 34:
            // MPEG_1_2_SpecificConfig();
            break;
        case 35:
            // DSTSpecificConfig();
            break;
        case 36:
            // ALSSpecificConfig();
            break;
        default:
            // error
            printf("[AAC] Mpeg4ReadAudioSpecificInfo error\n");        
            return ERR_UNKNOWN;            
            break;
    }
    switch( p_cfg->nObjectType )
    {
        case 17: case 19: case 20: case 21: case 22: case 23:
        case 24: case 25: case 26: case 27:
        {
            int epConfig = GetBits(bsi, 2);//bs_read( s, 2 );
            if( epConfig == 2 || epConfig == 3 )
            {
                //ErrorProtectionSpecificConfig();
            }
            if( epConfig == 3 )
            {
                int directMapping = GetBits(bsi, 1);//bs_read1( s );
                if( directMapping )
                {
                    // tbd ...
                }
            }
            break;
        }
        default:
            break;
    }

    if( p_cfg->extension.nObjectType != 5 && i_max_size > 0 && i_max_size - (CalcBitsUsed(bsi, gpBuffer, 0) - i_pos_start) >= 16 &&
        GetBits(bsi, 11) == 0x2b7 ) //bs_read( s, 11 )
    {
        p_cfg->extension.nObjectType = Mpeg4ReadAudioObjectType( bsi );
        if( p_cfg->extension.nObjectType == 5 )
        {
            p_cfg->nSBR  = GetBits(bsi, 1);//bs_read1( s );
            if( p_cfg->nSBR == 1 )
            {
                p_cfg->extension.nSampleRate = Mpeg4ReadAudioSamplerate( bsi ,aacDecInfo,0);
                if( i_max_size > 0 && i_max_size - (CalcBitsUsed(bsi, gpBuffer, 0) - i_pos_start) >= 12 && GetBits(bsi, 11) == 0x548 )//bs_read( s, 11 )
                {
                    p_cfg->nPs = GetBits(bsi, 1);//bs_read1( s );
                }
            }
        }
    }

    i_bits = CalcBitsUsed(bsi, gpBuffer, 0) - i_pos_start;

    *pi_extra = MIN( ( i_bits + 7 ) / 8, LATM_MAX_EXTRA_SIZE );
    for( i = 0; i < *pi_extra; i++ )
    {
        const int i_read = MIN( 8, i_bits - 8*i );
        p_extra[i] = GetBits(&tBsiSave, i_read) << (8-i_read);  //bs_read( &s_sav, i_read ) << (8-i_read);
    }
    return i_bits;
}

int LatmGetValue( BitStreamInfo *bsi )
{
    int i_bytes = GetBits(bsi, 2);//bs_read( s, 2 );
    int v = 0;
    int i;
    for( i = 0; i < i_bytes; i++ )
    {
        v = (v << 8) + GetBits(bsi, 8);//bs_read( s, 8 );
    }
    return v;
}

int LatmReadStreamMuxConfiguration( latm_mux_t *m, BitStreamInfo *bsi,AACDecInfo *aacDecInfo )
{
    int nMuxVersion;
    int nMuxVersionA;
    int nProgram;
    int nTemp;
    nMuxVersion = GetBits(bsi, 1);
    nMuxVersionA = 0;
    if (!m || !bsi || !aacDecInfo)
    {
        return ERR_AAC_LATM_PARSING_ERROR;
    }
    if (nMuxVersion)
    {
        nMuxVersionA = GetBits(bsi, 1);
    }      
    if (nMuxVersionA != 0) /* support only A=0 */
    {
        return -1;
    }
    memset( m, 0, sizeof(*m) );
    if (nMuxVersionA == 0)
    {
        if (nMuxVersion == 1)
        {
            LatmGetValue(bsi); /* taraBufferFullness */
        }
    }
    m->nSameTiemFraming = GetBits(bsi, 1);
    m->nSubFrames = 1 + GetBits(bsi, 6);
    m->nPrograms = 1 + GetBits(bsi, 4);
    for( nProgram = 0; nProgram < m->nPrograms; nProgram++ )
    {
        int nLayer;
        m->tLayers[nProgram] = 1+GetBits(bsi, 3);
        for( nLayer = 0; nLayer < m->tLayers[nProgram]; nLayer++ )
        {
            latm_stream_t *st = &m->stream[m->nStreams];
            uint8_t b_previous_cfg;
            m->tStream[nProgram][nLayer] = m->nStreams;
            st->nProgram = nProgram;
            st->nLayer = nLayer;
            b_previous_cfg = 0;
            if( nProgram != 0 || nLayer != 0 )
            {
                b_previous_cfg = GetBits(bsi, 1);
            }
            if( b_previous_cfg )
            {
                st->cfg = m->stream[m->nStreams-1].cfg;
            }
            else
            {
                int i_cfg_size = 0;
                if( nMuxVersion == 1 )
                {
                    i_cfg_size = LatmGetValue(bsi);
                }
                nTemp =Mpeg4ReadAudioSpecificInfo( &st->cfg, &st->nExtra, st->extra, bsi, i_cfg_size,aacDecInfo );
                if (nTemp==ERR_UNKNOWN)
                {
                    return ERR_AAC_LATM_PARSING_ERROR;
                }
                i_cfg_size -= nTemp;
                if( i_cfg_size > 0 )
                {
                    printf("[AAC] latm i_cfg_size %d \n",i_cfg_size);
                    if (i_cfg_size<300)
                    {
                        GetBits(bsi, i_cfg_size);//bs_skip( s, i_cfg_size );
                    }
                    else
                    {
                        return ERR_AAC_LATM_PARSING_ERROR;
                    }
                }
            }
            st->nFrameLengthType = GetBits(bsi, 3);//bs_read( s, 3 );
            switch( st->nFrameLengthType )
            {
                case 0:
                {
                    GetBits(bsi, 8); /* latmBufferFullnes */
                    if( !m->nSameTiemFraming )
                    {
                        if( st->cfg.nObjectType == 6 || st->cfg.nObjectType == 20 ||
                            st->cfg.nObjectType == 8 || st->cfg.nObjectType == 24 )
                        {
                            GetBits(bsi, 6);/* eFrameOffset */
                        }
                    }
                    break;
                }
                case 1:
                    st->nFrameLength = GetBits(bsi, 9);
                    break;
                case 3: case 4: case 5:
                    st->nFrameLengthIndex = GetBits(bsi, 6); // celp
                    //printf("[aac] latm celp index %d \n",st->nFrameLengthIndex);
                    break;
                case 6: case 7:
                    st->nFrameLengthIndex = GetBits(bsi, 1); // hvxc
                    //printf("[aac] latm hvxc index %d \n",st->nFrameLengthIndex);
                default:
                    break;
            }
            /* Next stream */
            m->nStreams++;
        }
    }

    /* other data */
    if( GetBits(bsi, 1) ) //bs_read1( s )
    {
        if( nMuxVersion == 1 )
        {
            m->nOtherData = LatmGetValue( bsi );
        }
        else
        {
            int b_continue;
            nTemp =0;
            do
            {
                b_continue = GetBits(bsi, 1);//bs_read1(s);
                m->nOtherData = (m->nOtherData << 8) + GetBits(bsi, 8);  //bs_read( s, 8 );
                nTemp++;
                if (nTemp>200)
                {
                    printf("[AAC] Aac_latm LatmReadStreamMuxConfiguration loop %d \n",nTemp);
                }
            } while( b_continue );
        }
    }

    /* crc */
    m->nCrc = -1;
    if( GetBits(bsi, 1) )  //bs_read1(s)
    {
        m->nCrc = GetBits(bsi, 8);//bs_read( s, 8 );
    }
    return 0;
}

int LOASParse(HAACDecoder hAACDecoder, uint8_t *p_buffer, int i_buffer )
{
    AACDecInfo *aacDecInfo = (AACDecInfo *)hAACDecoder;
    //decoder_sys_t *p_sys = p_dec->p_sys;
    latm_stream_t *st;
    //bs_t s;
    int nSub;
    int i_accumulated = 0;
    int nResult;
    int nFrameLength;
#ifdef LATM_TO_ADTS
    char AdtsHead[7];
#endif
    uint8_t uUseSameMux;
    uint8_t buf;
    BitStreamInfo bsi;
    int nTemp;   

    gpBuffer = p_buffer;
    /* init bitstream reader */
    SetBitstreamPointer(&bsi, i_buffer, p_buffer);
    //GetBits(&bsi, *bitOffset);
    //bs_init( &s, p_buffer, i_buffer );
    /* Read the stream mux configuration if present */
    //uUseSameMux = bs_read(&s,1);
    uUseSameMux = GetBits(&bsi, 1);
    if (!uUseSameMux)
    {
        nResult = LatmReadStreamMuxConfiguration(&tLatm, &bsi,aacDecInfo );
        if (nResult)
        {
            return ERR_AAC_LATM_PARSING_ERROR;
        }
    }
    if (tLatm.nSubFrames == 0 && aacDecInfo->sampRate == 0)
    {
        return ERR_AAC_LATM_PARSING_ERROR;
    }      
    /* Wait for the configuration */
    if (b_latm_cfg)
    {
        return 0;
    }
    /* FIXME do we need to split the subframe into independent packet ? */
    if( tLatm.nSubFrames > 1 )
    {
     //  printf("latm sub frames not yet supported, please send a sample \n");
    }
    else
    {
       //tLatm.nSubFrames=1;
       //return 0;
    }

    for( nSub = 0; nSub < tLatm.nSubFrames; nSub++ )
    {
        int pi_payload[LATM_MAX_PROGRAM][LATM_MAX_LAYER];

        //Table 1.31 ¡V Syntax of PayloadLengthInfo
        if( tLatm.nSameTiemFraming )
        {
            int nProgram;
            /* Payload length */
            for( nProgram = 0; nProgram < tLatm.nPrograms; nProgram++ )
            {
                int nLayer;
                for( nLayer = 0; nLayer < tLatm.tLayers[nProgram]; nLayer++ )
                {
                    latm_stream_t *st = &tLatm.stream[tLatm.tStream[nProgram][nLayer]];
                    if( st->nFrameLengthType == 0 )
                    {
                        int i_payload = 0;
                        for( ;; )
                        {
                            int i_tmp = GetBits(&bsi, 8);//bs_read( &s, 8 );
                            i_payload += i_tmp;
                            if( i_tmp != 255 )
                            {
                                break;
                            }
                        }
                        pi_payload[nProgram][nLayer] = i_payload;
                    }
                    else if( st->nFrameLengthType == 1 )
                    {
                        pi_payload[nProgram][nLayer] = st->nFrameLength / 8; /* XXX not correct */
                    }
                    else if( ( st->nFrameLengthType == 3 ) || ( st->nFrameLengthType == 5 ) ||( st->nFrameLengthType == 7 ) )
                    {
                        GetBits(&bsi, 2);//bs_skip( &s, 2 ); // muxSlotLengthCoded
                        pi_payload[nProgram][nLayer] = 0; /* TODO */
                    }
                    else
                    {
                        pi_payload[nProgram][nLayer] = 0; /* TODO */
                    }
                }
            }
            /* Payload Data */
            //Table 1.32 ¡V Syntax of PayloadMux
            for( nProgram = 0; nProgram < tLatm.nPrograms; nProgram++ )
            {
                int nLayer;
                int i;
                for( nLayer = 0; nLayer < tLatm.tLayers[nProgram]; nLayer++ )
                {
                    /* XXX we only extract 1 stream */
                    if( nProgram != 0 || nLayer != 0 )
                    {
                        break;
                    }
                    if( pi_payload[nProgram][nLayer] <= 0 )
                    {
                        continue;
                    }
                #ifdef LATM_TO_ADTS
                    nFrameLength = pi_payload[nProgram][nLayer] + 7; 
                    AdtsHead[0] = 0xff; 
                    AdtsHead[1] = 0xf1; 
                    AdtsHead[2] = (0x01 <<6)|(0x06 <<2); //24000 sample rate 
                    AdtsHead[3] = 0x80|(nFrameLength>>11); 
                    AdtsHead[4] = ((nFrameLength&0x7FF)>>3)&0xff; 
                    AdtsHead[5] = ((nFrameLength&0x07) <<5|0x1f); 
                    AdtsHead[6] = 0xfc; 
                    // write adts header
                    if(GetInfoFile()!=NULL)
                    {
                        fwrite(&AdtsHead[0],1,7,GetInfoFile());
                    }
                    /* FIXME that's slow (and a bit ugly to write in place) */
                    for( i = 0; i < pi_payload[nProgram][nLayer]; i++ )
                    {
                        //p_buffer[i_accumulated++] = GetBits(&bsi, 8);//bs_read( &s, 8 );
                        //printf("0x%x \n",bs_read( &s, 8 ));
                        buf = GetBits(&bsi, 8);
                        fwrite(&buf,1,1,GetInfoFile());
                    } 
                #endif
                     i_accumulated = pi_payload[nProgram][nLayer];
                }
            }
            aacDecInfo->nPayloadSize = i_accumulated;
        }
        else
        {
            const int i_chunks = GetBits(&bsi, 4);
            int pi_program[16];
            int pi_layer[16];
            int i_chunk;
            // printf("latm without same time frameing not yet supported, please send a sample");            
            for( i_chunk = 0; i_chunk < i_chunks; i_chunk++ )
            {
                const int streamIndex = GetBits(&bsi, 4);
                latm_stream_t *st = &tLatm.stream[streamIndex];
                const int nProgram = st->nProgram;
                const int nLayer = st->nLayer;
                pi_program[i_chunk] = nProgram;
                pi_layer[i_chunk] = nLayer;
                if( st->nFrameLengthType == 0 )
                {
                    int i_payload = 0;
                    nTemp = 0;
                    for( ;; )
                    {
                        int i_tmp = GetBits(&bsi, 8);
                        i_payload += i_tmp;
                        nTemp++;
                        if ( i_tmp != 255)
                        {
                            break;
                        }
                        if (nTemp >100)
                        {
                            return ERR_AAC_LATM_PARSING_ERROR;
                        }
                    }
                    nTemp = 0;
                    pi_payload[nProgram][nLayer] = i_payload;
                    GetBits(&bsi, 1);// auEndFlag
                }
                else if ( st->nFrameLengthType == 1 )
                {
                    pi_payload[nProgram][nLayer] = st->nFrameLength / 8; /* XXX not correct */
                }
                else if ( ( st->nFrameLengthType == 3 ) || ( st->nFrameLengthType == 5 ) || ( st->nFrameLengthType == 7 ) )
                {
                    GetBits(&bsi, 2); // muxSlotLengthCoded
                }
                else
                {
                }
            }
            for( i_chunk = 0; i_chunk < i_chunks; i_chunk++ )
            {
                //const int nProgram = pi_program[i_chunk];
                //const int nLayer = pi_layer[i_chunk];

                /* TODO ? Payload */
            }
        }
    }

    if ( tLatm.nOtherData > 0 )
    {
        /* Other data XXX we just ignore them */
    }
    if (bsi.cachedBits==0 || bsi.cachedBits==8 || bsi.cachedBits==16 || bsi.cachedBits== 24)
    {
      aacDecInfo->nBitOffsets = 0;
      aacDecInfo->nPayloadSize--;
    }
    else if (bsi.cachedBits > 24)
    {
       aacDecInfo->nBitOffsets = 32 -bsi.cachedBits;
    }
    else if (bsi.cachedBits > 16)
    {
       aacDecInfo->nBitOffsets = 24 -bsi.cachedBits;
    }
    else if (bsi.cachedBits > 8)
    {
       aacDecInfo->nBitOffsets = 16 -bsi.cachedBits;
    }
    else
    {
       aacDecInfo->nBitOffsets = 8 -bsi.cachedBits;
    }
    nResult = 0;
    return 0;
}

