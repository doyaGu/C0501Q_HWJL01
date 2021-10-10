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
// RTP sink for H.264 video (RFC 3984)
// Implementation

#include "H264VideoRTPSink.hh"
#include "H264VideoStreamFramer.hh"
#include "Base64.hh"
#include "H264VideoRTPSource.hh" // for "parseSPropParameterSets()"

////////// H264VideoRTPSink implementation //////////

H264VideoRTPSink
::H264VideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat,
		   u_int8_t const* sps, unsigned spsSize, u_int8_t const* pps, unsigned ppsSize)
  : VideoRTPSink(env, RTPgs, rtpPayloadFormat, 90000, "H264"),
    fOurFragmenter(NULL), fFmtpSDPLine(NULL) {
  if (sps != NULL) {
    fSPSSize = spsSize;
    fSPS = new u_int8_t[fSPSSize];
    memmove(fSPS, sps, fSPSSize);
  } else {
    fSPSSize = 0;
    fSPS = NULL;
  }
  if (pps != NULL) {
    fPPSSize = ppsSize;
    fPPS = new u_int8_t[fPPSSize];
    memmove(fPPS, pps, fPPSSize);
  } else {
    fPPSSize = 0;
    fPPS = NULL;
  }
}

H264VideoRTPSink::~H264VideoRTPSink() {
  fSource = fOurFragmenter; // hack: in case "fSource" had gotten set to NULL before we were called
  delete[] fFmtpSDPLine;
  delete[] fSPS; delete[] fPPS;
  stopPlaying(); // call this now, because we won't have our 'FUA fragmenter' when the base class destructor calls it later.

  // Close our 'FUA fragmenter' as well:
  Medium::close(fOurFragmenter);
  fSource = NULL; // for the base class destructor, which gets called next
}

H264VideoRTPSink*
H264VideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat) {
  return new H264VideoRTPSink(env, RTPgs, rtpPayloadFormat);
}

H264VideoRTPSink*
H264VideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat,
			    u_int8_t const* sps, unsigned spsSize, u_int8_t const* pps, unsigned ppsSize) {
  return new H264VideoRTPSink(env, RTPgs, rtpPayloadFormat, sps, spsSize, pps, ppsSize);
}

H264VideoRTPSink*
H264VideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat,
			    char const* sPropParameterSetsStr) {
  u_int8_t* sps = NULL; unsigned spsSize = 0;
  u_int8_t* pps = NULL; unsigned ppsSize = 0;

  unsigned numSPropRecords;
  SPropRecord* sPropRecords = parseSPropParameterSets(sPropParameterSetsStr, numSPropRecords);
  for (unsigned i = 0; i < numSPropRecords; ++i) {
    if (sPropRecords[i].sPropLength == 0) continue; // bad data
    u_int8_t nal_unit_type = (sPropRecords[i].sPropBytes[0])&0x1F;
    if (nal_unit_type == 7/*SPS*/) {
      sps = sPropRecords[i].sPropBytes;
      spsSize = sPropRecords[i].sPropLength;
    } else if (nal_unit_type == 8/*PPS*/) {
      pps = sPropRecords[i].sPropBytes;
      ppsSize = sPropRecords[i].sPropLength;
    }
  }

  H264VideoRTPSink* result = new H264VideoRTPSink(env, RTPgs, rtpPayloadFormat, sps, spsSize, pps, ppsSize);
  delete[] sPropRecords;

  return result;
}

Boolean H264VideoRTPSink::sourceIsCompatibleWithUs(MediaSource& source) {
  // Our source must be an appropriate framer:
  return source.isH264VideoStreamFramer();
}

Boolean H264VideoRTPSink::continuePlaying() {
  // First, check whether we have a 'fragmenter' class set up yet.
  // If not, create it now:
  if (fOurFragmenter == NULL) {
#if defined(CFG_DEMOD_ENABLE)
    fOurFragmenter = new H264FUAFragmenter(envir(), fSource, 660 * 1024,
					   ourMaxPacketSize() - 12/*RTP hdr size*/);
#else
    fOurFragmenter = new H264FUAFragmenter(envir(), fSource, OutPacketBuffer::maxSize,
					   ourMaxPacketSize() - 12/*RTP hdr size*/);
#endif
  } else {
    fOurFragmenter->reassignInputSource(fSource);
  }
  fSource = fOurFragmenter;

  // Then call the parent class's implementation:
  return MultiFramedRTPSink::continuePlaying();
}

void H264VideoRTPSink::doSpecialFrameHandling(unsigned /*fragmentationOffset*/,
					      unsigned char* /*frameStart*/,
					      unsigned /*numBytesInFrame*/,
					      struct timeval framePresentationTime,
					      unsigned /*numRemainingBytes*/) {
  // Set the RTP 'M' (marker) bit iff
  // 1/ The most recently delivered fragment was the end of (or the only fragment of) an NAL unit, and
  // 2/ This NAL unit was the last NAL unit of an 'access unit' (i.e. video frame).
  if (fOurFragmenter != NULL) {
    H264VideoStreamFramer* framerSource
      = (H264VideoStreamFramer*)(fOurFragmenter->inputSource());
    // This relies on our fragmenter's source being a "H264VideoStreamFramer".
    if (fOurFragmenter->lastFragmentCompletedNALUnit()
	&& framerSource != NULL && framerSource->pictureEndMarker()) {
      setMarkerBit();
      framerSource->pictureEndMarker() = False;
    }
  }

  setTimestamp(framePresentationTime);
}

Boolean H264VideoRTPSink
::frameCanAppearAfterPacketStart(unsigned char const* /*frameStart*/,
				 unsigned /*numBytesInFrame*/) const {
  return False;
}

char const* H264VideoRTPSink::auxSDPLine() {
  // Generate a new "a=fmtp:" line each time, using our SPS and PPS (if we have them),
  // otherwise parameters from our framer source (in case they've changed since the last time that
  // we were called):
  u_int8_t* sps = fSPS; unsigned spsSize = fSPSSize;
  u_int8_t* pps = fPPS; unsigned ppsSize = fPPSSize;
  if (sps == NULL || pps == NULL) {
    // We need to get SPS and PPS from our framer source:
    if (fOurFragmenter == NULL) return NULL; // we don't yet have a fragmenter (and therefore not a source)
    H264VideoStreamFramer* framerSource = (H264VideoStreamFramer*)(fOurFragmenter->inputSource());
    if (framerSource == NULL) return NULL; // we don't yet have a source

    framerSource->getSPSandPPS(sps, spsSize, pps, ppsSize);
    if (sps == NULL || pps == NULL) return NULL; // our source isn't ready
  }

  u_int32_t profile_level_id;
  if (spsSize < 4) { // sanity check
    profile_level_id = 0;
  } else {
    profile_level_id = (sps[1]<<16)|(sps[2]<<8)|sps[3]; // profile_idc|constraint_setN_flag|level_idc
  }
  
  // Set up the "a=fmtp:" SDP line for this stream:
  char* sps_base64 = base64Encode((char*)sps, spsSize);
  char* pps_base64 = base64Encode((char*)pps, ppsSize);
  char const* fmtpFmt =
    "a=fmtp:%d packetization-mode=1"
    ";profile-level-id=%06X"
    ";sprop-parameter-sets=%s,%s\r\n";
  unsigned fmtpFmtSize = strlen(fmtpFmt)
    + 3 /* max char len */
    + 6 /* 3 bytes in hex */
    + strlen(sps_base64) + strlen(pps_base64);
  char* fmtp = new char[fmtpFmtSize];
  sprintf(fmtp, fmtpFmt,
          rtpPayloadType(),
	  profile_level_id,
          sps_base64, pps_base64);
  delete[] sps_base64;
  delete[] pps_base64;

  delete[] fFmtpSDPLine; fFmtpSDPLine = fmtp;
  return fFmtpSDPLine;
}


////////// H264FUAFragmenter implementation //////////

H264FUAFragmenter::H264FUAFragmenter(UsageEnvironment& env,
				     FramedSource* inputSource,
				     unsigned inputBufferMax,
				     unsigned maxOutputPacketSize)
  : FramedFilter(env, inputSource),
    fInputBufferSize(inputBufferMax+1), fMaxOutputPacketSize(maxOutputPacketSize),
    fNumValidDataBytes(1), fCurDataOffset(1), fSaveNumTruncatedBytes(0),
    fLastFragmentCompletedNALUnit(True) {
#if !defined(CFG_ENABLE_H264_SRC_SINK_SHARE_BUF)
  fInputBuffer = new unsigned char[fInputBufferSize];
#else
  fInputBuffer = NULL;
#endif
}

H264FUAFragmenter::~H264FUAFragmenter() {
#if !defined(CFG_ENABLE_H264_SRC_SINK_SHARE_BUF)
  delete[] fInputBuffer;
#endif
  detachInputSource(); // so that the subsequent ~FramedFilter() doesn't delete it
}

void H264FUAFragmenter::doGetNextFrame() {
  if (fNumValidDataBytes == 1) {
    // We have no NAL unit data currently in the buffer.  Read a new one:
#if !defined(CFG_ENABLE_H264_SRC_SINK_SHARE_BUF)
    fInputSource->getNextFrame(&fInputBuffer[1], fInputBufferSize - 1,
			       afterGettingFrame, this,
			       FramedSource::handleClosure, this);
#else
    fInputSource->getNextFramePtr(&fInputBuffer,
		         afterGettingFrame, this,
		         FramedSource::handleClosure, this);
#endif
  } else {
    // We have NAL unit data in the buffer.  There are three cases to consider:
    // 1. There is a new NAL unit in the buffer, and it's small enough to deliver
    //    to the RTP sink (as is).
    // 2. There is a new NAL unit in the buffer, but it's too large to deliver to
    //    the RTP sink in its entirety.  Deliver the first fragment of this data,
    //    as a FU-A packet, with one extra preceding header byte.
    // 3. There is a NAL unit in the buffer, and we've already delivered some
    //    fragment(s) of this.  Deliver the next fragment of this data,
    //    as a FU-A packet, with two extra preceding header bytes.

    if (fMaxSize < fMaxOutputPacketSize) { // shouldn't happen
      envir() << "H264FUAFragmenter::doGetNextFrame(): fMaxSize ("
	      << fMaxSize << ") is smaller than expected\n";
    } else {
      fMaxSize = fMaxOutputPacketSize;
    }

    fLastFragmentCompletedNALUnit = True; // by default
    if (fCurDataOffset == 1) { // case 1 or 2
      if (fNumValidDataBytes - 1 <= fMaxSize) { // case 1
	    memmove(fTo, &fInputBuffer[1], fNumValidDataBytes - 1);
	    fFrameSize = fNumValidDataBytes - 1;
	    fCurDataOffset = fNumValidDataBytes;
      } else { // case 2
	    // We need to send the NAL unit data as FU-A packets.  Deliver the first
	    // packet now.  Note that we add FU indicator and FU header bytes to the front
	    // of the packet (reusing the existing NAL header byte for the FU header).
	    fInputBuffer[0] = (fInputBuffer[1] & 0xE0) | 28; // FU indicator
	    fInputBuffer[1] = 0x80 | (fInputBuffer[1] & 0x1F); // FU header (with S bit)
	    memmove(fTo, fInputBuffer, fMaxSize);
	    fFrameSize = fMaxSize;
	    fCurDataOffset += fMaxSize - 1;
	    fLastFragmentCompletedNALUnit = False;
      }
    } else { // case 3
      // We are sending this NAL unit data as FU-A packets.  We've already sent the
      // first packet (fragment).  Now, send the next fragment.  Note that we add
      // FU indicator and FU header bytes to the front.  (We reuse these bytes that
      // we already sent for the first fragment, but clear the S bit, and add the E
      // bit if this is the last fragment.)
      fInputBuffer[fCurDataOffset-2] = fInputBuffer[0]; // FU indicator
      fInputBuffer[fCurDataOffset-1] = fInputBuffer[1]&~0x80; // FU header (no S bit)
      unsigned numBytesToSend = 2 + fNumValidDataBytes - fCurDataOffset;
      if (numBytesToSend > fMaxSize) {
	    // We can't send all of the remaining data this time:
	    numBytesToSend = fMaxSize;
	    fLastFragmentCompletedNALUnit = False;
      } else {
	    // This is the last fragment:
	    fInputBuffer[fCurDataOffset-1] |= 0x40; // set the E bit in the FU header
	    fNumTruncatedBytes = fSaveNumTruncatedBytes;
      }
      memmove(fTo, &fInputBuffer[fCurDataOffset-2], numBytesToSend);
      fFrameSize = numBytesToSend;
      fCurDataOffset += numBytesToSend - 2;
    }

    if (fCurDataOffset >= fNumValidDataBytes) {
      // We're done with this data.  Reset the pointers for receiving new data:
      fNumValidDataBytes = fCurDataOffset = 1;
    }

    // Complete delivery to the client:
    FramedSource::afterGetting(this);
  }
}

void H264FUAFragmenter::afterGettingFrame(void* clientData, unsigned frameSize,
					  unsigned numTruncatedBytes,
					  struct timeval presentationTime,
					  unsigned durationInMicroseconds) {
  H264FUAFragmenter* fragmenter = (H264FUAFragmenter*)clientData;
  fragmenter->afterGettingFrame1(frameSize, numTruncatedBytes, presentationTime,
				 durationInMicroseconds);
}

void H264FUAFragmenter::afterGettingFrame1(unsigned frameSize,
					   unsigned numTruncatedBytes,
					   struct timeval presentationTime,
					   unsigned durationInMicroseconds) {
  fNumValidDataBytes += frameSize;
  fSaveNumTruncatedBytes = numTruncatedBytes;
  fPresentationTime = presentationTime;
  fDurationInMicroseconds = durationInMicroseconds;

  // Deliver data to the client:
  doGetNextFrame();
}
