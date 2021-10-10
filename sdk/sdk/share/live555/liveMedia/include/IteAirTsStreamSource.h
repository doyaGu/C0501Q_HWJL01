
#ifndef __ITE_AIR_TS_STREAM_H_Y1QLUWI5_O2O6_UENM_RKQ2_Y7FZTXPMO18C__
#define __ITE_AIR_TS_STREAM_H_Y1QLUWI5_O2O6_UENM_RKQ2_Y7FZTXPMO18C__

#if defined(CFG_DEMOD_ENABLE)

#ifndef _FRAMED_SOURCE_HH
    #include "FramedSource.hh"
#endif

#include "ts_airfile/ite_ts_airfile.h"
//=============================================================================
//                  Class Definition
//=============================================================================
class IteAirTsStreamSource: public FramedSource 
{
    public:
        static int 
            createNew_Probe(UsageEnvironment &env, int demod_idx);
            
        static IteAirTsStreamSource* 
            createNew(UsageEnvironment& env, int demod_idx, int service_idx, uint32_t preferredSampleSize); 

    protected:
        // called only by createNew()
        IteAirTsStreamSource(UsageEnvironment& env, 
                             int        demod_idx,
                             int        service_idx,
                             uint32_t   preferredSampleSize);
        
        virtual ~IteAirTsStreamSource();

        uint32_t        _preferredSampleSize;
        
    private:
        //===================================================
        // Private functions
        //===================================================  
        
        //===================================================
        // Redefined virtual functions
        //===================================================
        virtual void doGetNextFrame();

        //===================================================
        // Private Parameters
        //===================================================
        int             _fid; // posix file hanlde
        int             _demod_idex;
        int             _service_idex;
        TSAF_HANDLE     *_pHTsaf;
        
        unsigned        _preferredFrameSize;
        unsigned        _playTimePerFrame;
        unsigned        _lastPlayTime;
        Boolean         _bLimitNumBytesToStream;
        u_int64_t       _numBytesToStream; // used iff "fLimitNumBytesToStream" is True
};

#endif

#endif
