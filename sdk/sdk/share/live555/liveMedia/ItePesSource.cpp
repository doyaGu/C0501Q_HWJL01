/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2012 Live Networks, Inc.  All rights reserved.
// Framed File Sources
// Implementation
//#include "ite/audio.h"
#include "ite/itp.h"
#include "ite/audio.h"
#include "ite/ite_risc_ts_demux.h"
#include "ItePesSource.h"

////////// ItePesSource //////////
ItePesSource*
ItePesSource::createNew(UsageEnvironment& env, uint32_t demodId, uint32_t serviceId)
{
    ItePesSource* newSource = new ItePesSource(env, demodId, serviceId);
    return newSource;
}


ItePesSource::ItePesSource(UsageEnvironment& env, uint32_t demodId, uint32_t serviceId)
  : FramedSource(env), fDemodId(demodId), fServiceId(serviceId), fpOutputPtr(NULL), fBufferLen(0),
    fpStartPtr(NULL), fRemainLen(0), fState(0), fbFirstTime(1), fPrevTime(0) , fDuration(0)
{
    uint8*  pFrameStartAddr = 0;
    uint32  frameSize = 0;
}

ItePesSource::~ItePesSource()
{
}

void ItePesSource::doGetNextFrame()
{
    uint8_t* pCurPtr = fpOutputPtr;
    uint8_t* pVideoStartCodePtr = 0;
    uint32_t state = 0;
    uint32_t pesHeaderLen = 0;
    uint64_t time = 0;
    uint32_t bit32 = 0;
    uint32_t bit31_30 = 0;
    uint32_t bit29_15 = 0;
    uint32_t bit14_0 = 0;
    uint8_t nalType = 0;
    uint8*  pFrameStartAddr = 0;
    uint32  frameSize = 0;
    uint32  gap = 0;
    uint32_t duration_time = 0;
    uint32_t duration_time1 = 0;
    struct timeval curTime = { 0 };

#if !(_MSC_VER)
    if (fbFirstTime)
    {
        while (iteTsDemux_GetNextFrame(fDemodId, fServiceId, &pFrameStartAddr, &frameSize))
        {
            iteTsDemux_ReleaseFrame(fDemodId, fServiceId, pFrameStartAddr);
        }

        fbFirstTime = 0;
        fStartT.tv_sec = 0;
    }
    if (0 == fRemainLen)
    {
        if (iteTsDemux_GetNextFrame(fDemodId, fServiceId, &pFrameStartAddr, &frameSize))
        {
            fState = PES_FIND_START_CODE_STATE;
            pCurPtr = pFrameStartAddr;
            fRemainLen = frameSize;
            // iteTsDemux_ReleaseFrame(fDemodId, fServiceId, pFrameStartAddr);
        }
        else
        {
            // Delay this amount of time:
            nextTask() = envir().taskScheduler().scheduleDelayedTask(500, (TaskFunc*)retryLater, this);
            return;
        }
        if (fStartT.tv_sec)
        {
            gettimeofday(&curTime, NULL);
            duration_time = (curTime.tv_sec - fStartT.tv_sec) * 1000;      // sec to ms
            duration_time += ((curTime.tv_usec - fStartT.tv_usec) / 1000); // us to ms
            //printf("duration: %u, size: %u\n", duration_time, frameSize);
        }
        gettimeofday(&fStartT, NULL);
    }

    while (fRemainLen > 0)
    {
        switch (fState)
        {
            case PES_FIND_START_CODE_STATE:
                if (pCurPtr[0] == 0x0 && pCurPtr[1] == 0x0 && pCurPtr[2] == 0x1 && pCurPtr[3] == 0xE0)
                {
                    fpOutputPtr = pCurPtr;
                    // Just overwrite output ptr to improve performance, therefore, the AP shouldn't prepare fTo buffer to prevent memory leak.
                    pCurPtr += 4;
                    fRemainLen -= 4;
                    fpParsePtr = fpOutputPtr;
                    fpOutputPtr = pCurPtr;
                    pesHeaderLen = fpParsePtr[8];
                    if ((fpParsePtr[7] & 0xC0))
                    {
                        bit32    = ((fpParsePtr[9] & 0x08) >> 3);
                        bit31_30 = ((fpParsePtr[9] & 0x06) >> 1);

                        bit29_15 = (uint32_t) ((fpParsePtr[10] << 8) | fpParsePtr[11]) >> 1;
                        bit14_0  = (uint32_t) ((fpParsePtr[12] << 8) | fpParsePtr[13]) >> 1;

                        time = (((uint64_t) bit32 << 32) | (bit31_30 << 30) | (bit29_15 << 15) | bit14_0);
                        // translate to ms
                        time = time / 90;
                        if (fPrevTime)
                        {
                            if (time >= fPrevTime)
                            {
                                fDuration = (int32) (time - fPrevTime) * 1000;
                                //fDuration = 0;
                            }
                            else
                            {
                                fDuration = 0;
                            }
                        }
                        if (fDuration > 100 * 1000)
                        {
                            fDuration = 0;
                        }
                        fPrevTime = time;
                        fPresentationTime.tv_sec = time / 1000;
                        fPresentationTime.tv_usec = (time % 1000) * 1000;
                        //printf("time: %u.%03u\n", fPresentationTime.tv_sec, fPresentationTime.tv_usec / 1000);
                    }
                    // Ignore PES header
                    fpParsePtr += (9 + pesHeaderLen);
                    fState = PES_FIND_NAL_BOUNDARY;
                    fBufferLen = (uint32_t) ((pFrameStartAddr + frameSize) - fpParsePtr);
                    break;
                }
                else
                {
                    //pCurPtr++;
                    //fRemainLen--;
                    printf("unexpected pes header, retry later\n");
                    fRemainLen = 0;
                    nextTask() = envir().taskScheduler().scheduleDelayedTask(500, (TaskFunc*)retryLater, this);
                    return;
                }
                break;
            case PES_FIND_NAL_BOUNDARY:
                pVideoStartCodePtr = fpParsePtr + 4;

                // Get Nal Type
                nalType = (fpParsePtr[4] & 0x1F);
                //printf("type: %u, 0x%X \n", nalType);
                if (nalType < 6)
                {
                    fMaxSize = fFrameSize = fBufferLen - 4;

                #if !defined(CFG_ENABLE_H264_SRC_SINK_SHARE_BUF)
                    // copy nal
                    memcpy(fTo, &fpParsePtr[4], fBufferLen - 4);
                #else
                    // must advanced 1 byte for multi-frames in H264VideoRTPSink.cpp
                    *fpTo = &fpParsePtr[3];
                #endif
                    fState = PES_FIND_START_CODE_STATE;
                    fRemainLen = 0;
                    fDurationInMicroseconds = fDuration; // 40000;
                    //printf("nal: %u, size: %u, ptr: 0x%X\n", nalType, fFrameSize, fpParsePtr[4]);

                    if (pFrameStartAddr)
                    {
                        iteTsDemux_ReleaseFrame(fDemodId, fServiceId, pFrameStartAddr);
                    }
                    FramedSource::afterGetting(this);
                    return;
                }
                else
                {
                    fBufferLen -= 4;
                    while (fBufferLen > 0)
                    {
                        if (pVideoStartCodePtr[0] == 0x0 && pVideoStartCodePtr[1] == 0x0 && pVideoStartCodePtr[2] == 0x00 && pVideoStartCodePtr[3] == 0x01)
                        {
                            // boundary founds
                            // copy nal
                            fFrameSize = fMaxSize = (uint32_t) (pVideoStartCodePtr - fpParsePtr - 4);
                            //printf("nal: %u, size: %u, ptr: 0x%X\n", nalType, fFrameSize, fpParsePtr[4]);
                        #if !defined(CFG_ENABLE_H264_SRC_SINK_SHARE_BUF)
                            memcpy(fTo, &fpParsePtr[4], fMaxSize);
                        #else
                            // must advanced 1 byte for multi-frames in H264VideoRTPSink.cpp
                            *fpTo = &fpParsePtr[3];
                        #endif
                            fpParsePtr = pVideoStartCodePtr;
                            fDurationInMicroseconds = 0;

                            iteTsDemux_ReleaseFrame(fDemodId, fServiceId, pFrameStartAddr);

                            FramedSource::afterGetting(this);
                            return;
                        }
                        else
                        {
                            pVideoStartCodePtr++;
                            fBufferLen--;
                        }
                    }
                    if (fBufferLen <= 0)
                    {
                        fRemainLen = 0;
                        fState = PES_FIND_START_CODE_STATE;

                        iteTsDemux_ReleaseFrame(fDemodId, fServiceId, pFrameStartAddr);
                        nextTask() = envir().taskScheduler().scheduleDelayedTask(500, (TaskFunc*)retryLater, this);
                        return;
                    }
                }
                break;
        }
    }
#endif
    return;
}

void ItePesSource::doStopGettingFrames()
{
}

uint32_t ItePesSource::getRemainLen()
{
    return fRemainLen;
}

Boolean ItePesSource::isH264VideoStreamFramer() const {
  return True;
}

void ItePesSource::retryLater(void* firstArg) {
    ItePesSource* ptPesSource = (ItePesSource*)firstArg;
    //printf("%s(%d)\n", __FILE__, __LINE__);
    ptPesSource->doGetNextFrame();
}
