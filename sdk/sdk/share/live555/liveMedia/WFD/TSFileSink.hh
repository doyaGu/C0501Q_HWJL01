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
// "liveMedia"
// Copyright (c) 1996-2012 Live Networks, Inc.  All rights reserved.
// A sink that generates an AVI file from a composite media session
// C++ header

#ifndef _TS_FILE_SINK_HH
#define _TS_FILE_SINK_HH

#ifndef _MEDIA_SESSION_HH
#include "MediaSession.hh"
#endif

class TSFileSink: public Medium {
public:
  static TSFileSink* createNew(UsageEnvironment& env,
				MediaSession& inputSession,
				char const* outputFileName,
				unsigned bufferSize = 20000,
				Boolean packetLossCompensate = False);

  typedef void (afterPlayingFunc)(void* clientData);
  Boolean startPlaying(afterPlayingFunc* afterFunc,
                       void* afterClientData);

  unsigned numActiveSubsessions() const { return fNumSubsessions; }

private:
  TSFileSink(UsageEnvironment& env, MediaSession& inputSession,
	      char const* outputFileName, unsigned bufferSize,
		  Boolean packetLossCompensate);
      // called only by createNew()
  virtual ~TSFileSink();

  Boolean continuePlaying();
  static void afterGettingFrame(void* clientData, unsigned frameSize,
				unsigned numTruncatedBytes,
				struct timeval presentationTime,
				unsigned durationInMicroseconds);
  static void onSourceClosure(void* clientData);
  void onSourceClosure1();
  static void onRTCPBye(void* clientData);
  void completeOutputFile();

private:
  friend class TSSubsessionIOState;
  MediaSession& fInputSession;
  FILE* fOutFid;
  unsigned fBufferSize;
  Boolean fPacketLossCompensate;
  Boolean fAreCurrentlyBeingPlayed;
  afterPlayingFunc* fAfterFunc;
  void* fAfterClientData;
  unsigned fNumSubsessions;
  unsigned fNumBytesWritten;
  struct timeval fStartTime;
  Boolean fHaveCompletedOutputFile;

};

#endif
