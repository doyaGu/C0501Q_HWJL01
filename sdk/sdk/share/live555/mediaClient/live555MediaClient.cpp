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
// Copyright (c) 1996-2012, Live Networks, Inc.  All rights reserved
// A demo application, showing how to create and run a RTSP client (that can potentially receive multiple streams concurrently).
//
// NOTE: This code - although it builds a running application - is intended only to illustrate how to develop your own RTSP
// client application.  For a full-featured RTSP client application - with much more functionality, and many options - see
// "openRTSP": http://www.live555.com/openRTSP/

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include <pthread.h>
#include <semaphore.h>
#include "ite/itv.h"
#include "castor3player.h"
#include "itc_ipcam.h"

pthread_mutex_t rtsp_mutex = PTHREAD_MUTEX_INITIALIZER;
RTSPCLIENT_MODE	RtspClient_Mode = MODE_NONE;

// Forward function definitions:
static void *live555MediaClient_main(void *arg);

// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

// Other event handler functions:
void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void* clientData); // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);
  // called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

// The main streaming routine (for each "rtsp://" URL):
void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL);

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

void usage(UsageEnvironment& env, char const* progName) {
  env << "Usage: " << progName << " <rtsp-url-1> ... <rtsp-url-N>\n";
  env << "\t(where each <rtsp-url-i> is a \"rtsp://\" URL)\n";
}
char eventLoopWatchVariable;
bool isParseSPSandPPS;
static RTSPClient* OurRtspClient = NULL;
pthread_t tid;
sem_t sem;
cb_RtspHandler_t Rtsp_Client_cb;
RTSP_MEDIA_INFO sdp_media_info = {0};

unsigned char *temp_packet = NULL;
double last_packet_timestamp;
int total_packet_size = 0;

extern "C" int startRTSPClient(char *ip, int port, char *file_name, cb_RtspHandler_t callback)
{
    int rc;
	char temp[128] = {0};
	pthread_mutex_lock(&rtsp_mutex);
	sem_init(&sem, 0, 0);
    eventLoopWatchVariable = 0;
    isParseSPSandPPS = false;
	last_packet_timestamp = 0;
	total_packet_size = 0;
	if(temp_packet)
	{
		free(temp_packet);
		temp_packet = NULL;
	}
	Rtsp_Client_cb = callback;
#if defined(CFG_BUILD_ITV) && !defined(CFG_TEST_RTSPCLIENT)
	if(RtspClient_Mode != IPCAM_MODE)
		itv_set_pb_mode(1);
#endif

	if(file_name)
		sprintf(temp,"rtsp://%s:%d/%s", ip, port, file_name);
	else
		sprintf(temp,"rtsp://%s:%d/", ip, port);
    rc = pthread_create(&tid, NULL, live555MediaClient_main, (void *)temp);
    if(rc)
    {
        printf("Create thread fail...\n");
		pthread_mutex_unlock(&rtsp_mutex);
        return -1;
    }

	sem_wait(&sem);
	sem_destroy(&sem);
	pthread_mutex_unlock(&rtsp_mutex);
    return 0;
}

extern "C" int stopRTSPClient(void)
{
	pthread_mutex_lock(&rtsp_mutex);
    eventLoopWatchVariable = 1;
	if(tid)
	{
		pthread_join(tid, NULL);
		tid = 0;
	}		
#ifdef CFG_BUILD_ITV
	if(RtspClient_Mode != IPCAM_MODE)
		itv_set_pb_mode(0);
#endif
	memset(&sdp_media_info, 0, sizeof(sdp_media_info));
	pthread_mutex_unlock(&rtsp_mutex);
    return 0;
}

extern "C" void SetRTSPClientMode(RTSPCLIENT_MODE mode)
{
	RtspClient_Mode = mode;
}

static void *live555MediaClient_main(void *arg) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
  char *rtsp_url = (char *)arg;

  printf("Rtsp_url = %s\n", rtsp_url);
  //openURL(*env, NULL, "rtsp://192.168.1.100:8554/");
  openURL(*env, NULL, rtsp_url);
  
  sem_post(&sem);
  // All subsequent activity takes place within the event loop:
  env->taskScheduler().doEventLoop(&eventLoopWatchVariable); // does not return

  if(OurRtspClient != NULL)
  {
    shutdownStream(OurRtspClient);
  }
  pthread_exit(NULL);
  return 0; // We never actually get to this line; this is only to prevent a possible compiler warning
}

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

class StreamClientState {
public:
  StreamClientState();
  virtual ~StreamClientState();

public:
  MediaSubsessionIterator* iter;
  MediaSession* session;
  MediaSubsession* subsession;
  TaskToken streamTimerTask;
  double duration;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// struture for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:

class ourRTSPClient: public RTSPClient {
public:
  static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
				  int verbosityLevel = 0,
				  char const* applicationName = NULL,
				  portNumBits tunnelOverHTTPPortNum = 0);

protected:
  ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
		int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
  virtual ~ourRTSPClient();

public:
  StreamClientState scs;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink: public MediaSink {
public:
  static DummySink* createNew(UsageEnvironment& env,
			      MediaSubsession& subsession, // identifies the kind of data that's being received
			      char const* streamId = NULL); // identifies the stream itself (optional)

private:
  DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
    // called only by "createNew()"
  virtual ~DummySink();

  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
				struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
			 struct timeval presentationTime, unsigned durationInMicroseconds);

private:
  // redefined virtual functions:
  virtual Boolean continuePlaying();

private:
  u_int8_t* fReceiveBuffer;
  MediaSubsession& fSubsession;
  char* fStreamId;
};

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

static unsigned rtspClientCount = 0; // Counts how many streams (i.e., "RTSPClient"s) are currently in use.

void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL) {
  // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
  // to receive (even if more than stream uses the same "rtsp://" URL).
  RTSPClient* rtspClient = ourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
  if (rtspClient == NULL) {
    env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
    return;
  }
  OurRtspClient = rtspClient;
  ++rtspClientCount;

  // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
  // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
  // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
  rtspClient->sendDescribeCommand(continueAfterDESCRIBE); 
}


// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
      break;
    }

    char* sdpDescription = resultString;
    env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

    // Create a media session object from this SDP description:
    scs.session = MediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore

	if(Rtsp_Client_cb)
  		Rtsp_Client_cb(STREAM_GET_INFO, &sdp_media_info);
	
    if (scs.session == NULL) {
      env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
    // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
    // (Each 'subsession' will have its own data source.)
    scs.iter = new MediaSubsessionIterator(*scs.session);
    setupNextSubsession(rtspClient);
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  shutdownStream(rtspClient);
  if(Rtsp_Client_cb)
  	Rtsp_Client_cb(STREAM_ERROR, NULL);
}

void setupNextSubsession(RTSPClient* rtspClient) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
  
  scs.subsession = scs.iter->next();
  if (scs.subsession != NULL) {
    if (!scs.subsession->initiate()) {
      env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
      setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
    } else {
      env << *rtspClient << "Initiated the \"" << *scs.subsession
	  << "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1 << ")\n";

      // Continue setting up this subsession, by sending a RTSP "SETUP" command:
      rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP);
    }
    return;
  }

  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
  rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
      break;
    }

    env << *rtspClient << "Set up the \"" << *scs.subsession
	<< "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1 << ")\n";

    // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
    // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
    // after we've sent a RTSP "PLAY" command.)
	
    scs.subsession->sink = DummySink::createNew(env, *scs.subsession, rtspClient->url());
      // perhaps use your own custom "MediaSink" subclass instead
    if (scs.subsession->sink == NULL) {
      env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
	  << "\" subsession: " << env.getResultMsg() << "\n";
      break;
    }

    env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
    scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession 
    
    scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
				       subsessionAfterPlaying, scs.subsession);
				       
    // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
    if (scs.subsession->rtcpInstance() != NULL) {
      scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
    }
    
  } while (0);

  // Set up the next subsession, if any:
  setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
  	//MTAL_SPEC mtal_spec = {0};
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
      break;
    }

    // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
    // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
    // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
    // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
    if (scs.duration > 0) {
      unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
      scs.duration += delaySlop;
      int64_t uSecsToDelay = (int64_t)(scs.duration*1000000);
      scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
    }

    env << *rtspClient << "Started playing session";

    if (scs.duration > 0) {
      env << " (for up to " << scs.duration << " seconds)";
    }
    env << "...\n";

    if(Rtsp_Client_cb)
		Rtsp_Client_cb(STREAM_START_PLAYING, NULL);
    
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  shutdownStream(rtspClient);
}


// Implementation of the other event handlers:

void subsessionAfterPlaying(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

  // Begin by closing this subsession's stream:
  printf("CodecName = %s\n", subsession->codecName());
  if(RtspClient_Mode != IPCAM_MODE)
  {
	  if(0 == strcmp(subsession->codecName(), "H264"))
	  {
	    mtal_pb_DeinitAVDecodeEnv();
	  }
  }
  Medium::close(subsession->sink);
  subsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession& session = subsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((subsession = iter.next()) != NULL) {
    if (subsession->sink != NULL) return; // this subsession is still active
  }

  // All subsessions' streams have now been closed, so shutdown the client:
  shutdownStream(rtspClient);
}

void subsessionByeHandler(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
  UsageEnvironment& env = rtspClient->envir(); // alias

  env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";

  // Now act as if the subsession had closed:
  subsessionAfterPlaying(subsession);
  if(!OurRtspClient && Rtsp_Client_cb)
  	Rtsp_Client_cb(STREAM_EOF, NULL);		
}

void streamTimerHandler(void* clientData) {
  ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
  StreamClientState& scs = rtspClient->scs; // alias

  scs.streamTimerTask = NULL;

  // Shut down the stream:
  shutdownStream(rtspClient);
  if(!OurRtspClient && Rtsp_Client_cb)
  	Rtsp_Client_cb(STREAM_EOF, NULL);
}

void shutdownStream(RTSPClient* rtspClient, int exitCode) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

  // First, check whether any subsessions have still to be closed:
  if (scs.session != NULL) { 
    Boolean someSubsessionsWereActive = False;
    MediaSubsessionIterator iter(*scs.session);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
      if (subsession->sink != NULL) {
	  	if(RtspClient_Mode != IPCAM_MODE)
	  	{
	        if(0 == strcmp(subsession->codecName(), "H264"))
	        {
	            mtal_pb_DeinitAVDecodeEnv();
	        }
	  	}
	    Medium::close(subsession->sink);
	    subsession->sink = NULL;
	    someSubsessionsWereActive = True;
      }
    }

    if (someSubsessionsWereActive) {
      // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
      // Don't bother handling the response to the "TEARDOWN".
      rtspClient->sendTeardownCommand(*scs.session, NULL);
    }
  }

  env << *rtspClient << "Closing the stream.\n";
  Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.  
  if (--rtspClientCount == 0) {
    OurRtspClient = NULL;
    eventLoopWatchVariable = 1;
    isParseSPSandPPS = false;
    // The final stream has ended, so exit the application now.
    // (Of course, if you're embedding this code into your own application, you might want to comment this out.)
    //exit(exitCode);
  }
}


// Implementation of "ourRTSPClient":

ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
					int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) {
  return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
			     int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
  : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum) {
}

ourRTSPClient::~ourRTSPClient() {
}


// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
  : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0) {
}

StreamClientState::~StreamClientState() {
  delete iter;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment& env = session->envir(); // alias

    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    Medium::close(session);
  }
}


// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 1*1024*1024

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId) {
  return new DummySink(env, subsession, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
  : MediaSink(env),
    fSubsession(subsession) {
  fStreamId = strDup(streamId);
  fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
  //pH264 = (unsigned char*)malloc(256*1024);
  //ithMediaPlayer_InitH264DecodeEnv();
  if(RtspClient_Mode != IPCAM_MODE)
  {
	  mtal_pb_InitAVDecodeEnv();
	  if(0 == strncmp(fSubsession.codecName(), "H264", 16))
	  {
	  	mtal_pb_InitH264DecodeEnv();
	  } 
	  else if(0 == strncmp(fSubsession.codecName(), "PCMU", 16))
	  {
	  	mtal_pb_InitAudioDecodeEnv(fSubsession.rtpTimestampFrequency(), fSubsession.numChannels(), RTSP_CODEC_PCMU);
		if(Rtsp_Client_cb)
			Rtsp_Client_cb(STREAM_SET_VOLUME, NULL);
	  }
	  else if(0 == strncmp(fSubsession.codecName(), "MPA", 16))
	  {
	  	mtal_pb_InitAudioDecodeEnv(fSubsession.rtpTimestampFrequency(), fSubsession.numChannels(), RTSP_CODEC_MPA);
		if(Rtsp_Client_cb)
			Rtsp_Client_cb(STREAM_SET_VOLUME, NULL);
	  }
	  else if(0 == strncmp(fSubsession.codecName(), "MPEG4-GENERIC", 16) || 0 == strncmp(fSubsession.codecName(), "MP4A-LATM", 16))
	  {
	  	mtal_pb_InitAudioDecodeEnv(fSubsession.rtpTimestampFrequency(), fSubsession.numChannels(), RTSP_CODEC_MPEG4_AAC);
		if(Rtsp_Client_cb)
			Rtsp_Client_cb(STREAM_SET_VOLUME, NULL);
	  }
  }
}

DummySink::~DummySink() {
  delete[] fReceiveBuffer;
  delete[] fStreamId;
/*  
  if(NULL != pH264)
  	free(pH264);
*/  
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  DummySink* sink = (DummySink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 0
#define DEBUG_PRINT_NPT 0
#define DEBUG_DUMP_H264_TO_FILE 0
void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
  // We've just received a frame of data.  (Optionally) print out information about it:  
#if DEBUG_PRINT_EACH_RECEIVED_FRAME
  if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
  envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
  if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
  char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
  sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
  envir() << ".\tPresentation time: " << (unsigned)presentationTime.tv_sec << "." << uSecsStr;
  if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
    envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
  }
#if DEBUG_PRINT_NPT
  envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
  envir() << "\n";
#endif
	if(RtspClient_Mode == IPCAM_MODE)
	{
		if(strncmp(fSubsession.codecName(), "H264", 16) == 0)
		{
			unsigned char *pH264_packet = NULL;
			unsigned char nalu_header[4] = { 0, 0, 0, 1 };
			unsigned int num = 0;

			if(!isParseSPSandPPS)
			{
				SPropRecord *pSPropRecord;
				pSPropRecord = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(), num);
				for(unsigned int i = 0; i < num; i++)
				{
					unsigned char *pHeader_packet = NULL;
					pHeader_packet = (unsigned char*)malloc(pSPropRecord[i].sPropLength + 4);
					memcpy(pHeader_packet, &nalu_header[0], 4);
	            	memcpy(pHeader_packet + 4, pSPropRecord[i].sPropBytes, pSPropRecord[i].sPropLength);
#ifdef CFG_BUILD_MEDIASTREAMER2					
					PutIntoPacketQueue(pHeader_packet, pSPropRecord[i].sPropLength + 4, fSubsession.getNormalPlayTime(presentationTime));
#endif
				}
				delete[] pSPropRecord;
				isParseSPSandPPS = true;
			}
			pH264_packet = (unsigned char*)malloc(frameSize + 4);
			memcpy(pH264_packet, &nalu_header[0], 4);
			memcpy(pH264_packet + 4, fReceiveBuffer, frameSize);
#ifdef CFG_BUILD_MEDIASTREAMER2
			PutIntoPacketQueue(pH264_packet, frameSize + 4, fSubsession.getNormalPlayTime(presentationTime));
#endif
		}
	}
	else
	{
	    if(0 == strncmp(fSubsession.codecName(), "H264", 16))
	    {
			if(last_packet_timestamp == 0 || last_packet_timestamp == fSubsession.getNormalPlayTime(presentationTime))
			{
NEXT_PACKET:		
				unsigned char nalu_header[4] = { 0, 0, 0, 1 };   
		        unsigned char extraData[256]; 
		        unsigned int num = 0;
				unsigned int extraLen;
				
				if(!temp_packet)
					temp_packet = (unsigned char*)malloc(DUMMY_SINK_RECEIVE_BUFFER_SIZE);
				last_packet_timestamp = fSubsession.getNormalPlayTime(presentationTime);
				
				extraLen = 0;
				if(!isParseSPSandPPS)
		        {
		            SPropRecord *pSPropRecord;
		            pSPropRecord = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(), num);
		        //p_record[0] is sps 
		        //p+record[1] is pps
		        	for(unsigned int i = 0; i < num; i++){ 
		            	memcpy(&extraData[extraLen], &nalu_header[0], 4);
		            	extraLen += 4;
		            	memcpy(&extraData[extraLen], pSPropRecord[i].sPropBytes, pSPropRecord[i].sPropLength);
		            	extraLen += pSPropRecord[i].sPropLength;
		        	}/*for i*/
		            delete[] pSPropRecord;
		            isParseSPSandPPS = true;
		        }
				memcpy(&extraData[extraLen], &nalu_header[0], 4);
		        extraLen += 4;
				memcpy(temp_packet+total_packet_size, &extraData[0], extraLen);
				memcpy(temp_packet+total_packet_size+extraLen, fReceiveBuffer, frameSize);
				total_packet_size = total_packet_size + frameSize + extraLen;
			}
			else
			{
		
		        unsigned char *pH264_packet = NULL;
				#if 0
		        unsigned char nalu_header[4] = { 0, 0, 0, 1 };   
		        unsigned char extraData[256]; 
		        unsigned int num = 0;
				unsigned int extraLen;

		        extraLen = 0;
		        if(!isParseSPSandPPS)
		        {
		            SPropRecord *pSPropRecord;
		            pSPropRecord = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(), num);
		        //p_record[0] is sps 
		        //p+record[1] is pps
		        	for(unsigned int i = 0; i < num; i++){ 
		            	memcpy(&extraData[extraLen], &nalu_header[0], 4);
		            	extraLen += 4;
		            	memcpy(&extraData[extraLen], pSPropRecord[i].sPropBytes, pSPropRecord[i].sPropLength);
		            	extraLen += pSPropRecord[i].sPropLength;
		        	}/*for i*/
		            delete[] pSPropRecord;
		            isParseSPSandPPS = true;
		        }
		        memcpy(&extraData[extraLen], &nalu_header[0], 4);
		        extraLen += 4;  
				#endif
				pH264_packet = (unsigned char*)malloc(total_packet_size);
		        //memcpy(pH264_packet, &extraData[0], extraLen);
		        memcpy(pH264_packet, temp_packet, total_packet_size);
				memset(temp_packet, 0, sizeof(temp_packet));
				
		        int totalSize;
		        totalSize = total_packet_size;
				total_packet_size = 0;
#if DEBUG_DUMP_H264_TO_FILE
				int ret = 0;
				static FILE *fp = fopen("D:/saved.h264", "wb");

		        ret = fwrite(pH264_packet, 1,  totalSize, fp);
		        fflush(fp);
		        printf("\tSave %d bytes to file, packet totalSize = %d\n", ret, totalSize);

				if(pH264_packet)
		        {
		            free(pH264_packet);
		            pH264_packet = NULL;
		        }
#else
		        mtal_pb_h264_decode_from_rtsp(pH264_packet, totalSize, last_packet_timestamp);
#endif
				goto NEXT_PACKET;
			}             
	    }	
		else if(0 == strncmp(fSubsession.codecName(), "PCMU", 16))
		{
			unsigned char *pPCMU_packet = NULL;
			pPCMU_packet = (unsigned char*)malloc(frameSize);
			memcpy(pPCMU_packet, fReceiveBuffer, frameSize);
			//printf("YC: %s, %d, samplerate = %d, channel_num = %d\n", __FUNCTION__, __LINE__, fSubsession.rtpTimestampFrequency(), fSubsession.numChannels());
			if(fSubsession.getNormalPlayTime(presentationTime))
				mtal_pb_audio_decode_from_rtsp(pPCMU_packet, frameSize, fSubsession.getNormalPlayTime(presentationTime));
		}
		else if(0 == strncmp(fSubsession.codecName(), "MPA", 16)) //mp4
		{
			unsigned char *pMPA_packet = NULL;
			pMPA_packet = (unsigned char*)malloc(frameSize);
			memcpy(pMPA_packet, fReceiveBuffer, frameSize);
			//printf("YC: %s, %d, samplerate = %d, channel_num = %d\n", __FUNCTION__, __LINE__, fSubsession.rtpTimestampFrequency(), fSubsession.numChannels());
			if(fSubsession.getNormalPlayTime(presentationTime))
				mtal_pb_audio_decode_from_rtsp(pMPA_packet, frameSize, fSubsession.getNormalPlayTime(presentationTime));
		}
		else if(0 == strncmp(fSubsession.codecName(), "MPEG4-GENERIC", 16) || 0 == strncmp(fSubsession.codecName(), "MP4A-LATM", 16)) //aac
		{
			unsigned char *pAAC_packet = NULL;
			pAAC_packet = (unsigned char*)malloc(frameSize);
			memcpy(pAAC_packet, fReceiveBuffer, frameSize);
			//printf("YC: %s, %d, samplerate = %d, channel_num = %d\n", __FUNCTION__, __LINE__, fSubsession.rtpTimestampFrequency(), fSubsession.numChannels());
			if(fSubsession.getNormalPlayTime(presentationTime))
				mtal_pb_audio_decode_from_rtsp(pAAC_packet, frameSize, fSubsession.getNormalPlayTime(presentationTime));
		}
	}
  // Then continue, to request the next frame of data:
  continuePlaying();
}

Boolean DummySink::continuePlaying() {
  if (fSource == NULL) return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
                        afterGettingFrame, this,
                        onSourceClosure, this);
  return True;
}

