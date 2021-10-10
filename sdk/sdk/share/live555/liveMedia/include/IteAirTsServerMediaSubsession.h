
#ifndef __ITE_AIR_TS_SERVER_MEDIA_SUBSESSION_H_AMN9M0UG_3IYN_DS9F_Q3PX_TN5VHUUQ6S7U__
#define __ITE_AIR_TS_SERVER_MEDIA_SUBSESSION_H_AMN9M0UG_3IYN_DS9F_Q3PX_TN5VHUUQ6S7U__

#if defined(CFG_DEMOD_ENABLE)

#include "IteAirTsStreamSource.h"

#ifndef _FILE_SERVER_MEDIA_SUBSESSION_HH
    #include "FileServerMediaSubsession.hh"
#endif

#ifndef _MPEG2_TRANSPORT_STREAM_FRAMER_HH
    #include "MPEG2TransportStreamFramer.hh"
#endif

#define ITE_AIR_TS_FILE_EXTENSION   ".airts"
#define ITE_AIR_TS_SAMPLE_SIZE      (1316) // MultiFramedRTPSink.cpp (Ln.48)
//=============================================================================
//                  Class Definition
//=============================================================================
class IteAirTsServerMediaSubsession: public FileServerMediaSubsession
{
    public:
        static IteAirTsServerMediaSubsession*
               createNew(UsageEnvironment& env, char const* dataFileName);

    protected:           
        ~IteAirTsServerMediaSubsession();

        void searchChannel(uint32_t freq, uint32_t bandwidth);
        
    private:
        //===================================================
        // Private functions
        //===================================================    
        IteAirTsServerMediaSubsession(UsageEnvironment& env, 
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
        IteAirTsStreamSource    *_iteAirSource;
        unsigned                _inputDataSize;

};
#endif

#endif
