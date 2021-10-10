/*
Wifi display sink designed by powei
*/

#include "rtspcommon.hh"
#include "WifiDisplaySink.hh"

enum {
    CODEC_UNDEFINED = 0,
    CODEC_LPCM,
    CODEC_AAC,
    CODEC_AC3,
};
enum {
    PROTECT_NONE = 0,
    PROTECT_HDCP20,
    PROTECT_HDCP21
};
enum {
    TRIGGER_NONE = 0,
    TRIGGER_SETUP,
    TRIGGER_PAUSE,
    TRIGGER_TEARDOWN,
    TRIGGER_PLAY
};

enum {
    STATE_UNDEFINED = 0,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_PAUSED,
    STATE_PLAYING,
};

static char const* allowedOptionsNames
= "org.wfa.wfd1.0, GET_PARAMETER, SET_PARAMETER";

static const unsigned int displayMode_CEA[32][4] =
{ {640, 480, 60, 1},
  {720, 480, 60, 1},
  {720, 480, 60, 0},
  {720, 576, 50, 1},
  {720, 576, 50, 0},
  {1280, 720, 30, 1},
  {1280, 720, 60, 1},
  {1920, 1080, 30, 1},
  {1920, 1080, 60, 1},
  {1920, 1080, 60, 0},
  {1280, 720, 25, 1},
  {1280, 720, 50, 1},
  {1920, 1080, 25, 1},
  {1920, 1080, 50, 1},
  {1920, 1080, 50, 0},
  {1280, 720, 24, 1},
  {1920, 1080, 24, 1},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

static const unsigned int displayMode_VESA[32][4] =
{ {800, 600, 30, 1},
  {800, 600, 60, 1},
  {1024, 768, 30, 1},
  {1024, 768, 60, 1},
  {1152, 864, 30, 1},
  {1152, 864, 60, 1},
  {1280, 768, 30, 1},
  {1280, 768, 60, 1},
  {1280, 800, 30, 1},
  {1280, 800, 60, 1},
  {1360, 768, 30, 1},
  {1360, 768, 60, 1},
  {1366, 768, 30, 1},
  {1366, 768, 60, 1},
  {1280, 1024, 30, 1},
  {1280, 1024, 60, 1},
  {1400, 1050, 30, 1},
  {1400, 1050, 60, 1},
  {1440, 900, 30, 1},
  {1440, 900, 60, 1},
  {1600, 900, 30, 1},
  {1600, 900, 60, 1},
  {1600, 1200, 30, 1},
  {1600, 1200, 60, 1},
  {1680, 1024, 30, 1},
  {1680, 1024, 60, 1},
  {1680, 1050, 30, 1},
  {1680, 1050, 60, 1},
  {1920, 1200, 30, 1},
  {1920, 1200, 60, 1},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

static const unsigned int displayMode_HH[32][4] =
{ {800, 480, 30, 1},
  {800, 480, 60, 1},
  {854, 480, 30, 1},
  {854, 480, 60, 1},
  {864, 480, 30, 1},
  {864, 480, 60, 1},
  {640, 360, 30, 1},
  {640, 360, 60, 1},
  {960, 540, 30, 1},
  {960, 540, 60, 1},
  {848, 480, 30, 1},
  {848, 480, 60, 1},
};

static const unsigned int audioMode_LPCM[2][3] =
{ {44100, 16, 2},
  {48000, 16, 2}
};

static const unsigned int audioMode_AAC[4][3] =
{ {48000, 16, 2},
  {48000, 16, 4},
  {48000, 16, 6},
  {48000, 16, 8}
};

static const unsigned int audioMode_AC3[4][3] =
{ {48000, 16, 2},
  {48000, 16, 4},
  {48000, 16, 6},
  {0, 0, 0}
};

static unsigned bit2int(unsigned int mask) {
	unsigned pos=0;
	while(!(mask&0x1)) {
	  pos++;
	  mask>>=1;
	}
	return pos;
}

static WifiDisplaySink* wfdSink;
static UsageEnvironment* wfdEnv;

WifiDisplaySink::WifiDisplaySink(UsageEnvironment& env,  char const* url,
				  int verbosityLevel,
				  char const* applicationName,
				  Authenticator* authenticator)
  : fAuthenticator(authenticator), fDisplayWidth(0), fDisplayHeight(0), fFrameRate(0), fInterlace(0),
    fAudioCodec(CODEC_UNDEFINED), fProtection(PROTECT_NONE), fSession(NULL), fState(STATE_UNDEFINED),
    RTSPClient(env, url, verbosityLevel, applicationName, 0) {
  openConnection();
}

WifiDisplaySink::~WifiDisplaySink() {
  WFDSinkSession::close(fSession);
  TSFileSink::close(fTSOut);
  delete fWFDUrl0;
}

WifiDisplaySink* WifiDisplaySink::createNew(UsageEnvironment& env,  char const* url,
				  int verbosityLevel,
				  char const* applicationName,
				  Authenticator* authenticator) {
  wfdSink = new WifiDisplaySink(env, url, verbosityLevel, applicationName, authenticator);
  wfdEnv = &env;
  return wfdSink;
}

void WifiDisplaySink::handleIncomingRequest() {
  // Parse the request string into command name and 'CSeq', then 'handle' the command (by responding that we don't support it):
  char cmdName[RTSP_PARAM_STRING_MAX];
  char urlPreSuffix[RTSP_PARAM_STRING_MAX];
  char urlSuffix[RTSP_PARAM_STRING_MAX];
  char cseq[RTSP_PARAM_STRING_MAX];
  unsigned contentLength;
  if (!parseRTSPRequestString(fResponseBuffer, fResponseBytesAlreadySeen,
			      cmdName, sizeof cmdName,
			      urlPreSuffix, sizeof urlPreSuffix,
			      urlSuffix, sizeof urlSuffix,
			      cseq, sizeof cseq,
			      contentLength)) {
    return;
  } else {
    if (strcmp(cmdName, "OPTIONS") == 0) {
	  onOptionsRequest(cseq);
	} else if (strcmp(cmdName, "GET_PARAMETER") == 0) {
	  onGetParameterRequest(cseq);
	} else if (strcmp(cmdName, "SET_PARAMETER") == 0) {
	  onSetParameterRequest(cseq);
    } else {
	  onNotSupportRequest(cseq);
	}
  }
}

void WifiDisplaySink::setExtraHeaderString(char const* extraString) {
  // Change the existing user agent header string:
  delete[] fUserAgentHeaderStr;
  fUserAgentHeaderStr = strDup(extraString);
  fUserAgentHeaderStrLen = strlen(fUserAgentHeaderStr);
}

// Step1: M1
void WifiDisplaySink::onOptionsRequest(char const* cseq) {
  snprintf((char*)fResponseBuffer, responseBufferSize,
	   "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sPublic: %s\r\n\r\n\0",
	   cseq, dateHeader(), allowedOptionsNames);
  send(fOutputSocketNum, (char const*)fResponseBuffer, strlen((char*)fResponseBuffer), 0);
  fState = STATE_CONNECTING;
  sendOptions();
}

// M2
static void onReceiveOptionsResponse(RTSPClient*, int resultCode, char* resultString) {
  if (resultCode != 0) {
    *wfdEnv << "Failed to Options: " << resultString << "\n";
  }
  delete[] resultString;
}

void WifiDisplaySink::sendOptions() {
  setBaseURL("*");
  setExtraHeaderString("Require: org.wfa.wfd1.0\r\n");
  sendOptionsCommand(onReceiveOptionsResponse, fAuthenticator);
}


// M3
void WifiDisplaySink::onGetParameterRequest(char const* cseq) {
  Boolean success = True;
  unsigned short portNum;

  if( fSession==NULL) {
    fSession = WFDSinkSession::createNew(envir());
  }
  if (fSession == NULL) {
    envir() << "Failed to create a MediaSession object from the SDP description: " << envir().getResultMsg() << "\n";
    success = False;
  } else if (!fSession->hasSubsessions()) {
    envir() << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
    success = False;
  }

      // Then, setup the "RTPSource"s for the session:
  if( success==True) {
	MediaSubsession *subsession;
	MediaSubsessionIterator* iter = new MediaSubsessionIterator(*fSession);
    if((subsession = iter->next()) != NULL) {
      if (!subsession->initiate(-1)) {
	    envir() << "Unable to create receiver for \"" << subsession->mediumName()
	      << "/" << subsession->codecName()
	      << "\" subsession: " << envir().getResultMsg() << "\n";
        success = False;
	  }
	  portNum = subsession->clientPortNum();
	  envir() << "Created receiver for \"" << subsession->mediumName()
	    << "/" << subsession->codecName()
	    << "\" subsession (client ports " << subsession->clientPortNum()
	    << "-" << subsession->clientPortNum()+1 << ")\n";


      if (subsession->rtpSource() != NULL) {
	      // Because we're saving the incoming data, rather than playing
	      // it in real time, allow an especially large time threshold
	      // (1 second) for reordering misordered incoming packets:
	      unsigned const thresh = 1000000; // 1 second
	      subsession->rtpSource()->setPacketReorderingThresholdTime(thresh);
	  }
	}
  }

  if(success==True) {
    char const* const bodyStr =
	  "wfd_video_formats: 00 00 01 01 00000001 00000000 00000000 00 0000 0000 00 none none\r\n"
      "wfd_audio_codecs: LPCM 00000003 00\r\n"
      "wfd_3d_video_formats: none\r\n"
      "wfd_content_protection: none\r\n"
      "wfd_display_edid: none\r\n"
      "wfd_coupled_sink: none\r\n"
      "wfd_client_rtp_ports: RTP/AVP/UDP;unicast %d 0 mode=play";
	char* body = new char[strlen(bodyStr)+10];
    snprintf(body, strlen(bodyStr)+10, bodyStr, portNum);
	int len = sizeof(fResponseBuffer);
    snprintf((char*)fResponseBuffer, responseBufferSize,
	   "RTSP/1.0 200 OK\r\nCSeq: %s\r\nContent-Type: text/parameters\r\nContent-Length: %d\r\n\r\n%s",
	   cseq, strlen(body), body);
    delete[] body;
  } else {
	snprintf((char*)fResponseBuffer, responseBufferSize,
	   "RTSP/1.0 406 Not Acceptable\r\nCSeq: %s\r\n\r\n",cseq);
  }
  send(fOutputSocketNum, (char const*)fResponseBuffer, strlen((char*)fResponseBuffer), 0);
}

// M3, M4, M5
void WifiDisplaySink::onSetParameterRequest(char const* cseq) {
  char const* const errorbody =
       "wfd_video_formats: 457\r\n"
       "wfd_audio_codecs: 415\r\n";

  fTrigger = TRIGGER_NONE;
  if (!parseSetParemeter((char*)fResponseBuffer)) {
    snprintf((char*)fResponseBuffer, responseBufferSize,
	   "RTSP/1.0 303 See Other\r\nCSeq: %s\r\nContent-Type: text/parameters\r\nContent-Length: %d\r\n\r\n%s",
	   cseq, strlen(errorbody), errorbody);

  } else {
    snprintf((char*)fResponseBuffer, responseBufferSize,
	   "RTSP/1.0 200 OK\r\nCSeq: %s\r\n\r\n",cseq);
  }
  send(fOutputSocketNum, (char const*)fResponseBuffer, strlen((char*)fResponseBuffer), 0);

  // Do not use the port num of setparameter
  fSession->setWFDSession(0, fWFDUrl0, fDisplayWidth, fDisplayHeight, fFrameRate);

  do {
	if (fTrigger==TRIGGER_SETUP && fState==STATE_CONNECTING) {
        fState = STATE_CONNECTED;
        sendSetup();
	} else if(fTrigger==TRIGGER_TEARDOWN && fState==STATE_PLAYING) {
      sendTeardown();
	}

  } while(0);

}

void goTeardown(void* clientData) {
  wfdSink->sendTeardown();
}
// M6
static void onReceiveSetupResponse(RTSPClient*, int resultCode, char* resultString) {
  delete[] resultString;
  // Create an "TSFileSink", to write to 'stdout':
  TSFileSink* tsOut = TSFileSink::createNew(*wfdEnv, *wfdSink->session(), "rtsp.ts",
				                100000,
				                False);
  if (tsOut == NULL) {
	*wfdEnv << "Failed to create TS file sink for stdout: " << wfdEnv->getResultMsg();
  }

  tsOut->startPlaying(goTeardown, NULL);

  wfdSink->tsOut() = tsOut;
  wfdSink->state() = STATE_PAUSED;
  wfdSink->sendPlay();
}

void WifiDisplaySink::sendSetup() {
  Boolean forceMulticastOnUnspecified = False;
  Boolean streamUsingTCP = False;
  MediaSubsessionIterator* iter = new MediaSubsessionIterator(*fSession);
  MediaSubsession* subsession = iter->next();
  if(subsession != NULL) {
    char *pos=strrchr(fWFDUrl0,'/');
	*pos = '\0';  // remove control "streamid=0"
    setBaseURL(fWFDUrl0);
    setExtraHeaderString("");
    sendSetupCommand(*subsession, onReceiveSetupResponse, False, streamUsingTCP, forceMulticastOnUnspecified, fAuthenticator);
  }
}

// M7
void onReceivePlayResponse(RTSPClient*, int resultCode, char* resultString) {
  if (resultCode != 0) {
    *wfdEnv << "Failed to start playing session: " << resultString << "\n";
  }
  delete[] resultString;

  *wfdEnv << "Started playing session\n";
  wfdSink->state() = STATE_PLAYING;
}

void WifiDisplaySink::sendPlay() {
  if(fSession != NULL) {
    sendPlayCommand(*fSession, onReceivePlayResponse, 0, 0, 0, fAuthenticator);
  }
}

// M8
void onReceiveTeardownResponse(RTSPClient*, int resultCode, char* resultString) {
  if (resultCode != 0) {
    *wfdEnv << "Failed to teardown session: " << resultString << "\n";
  }
  delete[] resultString;

  *wfdEnv << "Teardown session\n";
  Medium::close(wfdSink->tsOut());
  wfdSink->tsOut() = NULL;
  
  Medium::close(wfdSink->session()); 
  wfdSink->session() = NULL;
  wfdSink->state() = STATE_UNDEFINED;
}

void WifiDisplaySink::sendTeardown() {
  sendTeardownCommand(*fSession, onReceiveTeardownResponse, fAuthenticator);
}


void WifiDisplaySink::onNotSupportRequest(char const* cseq) {
      char tmpBuf[2*RTSP_PARAM_STRING_MAX];
      snprintf((char*)tmpBuf, sizeof tmpBuf,
               "RTSP/1.0 405 Method Not Allowed\r\nCSeq: %s\r\n\r\n", cseq);
      send(fOutputSocketNum, tmpBuf, strlen(tmpBuf), 0);
}

static char* getLine(char* startOfLine) {
  // returns the start of the next line, or NULL if none.  Note that this modifies the input string to add '\0' characters.
  for (char* ptr = startOfLine; *ptr != '\0'; ++ptr) {
    // Check for the end of line: \r\n (but also accept \r or \n by itself):
    if (*ptr == '\r' || *ptr == '\n') {
      // We found the end of the line
      if (*ptr == '\r') {
	    *ptr++ = '\0';
	    if (*ptr == '\n') ++ptr;
      } else {
        *ptr++ = '\0';
      }
      return ptr;
    }
  }
  return NULL;
}

static Boolean checkForName(char const* line, char const* name, unsigned nameLength, char const*& params) {
  if (_strncasecmp(line, name, nameLength) != 0) return False;

  // The line begins with the desired header name.  Trim off any whitespace, and return the header parameters:
  unsigned paramIndex = nameLength;
  while (line[paramIndex] != '\0' && (line[paramIndex] == ' ' || line[paramIndex] == '\t')) ++paramIndex;
  if (&line[paramIndex] == '\0') return False; // the header is assumed to be bad if it has no parameters

  params = &line[paramIndex];
  return True;
}

Boolean WifiDisplaySink::parseSetParemeter(char* buf) {
  char* dataCopy = strDup(buf);
  char* lineStart = dataCopy;
  char* nextLineStart = getLine(lineStart);
  fTrigger = TRIGGER_NONE;
  while (1) {
  	lineStart = nextLineStart;
	if (lineStart == NULL) break;
	nextLineStart = getLine(lineStart);
	//if (lineStart[0] == '\0') break; // this is a blank line
	char const* paramsStr;
	if (checkForName(lineStart, "wfd_video_formats:", 18, paramsStr)) {
      unsigned native, prefered, profile, level, latency, rate;
      unsigned index, select;
      unsigned int CEA, VESA, HH, minSliceSize, sliceEncParams;
      char s_maxH[8], s_maxV[8];
      if(sscanf(paramsStr, "%2x %2x %2x %2x %8x %8x %8x %2x %4x %4x %2x %s %s",
		&native, &prefered, &profile, &level, &CEA, &VESA, &HH, &latency, &minSliceSize, &sliceEncParams, &rate, s_maxH, s_maxV)==13) {
        index = native>>3;
		select = native&0x7;
		if(select==0) {
		  fDisplayWidth = displayMode_CEA[index][0];
		  fDisplayHeight = displayMode_CEA[index][1];
		  fFrameRate = displayMode_CEA[index][2];
          fInterlace = displayMode_CEA[index][3];
		} else if(select==1) {
		  fDisplayWidth = displayMode_VESA[index][0];
		  fDisplayHeight = displayMode_VESA[index][1];
		  fFrameRate = displayMode_VESA[index][2];
          fInterlace = displayMode_VESA[index][3];
		} else if(select==2) {
		  fDisplayWidth = displayMode_HH[index][0];
		  fDisplayHeight = displayMode_HH[index][1];
		  fFrameRate = displayMode_HH[index][2];
          fInterlace = displayMode_HH[index][3];
		}
	  }
	} else if(checkForName(lineStart, "wfd_audio_codecs:", 17, paramsStr)) {
	  char codec[5];
	  unsigned int audioMode;
	  unsigned int latency;
	  int idx;
	  if(sscanf(paramsStr, "%s %8x %2x", codec, &audioMode, &latency)==3) {
		if(strcmp(codec, "LPCM")==0) {
		  fAudioCodec = CODEC_LPCM;
		  idx = bit2int(audioMode&0x3);
		  if(idx>0) {
		    fSampleFrequency = audioMode_LPCM[idx][0];
			fBitWidth = audioMode_LPCM[idx][1];
            fChannels = audioMode_LPCM[idx][2];
	      }
		} else if(strcmp(codec, "AAC")==0) {
		  fAudioCodec = CODEC_AAC;
		  idx = bit2int(audioMode&0xf);
		  if(idx>0) {
			fSampleFrequency = audioMode_AAC[idx-1][0];
			fBitWidth = audioMode_AAC[idx][1];
            fChannels = audioMode_AAC[idx][2];
	      }
		} else if(strcmp(codec, "AC3")==0) {
	      fAudioCodec = CODEC_AC3;
		  idx = bit2int(audioMode&0x7);
		  if(idx>0) {
			fSampleFrequency = audioMode_AC3[idx-1][0];
			fBitWidth = audioMode_AC3[idx][1];
            fChannels = audioMode_AC3[idx][2];
		  }
		}
	  }
	} else if(checkForName(lineStart, "wfd_content_protection:", 23, paramsStr)) {
	  if(strcmp(paramsStr, "none")==0)
		  fProtection = PROTECT_NONE;
	  else if(strcmp(paramsStr, "HDCP2.0")==0)
		  fProtection = PROTECT_HDCP20;
	  else if(strcmp(paramsStr, "HDCP2.1")==0)
		  fProtection = PROTECT_HDCP21;
	} else if(checkForName(lineStart, "wfd_client_rtp_ports:", 21, paramsStr)) {
		sscanf(paramsStr, "RTP/AVP/UDP;unicast %d %d", &fRTPPort0, &fRTPPort1);
	} else if(checkForName(lineStart, "wfd_trigger_method:", 19, paramsStr)) {
		if(strcmp(paramsStr, "SETUP")==0) fTrigger = TRIGGER_SETUP;
		else if(strcmp(paramsStr, "PAUSE")==0) fTrigger = TRIGGER_PAUSE;
		else if(strcmp(paramsStr, "TEARDOWN")==0) fTrigger = TRIGGER_TEARDOWN;
		else if(strcmp(paramsStr, "PLAY")==0) fTrigger = TRIGGER_PLAY;
	} else if(checkForName(lineStart, "wfd_presentation_URL:", 21, paramsStr)) {
		fWFDUrl0 = new char[strlen(paramsStr)];
		sscanf(paramsStr, "%s", fWFDUrl0);
	//} else if(checkForName(lineStart, "wfd_3d_video_formats:", 21, paramsStr)) {
	//} else if(checkForName(lineStart, "wfd_display_edid:", 17, paramsStr)) {
	//} else if(checkForName(lineStart, "wfd_coupled_sink:", 17, paramsStr)) {
	}
  }

  delete[] dataCopy;
  return True;
}