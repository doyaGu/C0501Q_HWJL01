﻿/**********
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
// "liveMedia"
// Copyright (c) 1996-2012 Live Networks, Inc.  All rights reserved.
// A server that supports both RTSP, and HTTP streaming (using Apple's "HTTP Live Streaming" protocol)
// Implementation

#include "RTSPServerSupportingHTTPStreaming.hh"
#include "RTSPCommon.hh"
#include <sys/stat.h>
#include <time.h>

RTSPServerSupportingHTTPStreaming*
RTSPServerSupportingHTTPStreaming::createNew(UsageEnvironment& env, Port rtspPort,
					     UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds) {
  int ourSocket = setUpOurSocket(env, rtspPort);
  if (ourSocket == -1) return NULL;

  return new RTSPServerSupportingHTTPStreaming(env, ourSocket, rtspPort, authDatabase, reclamationTestSeconds);
}

RTSPServerSupportingHTTPStreaming
::RTSPServerSupportingHTTPStreaming(UsageEnvironment& env, int ourSocket, Port rtspPort,
				    UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds)
  : RTSPServer(env, ourSocket, rtspPort, authDatabase, reclamationTestSeconds) {
}

RTSPServerSupportingHTTPStreaming::~RTSPServerSupportingHTTPStreaming() {
}

RTSPServer::RTSPClientSession*
RTSPServerSupportingHTTPStreaming::createNewClientSession(unsigned sessionId, int clientSocket, struct sockaddr_in clientAddr) {
  return new RTSPClientSessionSupportingHTTPStreaming(*this, sessionId, clientSocket, clientAddr);
}

RTSPServerSupportingHTTPStreaming::RTSPClientSessionSupportingHTTPStreaming
::RTSPClientSessionSupportingHTTPStreaming(RTSPServer& ourServer, unsigned sessionId, int clientSocket, struct sockaddr_in clientAddr)
  : RTSPClientSession(ourServer, sessionId, clientSocket, clientAddr),
    fPlaylistSource(NULL), fTCPSink(NULL) {
}

RTSPServerSupportingHTTPStreaming::RTSPClientSessionSupportingHTTPStreaming::~RTSPClientSessionSupportingHTTPStreaming() {
  Medium::close(fPlaylistSource);
  Medium::close(fTCPSink);
}

static char const* lastModifiedHeader(char const* fileName) {
  static char buf[200];

  struct stat sb;
  int statResult = stat(fileName, &sb);
  if (statResult != 0) {
    // Failed to 'stat' the file; return an empty string
    buf[0] = '\0';
  } else {
    strftime(buf, sizeof buf, "Last-Modified: %a, %b %d %Y %H:%M:%S GMT\r\n", gmtime((const time_t*)&sb.st_mtime));
  }

  return buf;
}

void RTSPServerSupportingHTTPStreaming::RTSPClientSessionSupportingHTTPStreaming
::handleHTTPCmd_StreamingGET(char const* urlSuffix, char const* /*fullRequestStr*/) {
  // If "urlSuffix" ends with "?segment=<offset-in-seconds>,<duration-in-seconds>", then strip this off, and send the
  // specified segment.  Otherwise, construct and send a playlist that consists of segments from the specified file.
  do {
    char const* questionMarkPos = strrchr(urlSuffix, '?');
    if (questionMarkPos == NULL) break;
    unsigned offsetInSeconds, durationInSeconds;
    if (sscanf(questionMarkPos, "?segment=%u,%u", &offsetInSeconds, &durationInSeconds) != 2) break;

    char* streamName = strDup(urlSuffix);
    streamName[questionMarkPos-urlSuffix] = '\0';

    do {
      ServerMediaSession* session = fOurServer.lookupServerMediaSession(streamName);
      if (session == NULL) {
        handleHTTPCmd_notFound();
        break;
      }

      // We can't send multi-subsession streams over HTTP (because there's no defined way to multiplex more than one subsession).
      // Therefore, use the first (and presumed only) substream:
      ServerMediaSubsessionIterator iter(*session);
      ServerMediaSubsession* subsession = iter.next();
      if (subsession == NULL) {
        // Treat an 'empty' ServerMediaSession the same as one that doesn't exist at all:
        handleHTTPCmd_notFound();
        break;
      }

      // Call "getStreamParameters()" to create the stream's source.  (Because we're not actually streaming via RTP/RTCP, most
      // of the parameters to the call are dummy.)
      Port clientRTPPort(0), clientRTCPPort(0), serverRTPPort(0), serverRTCPPort(0);
      netAddressBits destinationAddress = 0;
      u_int8_t destinationTTL = 0;
      Boolean isMulticast = False;
      void* streamToken;
      subsession->getStreamParameters(fOurSessionId, 0, clientRTPPort,clientRTCPPort, 0,0,0, destinationAddress,destinationTTL, isMulticast, serverRTPPort,serverRTCPPort, streamToken);
      
      // Set up our "fStreamStates", just as we would if we were handling a RTSP "SETUP":
      reclaimStreamStates();
      fNumStreamStates = 1;
      fStreamStates = new struct streamState[fNumStreamStates];
      fStreamStates[0].subsession = subsession;
      fStreamStates[0].streamToken = streamToken;
      
      // Seek the stream source to the desired place, with the desired duration, and (as a side effect) get the number of bytes:
      double dOffsetInSeconds = (double)offsetInSeconds;
      u_int64_t numBytes;
      subsession->seekStream(fOurSessionId, streamToken, dOffsetInSeconds, (double)durationInSeconds, numBytes);
      unsigned numTSBytesToStream = (unsigned)numBytes;
      
      if (numTSBytesToStream == 0) {
        // For some reason, we do not know the size of the requested range.  We can't handle this request:
        handleHTTPCmd_notSupported();
        break;
      }
      
      // Construct our response:
      snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	       "HTTP/1.1 200 OK\r\n"
	       "%s"
	       "Server: RTSP Streaming Media v%s\r\n"
	       "%s"
	       "Content-Length: %d\r\n"
	       "Content-Type: text/plain; charset=ISO-8859-1\r\n"
	       "\r\n",
	       dateHeader(),
	       LIVEMEDIA_LIBRARY_VERSION_STRING,
	       lastModifiedHeader(streamName),
	       numTSBytesToStream);
      // Send the response now, because we're about to add more data (from the source):
      send(fClientOutputSocket, (char const*)fResponseBuffer, strlen((char*)fResponseBuffer), 0);
      fResponseBuffer[0] = '\0'; // We've already sent the response.  This tells the calling code not to send it again.
      
      // Ask the media source to deliver - to the TCP sink - the desired data:
      FramedSource* mediaSource = subsession->getStreamSource(streamToken);
      if (mediaSource != NULL) {
        if (fTCPSink == NULL) fTCPSink = TCPStreamSink::createNew(envir(), fClientOutputSocket);
        fTCPSink->startPlaying(*mediaSource, afterStreaming, this);
      }
    } while(0);

    delete[] streamName;
    return;
  } while (0);

  // "urlSuffix" does not end with "?segment=<offset-in-seconds>,<duration-in-seconds>".
  // Construct and send a playlist that describes segments from the specified file.

  // First, make sure that the named file exists, and is streamable:
  ServerMediaSession* session = fOurServer.lookupServerMediaSession(urlSuffix);
  if (session == NULL) {
    handleHTTPCmd_notFound();
    return;
  }

  // To be able to construct a playlist for the requested file, we need to know its duration:
  float duration = session->duration();
  if (duration <= 0.0) {
    // We can't handle this request:
    handleHTTPCmd_notSupported();
    return;
  }

  // Now, construct the playlist.  It will consist of a prefix, one or more media file specifications, and a suffix:
  unsigned const maxIntLen = 10; // >= the maximum possible strlen() of an integer in the playlist
  char const* const playlistPrefixFmt =
    "#EXTM3U\r\n"
    "#EXT-X-ALLOW-CACHE:YES\r\n"
    "#EXT-X-MEDIA-SEQUENCE:0\r\n"
    "#EXT-X-TARGETDURATION:%d\r\n";
  unsigned const playlistPrefixFmt_maxLen = strlen(playlistPrefixFmt) + maxIntLen;

  char const* const playlistMediaFileSpecFmt =
    "#EXTINF:%d,\r\n"
    "%s?segment=%d,%d\r\n";
  unsigned const playlistMediaFileSpecFmt_maxLen = strlen(playlistMediaFileSpecFmt) + maxIntLen + strlen(urlSuffix) + 2*maxIntLen;

  char const* const playlistSuffixFmt =
    "#EXT-X-ENDLIST\r\n";
  unsigned const playlistSuffixFmt_maxLen = strlen(playlistSuffixFmt);

  // Figure out the 'target duration' that will produce a playlist that will fit in our response buffer.  (But make it at least 10s.)
  unsigned const playlistMaxSize = 10000;
  unsigned const mediaFileSpecsMaxSize = playlistMaxSize - (playlistPrefixFmt_maxLen + playlistSuffixFmt_maxLen);
  unsigned const maxNumMediaFileSpecs = mediaFileSpecsMaxSize/playlistMediaFileSpecFmt_maxLen;

  unsigned targetDuration = (unsigned)(duration/maxNumMediaFileSpecs + 1);
  if (targetDuration < 10) targetDuration = 10;

  char* playlist = new char[playlistMaxSize];
  char* s = playlist;
  sprintf(s, playlistPrefixFmt, targetDuration);
  s += strlen(s);

  unsigned durSoFar = 0;
  while (1) {
    unsigned dur = targetDuration < duration ? targetDuration : (unsigned)duration;
    duration -= dur;
    sprintf(s, playlistMediaFileSpecFmt, dur, urlSuffix, durSoFar, dur);
    s += strlen(s);
    if (duration < 1.0) break;

    durSoFar += dur;
  }

  sprintf(s, playlistSuffixFmt);
  s += strlen(s);
  unsigned playlistLen = s - playlist;

  // Construct our response:
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "HTTP/1.1 200 OK\r\n"
	   "%s"
	   "Server: RTSP Streaming Media v%s\r\n"
	   "%s"
	   "Content-Length: %d\r\n"
	   "Content-Type: application/vnd.apple.mpegurl\r\n"
	   "\r\n",
	   dateHeader(),
	   LIVEMEDIA_LIBRARY_VERSION_STRING,
	   lastModifiedHeader(urlSuffix),
	   playlistLen);

  // Send the response header now, because we're about to add more data (the playlist):
  send(fClientOutputSocket, (char const*)fResponseBuffer, strlen((char*)fResponseBuffer), 0);
  fResponseBuffer[0] = '\0'; // We've already sent the response.  This tells the calling code not to send it again.

  // Then, send the playlist.  Because it's large, we don't do so using "send()", because that might not send it all at once.
  // Instead, we stream the playlist over the TCP socket:
  if (fPlaylistSource != NULL) { // sanity check
    if (fTCPSink != NULL) fTCPSink->stopPlaying();
    Medium::close(fPlaylistSource);
  }
  fPlaylistSource = ByteStreamMemoryBufferSource::createNew(envir(), (u_int8_t*)playlist, playlistLen);
  if (fTCPSink == NULL) fTCPSink = TCPStreamSink::createNew(envir(), fClientOutputSocket);
  fTCPSink->startPlaying(*fPlaylistSource, afterStreaming, this);
}

void RTSPServerSupportingHTTPStreaming::RTSPClientSessionSupportingHTTPStreaming::afterStreaming(void* clientData) {
   RTSPServerSupportingHTTPStreaming::RTSPClientSessionSupportingHTTPStreaming* clientSession
    = (RTSPServerSupportingHTTPStreaming::RTSPClientSessionSupportingHTTPStreaming*)clientData;
   delete clientSession;
}
