/**********
Wifi Display source play
**********/
// Copyright (c) 1996-2012, Live Networks, Inc.  All rights reserved
// LIVE555 Media Server
// main program

#include <BasicUsageEnvironment.hh>
#include "WifiDisplaySource.hh"
#include "version.hh"

extern "C" int WFDSourceDevice_main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

  UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
  // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("username1", "password1"); // replace these with real strings
  // Repeat the above with each <username>, <password> that you wish to allow
  // access to the server.
#endif

  // Create the RTSP server.  Try first with the default port number (554),
  // and then with the alternative port number (8554):
  WifiDisplaySource* wfdSource;
  portNumBits wfdSourcePortNum = 7236;
  wfdSource = WifiDisplaySource::createNew(*env, wfdSourcePortNum, authDB);
  if (wfdSource == NULL) {
    *env << "Failed to create Wifi Display Source server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  char* urlPrefix = wfdSource->rtspURLPrefix();
  *env << "Wifi Display Source Server\n";
  *env << "Play streams from this server using the URL\n\t"
       << urlPrefix << "<filename>\nwhere <filename> is a file present in the current directory.\n";

  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).
/*  
  if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
    *env << "(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling, or for HTTP live streaming (for indexed Transport Stream files only).)\n";
  } else {
    *env << "(RTSP-over-HTTP tunneling is not available.)\n";
  }
*/

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}
