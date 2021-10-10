/**********
Wifi Display Session
Created by powei
**********/


#include "liveMedia.hh"
#include "Locale.hh"
#include "GroupsockHelper.hh"
#include "WFDSinkSession.hh"
#include <ctype.h>

////////// WFDSession //////////

WFDSinkSession* 
WFDSinkSession::createNew(UsageEnvironment& env) {
  WFDSinkSession* newSession = new WFDSinkSession(env);
  if (newSession != NULL) {
    if (!newSession->initialize()) {
      delete newSession;
      return NULL;
    }
  }

  return newSession;
}

WFDSinkSession::WFDSinkSession(UsageEnvironment& env)
  : MediaSession(env) {
}

WFDSinkSession::~WFDSinkSession() {
  delete fSubsessionsHead;
}

static Boolean parseSourceFilter(char const* sourceUrl,
					  struct in_addr& sourceAddr) {
  Boolean result = False; // until we succeed
  char* sourceName = strDupSize(sourceUrl); // ensures we have enough space
  do {
    if (sscanf(sourceUrl, "%[^/]", sourceName) != 1) break;	       

    // Now, convert this name to an address, if we can:
    NetAddressList addresses(sourceName);
    if (addresses.numAddresses() == 0) break;

    netAddressBits sourceAddrBits
      = *(netAddressBits*)(addresses.firstAddress()->data());
    if (sourceAddrBits == 0) break;

    sourceAddr.s_addr = sourceAddrBits;
    result = True;
  } while (0);
  delete[] sourceName;
  return result;
}

Boolean WFDSinkSession::initialize() {
  MediaSubsession* subsession = createNewMediaSubsession();
  if (subsession == NULL) {
    envir().setResultMsg("Unable to create new MediaSubsession");
    return False;
  }

  subsession->fMediumName = "WFD";
  subsession->fProtocolName = "RTP";
  subsession->fRTPPayloadFormat = 33; //MP2T
  subsession->fCodecName = "MP2T";
  subsession->fRTPTimestampFrequency = 90000;
  subsession->fNumChannels = 1;
  subsession->fControlPath = strDup("streamid=0");
  // Insert this subsession at the end of the list:
  if (fSubsessionsTail == NULL) {
    fSubsessionsHead = fSubsessionsTail = subsession;
  } else {
    fSubsessionsTail->setNext(subsession);
    fSubsessionsTail = subsession;
  }

  return True;
}

Boolean WFDSinkSession::setWFDSession(unsigned short clientPortNum, char* sourceUrl, 
					 unsigned short width, unsigned short height, unsigned rate) {
  MediaSubsession *subsession = fSubsessionsHead;
  if(clientPortNum!=0) {
    subsession->fClientPortNum = clientPortNum;
    subsession->serverPortNum = clientPortNum; // by default
  }
  subsession->fVideoFPS = rate;
  subsession->fVideoWidth = width;
  subsession->fVideoHeight = height;
  parseSourceFilter(sourceUrl, subsession->fSourceFilterAddr);
  subsession->fConnectionEndpointName = strDup(sourceUrl);
  return True;
}
