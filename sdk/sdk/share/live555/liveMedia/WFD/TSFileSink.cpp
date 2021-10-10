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
// A sink that generates an TS file from a composite media session
// Implementation

#include "TSFileSink.hh"
#include "InputFile.hh"
#include "OutputFile.hh"
#include "GroupsockHelper.hh"

#define fourChar(x,y,z,w) ( ((w)<<24)|((z)<<16)|((y)<<8)|(x) )/*little-endian*/

////////// TSSubsessionIOState ///////////
// A structure used to represent the I/O state of each input 'subsession':

class SubsessionBuffer {
public:
  SubsessionBuffer(unsigned bufferSize)
    : fBufferSize(bufferSize) {
    reset();
    fData = new unsigned char[bufferSize];
  }
  virtual ~SubsessionBuffer() { delete[] fData; }
  void reset() { fBytesInUse = 0; }
  void addBytes(unsigned numBytes) { fBytesInUse += numBytes; }

  unsigned char* dataStart() { return &fData[0]; }
  unsigned char* dataEnd() { return &fData[fBytesInUse]; }
  unsigned bytesInUse() const { return fBytesInUse; }
  unsigned bytesAvailable() const { return fBufferSize - fBytesInUse; }

  void setPresentationTime(struct timeval const& presentationTime) {
    fPresentationTime = presentationTime;
  }
  struct timeval const& presentationTime() const {return fPresentationTime;}

private:
  unsigned fBufferSize;
  struct timeval fPresentationTime;
  unsigned char* fData;
  unsigned fBytesInUse;
};

class TSSubsessionIOState {
public:
  TSSubsessionIOState(TSFileSink& sink, MediaSubsession& subsession);
  virtual ~TSSubsessionIOState();

  void setTSstate(unsigned subsessionIndex);
  void setFinalTSstate();

  void afterGettingFrame(unsigned packetDataSize,
			 struct timeval presentationTime);
  void onSourceClosure();

  UsageEnvironment& envir() const { return fOurSink.envir(); }

public:
  SubsessionBuffer *fBuffer, *fPrevBuffer;
  TSFileSink& fOurSink;
  MediaSubsession& fOurSubsession;

  unsigned short fLastPacketRTPSeqNum;
  Boolean fOurSourceIsActive;
  struct timeval fPrevPresentationTime;
  unsigned fMaxBytesPerSecond;
  unsigned fNumFrames;

private:
  void useFrame(SubsessionBuffer& buffer);
};


////////// TSFileSink implementation //////////

TSFileSink::TSFileSink(UsageEnvironment& env,
			 MediaSession& inputSession,
			 char const* outputFileName,
			 unsigned bufferSize,
			 Boolean packetLossCompensate)
  : Medium(env), fInputSession(inputSession),
    fBufferSize(bufferSize), fPacketLossCompensate(packetLossCompensate),
    fAreCurrentlyBeingPlayed(False), fNumSubsessions(0), fNumBytesWritten(0),
    fHaveCompletedOutputFile(False) {
  fOutFid = OpenOutputFile(env, outputFileName);
  if (fOutFid == NULL) return;

  // Set up I/O state for each input subsession:
  MediaSubsessionIterator iter(fInputSession);
  MediaSubsession* subsession;
  while ((subsession = iter.next()) != NULL) {
    // Ignore subsessions without a data source:
    FramedSource* subsessionSource = subsession->readSource();
    if (subsessionSource == NULL) continue;

    TSSubsessionIOState* ioState
      = new TSSubsessionIOState(*this, *subsession);
    subsession->miscPtr = (void*)ioState;

    // Also set a 'BYE' handler for this subsession's RTCP instance:
    if (subsession->rtcpInstance() != NULL) {
      subsession->rtcpInstance()->setByeHandler(onRTCPBye, ioState);
    }

    ++fNumSubsessions;
  }

}

TSFileSink::~TSFileSink() {
  completeOutputFile();

  // Then, delete each active "TSSubsessionIOState":
  MediaSubsessionIterator iter(fInputSession);
  MediaSubsession* subsession;
  while ((subsession = iter.next()) != NULL) {
    TSSubsessionIOState* ioState
      = (TSSubsessionIOState*)(subsession->miscPtr);
    if (ioState == NULL) continue;

    delete ioState;
  }

  // Finally, close our output file:
  CloseOutputFile(fOutFid);
}

TSFileSink* TSFileSink
::createNew(UsageEnvironment& env, MediaSession& inputSession,
	    char const* outputFileName,
	    unsigned bufferSize,
        Boolean packetLossCompensate) {
  TSFileSink* newSink =
    new TSFileSink(env, inputSession, outputFileName, bufferSize,
		    packetLossCompensate);
  if (newSink == NULL || newSink->fOutFid == NULL) {
    Medium::close(newSink);
    return NULL;
  }

  return newSink;
}

Boolean TSFileSink::startPlaying(afterPlayingFunc* afterFunc,
				  void* afterClientData) {
  // Make sure we're not already being played:
  if (fAreCurrentlyBeingPlayed) {
    envir().setResultMsg("This sink has already been played");
    return False;
  }

  fAreCurrentlyBeingPlayed = True;
  fAfterFunc = afterFunc;
  fAfterClientData = afterClientData;

  return continuePlaying();
}

Boolean TSFileSink::continuePlaying() {
  // Run through each of our input session's 'subsessions',
  // asking for a frame from each one:
  Boolean haveActiveSubsessions = False;
  MediaSubsessionIterator iter(fInputSession);
  MediaSubsession* subsession;
  while ((subsession = iter.next()) != NULL) {
    FramedSource* subsessionSource = subsession->readSource();
    if (subsessionSource == NULL) continue;

    if (subsessionSource->isCurrentlyAwaitingData()) continue;

    TSSubsessionIOState* ioState
      = (TSSubsessionIOState*)(subsession->miscPtr);
    if (ioState == NULL) continue;

    haveActiveSubsessions = True;
    unsigned char* toPtr = ioState->fBuffer->dataEnd();
    unsigned toSize = ioState->fBuffer->bytesAvailable();
    subsessionSource->getNextFrame(toPtr, toSize,
				   afterGettingFrame, ioState,
				   onSourceClosure, ioState);
  }
  if (!haveActiveSubsessions) {
    envir().setResultMsg("No subsessions are currently active");
    return False;
  }

  return True;
}

void TSFileSink
::afterGettingFrame(void* clientData, unsigned packetDataSize,
		    unsigned numTruncatedBytes,
		    struct timeval presentationTime,
		    unsigned /*durationInMicroseconds*/) {
  TSSubsessionIOState* ioState = (TSSubsessionIOState*)clientData;
  if (numTruncatedBytes > 0) {
    ioState->envir() << "TSFileSink::afterGettingFrame(): The input frame data was too large for our buffer.  "
		     << numTruncatedBytes
		     << " bytes of trailing data was dropped!  Correct this by increasing the \"bufferSize\" parameter in the \"createNew()\" call.\n";
  }
  ioState->afterGettingFrame(packetDataSize, presentationTime);
}

void TSFileSink::onSourceClosure(void* clientData) {
  TSSubsessionIOState* ioState = (TSSubsessionIOState*)clientData;
  ioState->onSourceClosure();
}

void TSFileSink::onSourceClosure1() {
  // Check whether *all* of the subsession sources have closed.
  // If not, do nothing for now:
  MediaSubsessionIterator iter(fInputSession);
  MediaSubsession* subsession;
  while ((subsession = iter.next()) != NULL) {
    TSSubsessionIOState* ioState
      = (TSSubsessionIOState*)(subsession->miscPtr);
    if (ioState == NULL) continue;

    if (ioState->fOurSourceIsActive) return; // this source hasn't closed
  }

  completeOutputFile();

  // Call our specified 'after' function:
  if (fAfterFunc != NULL) {
    (*fAfterFunc)(fAfterClientData);
  }
}

void TSFileSink::onRTCPBye(void* clientData) {
  TSSubsessionIOState* ioState = (TSSubsessionIOState*)clientData;

  struct timeval timeNow;
  gettimeofday(&timeNow, NULL);
  unsigned secsDiff
    = timeNow.tv_sec - ioState->fOurSink.fStartTime.tv_sec;

  MediaSubsession& subsession = ioState->fOurSubsession;
  ioState->envir() << "Received RTCP \"BYE\" on \""
		   << subsession.mediumName()
		   << "/" << subsession.codecName()
		   << "\" subsession (after "
		   << secsDiff << " seconds)\n";

  // Handle the reception of a RTCP "BYE" as if the source had closed:
  ioState->onSourceClosure();
}

void TSFileSink::completeOutputFile() {
  if (fHaveCompletedOutputFile || fOutFid == NULL) return;

  // Update various TS 'size' fields to take account of the codec data that
  // we've now written to the file:
  unsigned maxBytesPerSecond = 0;
  unsigned numVideoFrames = 0;
  unsigned numAudioFrames = 0;

  //// Subsession-specific fields:
  MediaSubsessionIterator iter(fInputSession);
  MediaSubsession* subsession;
  while ((subsession = iter.next()) != NULL) {
    TSSubsessionIOState* ioState
      = (TSSubsessionIOState*)(subsession->miscPtr);
    if (ioState == NULL) continue;

    maxBytesPerSecond += ioState->fMaxBytesPerSecond;
  }

  // We're done:
  fHaveCompletedOutputFile = True;
}


////////// TSSubsessionIOState implementation ///////////

TSSubsessionIOState::TSSubsessionIOState(TSFileSink& sink,
				     MediaSubsession& subsession)
  : fOurSink(sink), fOurSubsession(subsession),
    fMaxBytesPerSecond(0), fNumFrames(0) {
  fBuffer = new SubsessionBuffer(fOurSink.fBufferSize);
  fPrevBuffer = sink.fPacketLossCompensate
    ? new SubsessionBuffer(fOurSink.fBufferSize) : NULL;

  FramedSource* subsessionSource = subsession.readSource();
  fOurSourceIsActive = subsessionSource != NULL;

  fPrevPresentationTime.tv_sec = 0;
  fPrevPresentationTime.tv_usec = 0;
}

TSSubsessionIOState::~TSSubsessionIOState() {
  delete fBuffer; delete fPrevBuffer;
}

void TSSubsessionIOState::setTSstate(unsigned subsessionIndex) {
}

void TSSubsessionIOState::afterGettingFrame(unsigned packetDataSize,
					  struct timeval presentationTime) {
  // Begin by checking whether there was a gap in the RTP stream.
  // If so, try to compensate for this (if desired):
  unsigned short rtpSeqNum
    = fOurSubsession.rtpSource()->curPacketRTPSeqNum();
  if (fOurSink.fPacketLossCompensate && fPrevBuffer->bytesInUse() > 0) {
    short seqNumGap = rtpSeqNum - fLastPacketRTPSeqNum;
    for (short i = 1; i < seqNumGap; ++i) {
      // Insert a copy of the previous frame, to compensate for the loss:
      useFrame(*fPrevBuffer);
    }
  }
  fLastPacketRTPSeqNum = rtpSeqNum;

  // Now, continue working with the frame that we just got
  if (fBuffer->bytesInUse() == 0) {
    fBuffer->setPresentationTime(presentationTime);
  }
  fBuffer->addBytes(packetDataSize);

  useFrame(*fBuffer);
  if (fOurSink.fPacketLossCompensate) {
    // Save this frame, in case we need it for recovery:
    SubsessionBuffer* tmp = fPrevBuffer; // assert: != NULL
    fPrevBuffer = fBuffer;
    fBuffer = tmp;
  }
  fBuffer->reset(); // for the next input

  // Now, try getting more frames:
  fOurSink.continuePlaying();
}

void TSSubsessionIOState::useFrame(SubsessionBuffer& buffer) {
  unsigned char* const frameSource = buffer.dataStart();
  unsigned const frameSize = buffer.bytesInUse();
  struct timeval const& presentationTime = buffer.presentationTime();
  if (fPrevPresentationTime.tv_usec != 0||fPrevPresentationTime.tv_sec != 0) {
    int uSecondsDiff
      = (presentationTime.tv_sec - fPrevPresentationTime.tv_sec)*1000000
      + (presentationTime.tv_usec - fPrevPresentationTime.tv_usec);
    if (uSecondsDiff > 0) {
      unsigned bytesPerSecond = (unsigned)((frameSize*1000000.0)/uSecondsDiff);
      if (bytesPerSecond > fMaxBytesPerSecond) {
        fMaxBytesPerSecond = bytesPerSecond;
      }
    }
  }
  fPrevPresentationTime = presentationTime;

  // Write the data into the file:
  fwrite(frameSource, 1, frameSize, fOurSink.fOutFid);
  fOurSink.fNumBytesWritten += frameSize;

  ++fNumFrames;
}

void TSSubsessionIOState::onSourceClosure() {
  fOurSourceIsActive = False;
  fOurSink.onSourceClosure1();
}
