/**********
 Wifi Display Session
 created by powei
*/

#ifndef _WFD_SINK_SESSION_HH
#define _WFD_SINK_SESSION_HH

#ifndef _MEDIA_SESSION_HH
#include "MediaSession.hh"
#endif

class WFDSinkSession: public MediaSession {
public:
  friend class WifiDisplaySink;
  static WFDSinkSession* createNew(UsageEnvironment& env);

protected:

  WFDSinkSession(UsageEnvironment& env);
  virtual ~WFDSinkSession();

  Boolean initialize();
  Boolean setWFDSession(unsigned short clientPortNum, char* sourceUrl,
					 unsigned short width, unsigned short height, unsigned rate);
private:

};

#endif
