/**********
Play Wifi Display
Designed by powei
**********/


#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "WifiDisplaySink.hh"
#include "ite/itp.h"
#include "sys/times.h"

#if 0 //defined(__WIN32__) || defined(_WIN32)
#define snprintf _snprintf
#endif


struct timeval startTime;
WifiDisplaySink* wfdSinkDevice;
//TaskToken sessionTimerTask = NULL;
//TaskToken arrivalCheckTimerTask = NULL;
//TaskToken interPacketGapCheckTimerTask = NULL;
//TaskToken qosMeasurementTimerTask = NULL;

//static void shutdown(int exitCode) {
//
//  Medium::close(wfdSinkDevice);
//
//  if (env != NULL) {
//    env->taskScheduler().unscheduleDelayedTask(sessionTimerTask);
//    env->taskScheduler().unscheduleDelayedTask(arrivalCheckTimerTask);
//    env->taskScheduler().unscheduleDelayedTask(interPacketGapCheckTimerTask);
//    env->taskScheduler().unscheduleDelayedTask(qosMeasurementTimerTask);
//  }
//  exit(exitCode);
//}

extern "C" int WFDSinkDevice_main(int argc, char** argv) {
    TaskScheduler* scheduler;
    UsageEnvironment* env;
    Authenticator* ourAuthenticator = NULL;
    int verbosityLevel = 1; // by default, print verbose output
    char const* progName;
    char* url;

    // Begin by setting up our usage environment:
    scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);
    progName = argv[0];

    if (argc != 2) 
    {
        // there must be exactly one "rtsp://" URL at the end
        *env << "Usage: " << progName << "rtsp://192.168.1.2:7236\n";	  
        exit(1);
    }

    url = new char [strlen(argv[1]) + 6];  // example: "rtsp://192.168.1.2:7236" , this is rtsp control port rather rtp port
    sprintf(url, "%s:7236", argv[1]);
    gettimeofday(&startTime, NULL);

    // Create our client object:
    wfdSinkDevice = WifiDisplaySink::createNew(*env, url, verbosityLevel, progName, ourAuthenticator);
    if (wfdSinkDevice == NULL) {
        *env << "Failed to create Wifi Display Sink\n";
        exit(1);
    }
    delete url;

    // All subsequent activity takes place within the event loop:
    env->taskScheduler().doEventLoop(); // does not return

    return 0; // only to prevent compiler warning
}
