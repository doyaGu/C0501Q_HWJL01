/**********
Wifi Display Source
**********/

#include "WifiDisplaySource.hh"
#include "RTSPCommon.hh"
#include <liveMedia.hh>
#include <string.h>
#include <GroupsockHelper.hh>

enum {
    STATE_INITIALIZED,
    STATE_AWAITING_CLIENT_CONNECTION,
    STATE_AWAITING_CLIENT_SETUP,
    STATE_AWAITING_CLIENT_PLAY,
    STATE_ABOUT_TO_PLAY,
    STATE_PLAYING,
    STATE_AWAITING_CLIENT_TEARDOWN,
    STATE_STOPPING,
    STATE_STOPPED,
};

static UsageEnvironment* wfdEnv;
static WifiDisplaySource::WFDClientSession* wfdClient;

WifiDisplaySource*
WifiDisplaySource::createNew(UsageEnvironment& env, Port ourPort,
			     UserAuthenticationDatabase* authDatabase,
			     unsigned reclamationTestSeconds) {
  wfdEnv = &env;
  int ourSocket = setUpOurSocket(env, ourPort);
  if (ourSocket == -1) return NULL;

  return new WifiDisplaySource(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

WifiDisplaySource::WifiDisplaySource(UsageEnvironment& env, int ourSocket,
				     Port ourPort,
				     UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds)
  : RTSPServerSupportingHTTPStreaming(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds) {
}

WifiDisplaySource::~WifiDisplaySource() {
}

static ServerMediaSession* createNewSMS(UsageEnvironment& env,
					char const* fileName, FILE* fid); // forward

ServerMediaSession*
WifiDisplaySource::lookupServerMediaSession(char const* streamName) {
  // First, check whether the specified "streamName" exists as a local file:
  FILE* fid = fopen(streamName, "rb");
  Boolean fileExists = fid != NULL;

  // Next, check whether we already have a "ServerMediaSession" for this file:
  ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(streamName);
  Boolean smsExists = sms != NULL;

  // Handle the four possibilities for "fileExists" and "smsExists":
  if (!fileExists) {
    if (smsExists) {
      // "sms" was created for a file that no longer exists. Remove it:
      removeServerMediaSession(sms);
    }
    return NULL;
  } else {
    if (!smsExists) {
      // Create a new "ServerMediaSession" object for streaming from the named file.
      sms = createNewSMS(envir(), streamName, fid);
      addServerMediaSession(sms);
    }
    fclose(fid);
    return sms;
  }
}

#define NEW_SMS(description) do {\
char const* descStr = description\
    ", streamed by the Wifi Display Source";\
sms = ServerMediaSession::createNew(env, fileName, fileName, descStr);\
} while(0)

static ServerMediaSession* createNewSMS(UsageEnvironment& env,
					char const* fileName, FILE* /*fid*/) {
  ServerMediaSession* sms = NULL;
  Boolean const reuseSource = False;

  // Assumed to be a MPEG Transport Stream file:
  // Use an index file name that's the same as the TS file name, except with ".tsx":
  unsigned indexFileNameLen = strlen(fileName) + 2; // allow for trailing "x\0"
  char* indexFileName = new char[indexFileNameLen];
  sprintf(indexFileName, "%sx", fileName);
  NEW_SMS("MPEG Transport Stream");
  sms->addSubsession(MPEG2TransportFileServerMediaSubsession::createNew(env, fileName, indexFileName, reuseSource));
  delete[] indexFileName;

  return sms;
}

// WFDClientSession

RTSPServer::RTSPClientSession*
WifiDisplaySource::createNewClientSession(unsigned sessionId, int clientSocket, struct sockaddr_in clientAddr) {
  wfdClient = new WFDClientSession(*this, sessionId, clientSocket, clientAddr);
  return wfdClient;
}


WifiDisplaySource::WFDClientSession
::WFDClientSession(RTSPServer& ourServer, unsigned sessionId, int clientSocket, struct sockaddr_in clientAddr)
  : RTSPClientSessionSupportingHTTPStreaming(ourServer, sessionId, clientSocket, clientAddr), fCSeq(1) {
  sendM1();
}

WifiDisplaySource::WFDClientSession::~WFDClientSession() {
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

static Boolean checkForHeader(char const* line, char const* headerName, unsigned headerNameLength, char const*& headerParams) {
  if (_strncasecmp(line, headerName, headerNameLength) != 0) return False;

  // The line begins with the desired header name.  Trim off any whitespace, and return the header parameters:
  unsigned paramIndex = headerNameLength;
  while (line[paramIndex] != '\0' && (line[paramIndex] == ' ' || line[paramIndex] == '\t')) ++paramIndex;
  if (&line[paramIndex] == '\0') return False; // the header is assumed to be bad if it has no parameters

  headerParams = &line[paramIndex];
  return True;
}

void
WifiDisplaySource::WFDClientSession::handleCmd_Response(unsigned responseCode, char const* responseStr) {  
    Boolean responseSuccess = False; // by default
    // Data was read OK.  Look through the data that we've read so far, to see if it contains <CR><LF><CR><LF>.
    // (If not, wait for more data to arrive.)
    Boolean endOfHeaders = False;        
    // Now that we have the complete response headers (ending with <CR><LF><CR><LF>), parse them to get the response code, CSeq,
    // and various other header parameters.  To do this, we first make a copy of the received header data, because we'll be
    // modifying it by adding '\0' bytes.
    char* headerDataCopy;
    RequestRecord* foundRequest = NULL;
    char const* wwwAuthenticateParamsStr = NULL;
    char const* publicParamsStr = NULL;
    char* bodyStart = NULL;
    unsigned numBodyBytes = 0;
    responseSuccess = False;
    do {
      headerDataCopy = strDup(responseStr);      
      char* lineStart = headerDataCopy;
      char* nextLineStart = getLine(lineStart);
      
      // Scan through the headers, handling the ones that we're interested in:
      Boolean reachedEndOfHeaders;
      unsigned cseq = 0;
      unsigned contentLength = 0;
      
	  while (1) {
	    reachedEndOfHeaders = True; // by default; may get changed below
	    lineStart = nextLineStart;
	    if (lineStart == NULL) break;
	
	    nextLineStart = getLine(lineStart);
	    if (lineStart[0] == '\0') break; // this is a blank line
	    reachedEndOfHeaders = False;
	
	    char const* headerParamsStr; 
        if (checkForHeader(lineStart, "Content-Length:", 15, headerParamsStr)) {
	      if (sscanf(headerParamsStr, "%u", &contentLength) != 1) {
	        envir().setResultMsg("Bad \"Content-Length:\" header: \"", lineStart, "\"");
	        break;
	      }
	    } else if (checkForHeader(lineStart, "Public:", 7, publicParamsStr)) {
	    }
	  }
      if (!reachedEndOfHeaders) break; // an error occurred
      
      // If we saw a "Content-Length:" header, then make sure that we have the amount of data that it specified:
      unsigned bodyOffset = nextLineStart - headerDataCopy;
      bodyStart = &headerDataCopy[bodyOffset];
      numBodyBytes = strlen(headerDataCopy) - bodyOffset;
      if (contentLength > numBodyBytes) {	
	    envir() << "Need received more.\n";
	    delete[] headerDataCopy;
	  }
      
      if (fRequest != NULL) {
	    Boolean needToResendCommand = False; // by default...
		if (responseCode == 401) { // && handleAuthenticationFailure(wwwAuthenticateParamsStr)) {
	      // We need to resend the command, with an "Authorization:" header:
	      needToResendCommand = True;
	  
	    } else if (responseCode == 301 || responseCode == 302) { // redirection
	      needToResendCommand = True;
		}
	
	    if (needToResendCommand) {
          envir() << "Resending...\n";
          if (fRequest != NULL && strcmp(fRequest->commandName(), "GET") != 0) fRequest->cseq() = ++fCSeq;
          if (sendRequest(fRequest) != 0) break;
	      delete[] headerDataCopy;
	      return; // without calling our response handler; the response to the resent command will do that
	    }
	  }     
      responseSuccess = True;
    } while (0);
    
    if (fRequest != NULL && fRequest->handler() != NULL) {
      int resultCode;
      char* resultString;
      if (responseSuccess) {
	    if (responseCode == 200) {
	      resultCode = 0;
	      resultString = numBodyBytes > 0 ? strDup(bodyStart) : strDup(publicParamsStr);
          // Note: The "strDup(bodyStart)" call assumes that the body is encoded without interior '\0' bytes
	    } else {
	      resultCode = responseCode;
	      resultString = strDup(responseStr);
	      envir().setResultMsg(responseStr);
	    }
	    (*fRequest->handler())(resultCode, resultString);
      } else {
	    // An error occurred parsing the response, so call the handler, indicating an error:
	    handleRequestError(fRequest);
      }
	}
    delete fRequest;
    delete[] headerDataCopy;
}

static void sendM3_1(void*) {
	wfdClient->sendM3();
}

void WifiDisplaySource::WFDClientSession::handleCmd_OPTIONS(char const* cseq) {
  char* const optionsResponse = 
	  "org.wfa.wfd1.0, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER";
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sPublic: %s\r\n\r\n",
	   cseq, dateHeader(), optionsResponse);

  envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)sendM3_1, this);
}

// M1
static void onReceiveM1Response(int resultCode, char* resultString) {
  if (resultCode != 0) {
    *wfdEnv << "Failed to M1 (Options): " << resultString << "\n";
  }
  delete[] resultString;
}

void WifiDisplaySource::WFDClientSession::sendM1() {
  sendRequest(new RequestRecord(++fCSeq, "OPTIONS", onReceiveM1Response));
}

static void sendM4_1(void*) {
	wfdClient->sendM4();
}

// M3
static void onReceiveM3Response(int resultCode, char* resultString) {
  if (resultCode != 0) {
    *wfdEnv << "Failed to M3 (GetParameter): " << resultString << "\n";
  }
  delete[] resultString;
  wfdEnv->taskScheduler().scheduleDelayedTask(0, (TaskFunc*)sendM4_1, wfdClient);
}

void WifiDisplaySource::WFDClientSession::sendM3() {
  char* const contentStr =
	  "wfd_video_formats\r\n"
      "wfd_audio_codecs\r\n"
      "wfd_3d_video_formats\r\n"
      "wfd_content_protection\r\n"
      "wfd_display_edid\r\n"
      "wfd_coupled_sink\r\n"
      "wfd_client_rtp_ports\r\n";
  sendRequest(new RequestRecord(++fCSeq, "GET_PARAMETER", onReceiveM3Response, contentStr));
}

static void sendM5_1(void*) {
	wfdClient->sendM5();
}

// M4
static void onReceiveM4Response(int resultCode, char* resultString) {
  if (resultCode != 0) {
    *wfdEnv << "Failed to M4 (SetParameter): " << resultString << "\n";
  }
  delete[] resultString;
  wfdEnv->taskScheduler().scheduleDelayedTask(0, (TaskFunc*)sendM5_1, wfdClient);

}

void WifiDisplaySource::WFDClientSession::sendM4() {
#if 0
    char* const contentStr =
      "wfd_video_formats: 00 00 01 01 00000001 00000000 00000000 00 0000 0000 00 none none\r\n"
      "wfd_audio_codecs: LPCM 00000002 00\r\n"
      "wfd_presentation_URL: rtsp://192.168.69.40/wfd1.0/streamid=0 none\r\n"
      "wfd_client_rtp_ports: RTP/AVP/UDP;unicast 1028 0 mode=play\r\n";  
#else
    char* contentStr = 0;
    char* const act_contentStr =
      "wfd_video_formats: 00 00 01 01 00000001 00000000 00000000 00 0000 0000 00 none none\r\n"
      "wfd_audio_codecs: LPCM 00000002 00\r\n"
      "wfd_presentation_URL: rtsp://%s/wfd1.0/streamid=0 none\r\n"
      "wfd_client_rtp_ports: RTP/AVP/UDP;unicast 1028 0 mode=play\r\n";  
    struct sockaddr_in ourAddress;
    char tmp_str[512] = {0};

    ourAddress.sin_addr.s_addr = ourIPAddress((*wfdEnv));

    snprintf(tmp_str, 512, act_contentStr, AddressString(ourAddress).val());
    contentStr = tmp_str;
#endif
    sendRequest(new RequestRecord(++fCSeq, "SET_PARAMETER", onReceiveM4Response, contentStr));
}

// M5
static void onReceiveM5Response(int resultCode, char* resultString) {
  if (resultCode != 0) {
    *wfdEnv << "Failed to M5 (SetParameter): " << resultString << "\n";
  }
  delete[] resultString;
}

void WifiDisplaySource::WFDClientSession::sendM5() {
  char* const contentStr =
	  "wfd_trigger_method: SETUP";
  sendRequest(new RequestRecord(++fCSeq, "SET_PARAMETER", onReceiveM5Response, contentStr));
}

// M16
static void onReceiveM16Response(int resultCode, char* resultString) {
  if (resultCode != 0) {
    *wfdEnv << "Failed to M15 (GetParameter): " << resultString << "\n";
  }
  delete[] resultString;
}

void WifiDisplaySource::WFDClientSession::sendM16() {
  sendRequest(new RequestRecord(++fCSeq, "GET_PARAMETER", onReceiveM16Response));
}

unsigned WifiDisplaySource::WFDClientSession::sendRequest(RequestRecord* request) {
  char* cmd = NULL;
  do {
    // Construct and send the command:
    // First, construct command-specific headers that we need:

	char* cmdURL = "rtsp://localhost/wfd1.0"; // by default
    Boolean cmdURLWasAllocated = False;

    char const* protocolStr = "RTSP/1.0"; // by default

    char* extraHeaders = (char*)""; // by default
    Boolean extraHeadersWereAllocated = False; 

    char* contentLengthHeader = (char*)""; // by default
    Boolean contentLengthHeaderWasAllocated = False;

    char const* contentStr = request->contentStr(); // by default
    if (contentStr == NULL) contentStr = "";
    unsigned contentStrLen = strlen(contentStr);
    if (contentStrLen > 0) {
      char const* contentLengthHeaderFmt =
	"Content-Length: %d\r\n";
      unsigned contentLengthHeaderSize = strlen(contentLengthHeaderFmt)
	+ 20 /* max int len */;
      contentLengthHeader = new char[contentLengthHeaderSize];
      sprintf(contentLengthHeader, contentLengthHeaderFmt, contentStrLen);
      contentLengthHeaderWasAllocated = True;
    }

    if (strcmp(request->commandName(), "OPTIONS") == 0) {
      cmdURL = strDup("*");
      cmdURLWasAllocated = True;
      extraHeaders = strDup("Require: org.wfa.wfd1.0\r\n");
      extraHeadersWereAllocated = True; 
    } else if (strcmp(request->commandName(), "GET_PARAMETER") == 0) {
      extraHeaders = strDup("Content-Type: text/parameters\r\n");
      extraHeadersWereAllocated = True; 
    } else if (strcmp(request->commandName(), "SET_PARAMETER") == 0) {
      extraHeaders = strDup("Content-Type: text/parameters\r\n");
      extraHeadersWereAllocated = True; 
    }

    //char* authenticatorStr = createAuthenticatorString(request->commandName(), fBaseURL);

    char const* const cmdFmt =
      "%s %s %s\r\n"
      "CSeq: %d\r\n"
      //"%s"
      //"%s"
      "%s"
      "%s"
      "\r\n"
      "%s";
    unsigned cmdSize = strlen(cmdFmt)
      + strlen(request->commandName()) + strlen(cmdURL) + strlen(protocolStr)
      + 20 /* max int len */
      //+ strlen(fAuthenticatorStr)
      //+ fUserAgentHeaderStrLen
      + strlen(extraHeaders)
      + strlen(contentLengthHeader)
      + contentStrLen;
    cmd = new char[cmdSize];
    sprintf(cmd, cmdFmt,
	    request->commandName(), cmdURL, protocolStr,
	    request->cseq(),
	    //authenticatorStr,
	    //fUserAgentHeaderStr,
        extraHeaders,
	    contentLengthHeader,
	    contentStr);
    //delete [] authenticatorStr;
    if (cmdURLWasAllocated) delete[] cmdURL;
    if (extraHeadersWereAllocated) delete[] extraHeaders;
    if (contentLengthHeaderWasAllocated) delete[] contentLengthHeader;

    if (send(fClientOutputSocket, cmd, strlen(cmd), 0) < 0) {
      char const* errFmt = "%s send() failed: ";
      unsigned const errLength = strlen(errFmt) + strlen(request->commandName());
      char* err = new char[errLength];
      sprintf(err, errFmt, request->commandName());
      envir().setResultErrMsg(err);
      delete[] err;
      break;
    }

    // The command send succeeded, so enqueue the request record, so that its response (when it comes) can be handled:
    fRequest = request;

    delete[] cmd;
    return request->cseq();
  } while (0);

  // An error occurred, so call the response handler immediately (indicating the error):
  delete[] cmd;
  handleRequestError(request);
  delete request;
  return 0;
}

void WifiDisplaySource::WFDClientSession::handleRequestError(RequestRecord* request) {
  int resultCode = -envir().getErrno();
  if (resultCode == 0) {
    // Choose some generic error code instead:
#if 0 //defined(__WIN32__) || defined(_WIN32) || defined(_QNX4)
    resultCode = -WSAENOTCONN;
#else
    resultCode = -ENOTCONN;
#endif
  }
  if (request->handler() != NULL) (*request->handler())(resultCode, strDup(envir().getResultMsg()));
}

WifiDisplaySource::WFDClientSession::RequestRecord::RequestRecord(unsigned cseq, char const* commandName, 
			responseHandler* handler, char const* contentStr)
  : fCSeq(cseq), fCommandName(commandName), fHandler(handler), fContentStr(strDup(contentStr)) {
}

WifiDisplaySource::WFDClientSession::RequestRecord::~RequestRecord() {
  delete [] fContentStr;
}

