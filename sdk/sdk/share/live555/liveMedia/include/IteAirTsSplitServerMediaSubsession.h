
#ifndef __ITE_AIR_TS_SPLIT_SERVER_MEDIA_SUBSESSION_H_AMN9M0UG_3IYN_DS9F_Q3PX_TN5VHUUQ6S7U__
#define __ITE_AIR_TS_SPLIT_SERVER_MEDIA_SUBSESSION_H_AMN9M0UG_3IYN_DS9F_Q3PX_TN5VHUUQ6S7U__

#ifndef _FILE_SERVER_MEDIA_SUBSESSION_HH
    #include "FileServerMediaSubsession.hh"
#endif

#ifndef _H264_VIDEO_STREAM_FRAMER_HH
    #include "H264VideoStreamFramer.hh"
#endif

#ifndef _ITE_PES_SOURCE_HH
    #include "ItePesSource.h"
#endif

#define ITE_AIR_TS_FILE_EXTENSION   ".airts"

//=============================================================================
//                  Class Definition
//=============================================================================
class IteAirTsSplitServerMediaSubsession: public FileServerMediaSubsession
{
    public:
        static IteAirTsSplitServerMediaSubsession*
               createNew(UsageEnvironment& env, char const* dataFileName);

    protected:           
        ~IteAirTsSplitServerMediaSubsession();

        void searchChannel(uint32_t freq, uint32_t bandwidth);
        
    private:
        //===================================================
        // Private functions
        //===================================================    
        IteAirTsSplitServerMediaSubsession(UsageEnvironment& env, 
                                      char const* fileName,
                                      int         demod_idx,
                                      int         service_id);

        //===================================================
        // Redefined virtual functions
        //===================================================
        /**
         * Note that because - to implement 'trick play' operations - we're operating on
         * more than just the input source, we reimplement some functions that are
         * already implemented in "OnDemandServerMediaSubsession", rather than
         * reimplementing "seekStreamSource()" and "setStreamSourceScale()":
         **/
        virtual void startStream(unsigned clientSessionId, void* streamToken,
                                 TaskFunc* rtcpRRHandler,
                                 void* rtcpRRHandlerClientData,
                                 unsigned short& rtpSeqNum,
                                 unsigned& rtpTimestamp,
                                 ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                                 void* serverRequestAlternativeByteHandlerClientData);                                      
        // The virtual functions thare are usually implemented by "ServerMediaSubsession"s:
        virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
                                                    unsigned& estBitrate);

        virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                          unsigned char rtpPayloadTypeIfDynamic,
                                          FramedSource* inputSource);

        virtual void testScaleFactor(float& scale);
        virtual void seekStream(unsigned clientSessionId, void* streamToken, double& seekNPT,
                                double streamDuration, u_int64_t& numBytes);
        //===================================================
        // Private Parameters
        //===================================================
        bool                    _bDemod_ready;
        int                     _service_idx;
        int                     _demod_idx;
        ItePesSource            *_iteAirSource;
};

#endif
