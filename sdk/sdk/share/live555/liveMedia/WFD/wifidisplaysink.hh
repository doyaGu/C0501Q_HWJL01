/*
Wifi display sink designed by powei
*/

#ifndef _WIFI_DISPLAY_SINK_HH
#define _WIFI_DISPLAY_SINK_HH

#ifndef _RTSP_CLIENT_HH
#include "RTSPClient.hh"
#endif
#ifndef _WFD_SINK_SESSION_HH
#include "WFDSinkSession.hh"
#endif
#ifndef _TS_FILE_SINK_HH
#include "TSFileSink.hh"
#endif

class WifiDisplaySink: public RTSPClient {
public:

  static WifiDisplaySink* createNew(UsageEnvironment& env, char const* url,
				  int verbosityLevel,
				  char const* applicationName,
				  Authenticator* authenticator);

  TSFileSink*&  tsOut() { return fTSOut; }
  unsigned&  state() { return fState; }
  WFDSinkSession*&  session() { return fSession; }
  void sendOptions();
  void sendSetup();
  void sendPlay();
  void sendTeardown();

protected:
  WifiDisplaySink(UsageEnvironment& env, char const* url,
	              int verbosityLevel, char const* applicationName, 
				  Authenticator* authenticator);
  // (we're an abstract base class)
  virtual ~WifiDisplaySink();
  virtual void handleIncomingRequest();

private: // redefined virtual functions:

  void onOptionsRequest(char const* cseq);
  void onGetParameterRequest(char const* cseq);
  void onSetParameterRequest(char const* cseq);
  void onNotSupportRequest(char const* cseq);
  Boolean parseSetParemeter(char* buf);
  void setExtraHeaderString(char const* extraString);  

  Authenticator* fAuthenticator;
  unsigned int fDisplayWidth;
  unsigned int fDisplayHeight;
  unsigned int fFrameRate;
  unsigned int fInterlace;
  unsigned fAudioCodec;
  unsigned int fSampleFrequency;
  unsigned int fBitWidth;
  unsigned int fChannels;
  unsigned fProtection;
  char* fWFDUrl0;
  char* fWFDUrl1;
  unsigned fTrigger;
  unsigned int fRTPPort0;
  unsigned int fRTPPort1;
  WFDSinkSession* fSession;
  TSFileSink* fTSOut;
  unsigned fState;
};

#endif
