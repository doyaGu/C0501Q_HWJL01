
#ifndef __ITE_EVENT_SERVER_MEDIA_SUBSESSION_H
#define __ITE_EVENT_SERVER_MEDIA_SUBSESSION_H

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
    #include "OnDemandServerMediaSubsession.hh"
#endif


#ifndef _FRAMED_SOURCE_HH
    #include "FramedSource.hh"
#endif

#ifndef _MULTI_FRAMED_RTP_SINK_HH
    #include "MultiFramedRTPSink.hh"
#endif

#define ITE_METADATA_FILE_EXTENSION     ".meta"
//=============================================================================
//                  Class Definition
//=============================================================================
class IteEventSource: public FramedSource
{
    public:
        static IteEventSource *createNew(UsageEnvironment &env,
                                         unsigned demodId,
                                         unsigned int duration);
        virtual void doGetNextFrame();

    protected:

        static void retryLater(void *firstArg);
        IteEventSource(UsageEnvironment &env, unsigned demodId, unsigned int duration); // abstract base class
        virtual ~IteEventSource();

    private:
        unsigned      fDemodId;
        unsigned int  fDuration;
};

class ApplicationRTPSink: public MultiFramedRTPSink
{
    public:
        static ApplicationRTPSink *createNew(UsageEnvironment &env, Groupsock *RTPgs, unsigned char rtpPayloadType);

    protected:
        ApplicationRTPSink(UsageEnvironment &env,
                           Groupsock *rtpgs, unsigned char rtpPayloadType);
        // (we're an abstract base class)
        virtual ~ApplicationRTPSink();
        virtual void doSpecialFrameHandling(unsigned fragmentationOffset,
                                            unsigned char *frameStart,
                                            unsigned numBytesInFrame,
                                            struct timeval framePresentationTime,
                                            unsigned numRemainingBytes);

    private: // redefined virtual functions:
        virtual char const *sdpMediaType() const;
};

class IteEventServerMediaSubsession: public OnDemandServerMediaSubsession
{
    public:
        static IteEventServerMediaSubsession *
        createNew(UsageEnvironment &env, char const *dataFileName);

    protected:
        ~IteEventServerMediaSubsession();

    private:
        //===================================================
        // Private functions
        //===================================================
        IteEventServerMediaSubsession(UsageEnvironment &env,
                                      unsigned demod_idx,
                                      unsigned service_idx);

        // The virtual functions thare are usually implemented by "ServerMediaSubsession"s:
        virtual FramedSource *createNewStreamSource(unsigned clientSessionId,
                unsigned &estBitrate);

        virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock,
                                          unsigned char rtpPayloadTypeIfDynamic,
                                          FramedSource *inputSource);
        //===================================================
        // Private Parameters
        //===================================================
        unsigned                  _demod_idx;
        unsigned                  _service_idx;
};

#endif
