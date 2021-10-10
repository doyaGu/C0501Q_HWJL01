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
// Framed File Sources
// C++ header

#ifndef _ITE_PES_SOURCE_HH
#define _ITE_PES_SOURCE_HH

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

class ItePesSource: public FramedSource {
public:
  static ItePesSource* createNew(UsageEnvironment& env,
					 uint32_t demodId,
                     uint32_t serviceId);
  uint32_t getRemainLen();
  virtual void doGetNextFrame();
  virtual void doStopGettingFrames();
  static void retryLater(void* firstArg);
protected:

  ItePesSource(UsageEnvironment& env, uint32_t demodId, uint32_t serviceId); // abstract base class
  virtual ~ItePesSource();
  // redefined virtual functions:
  virtual Boolean isH264VideoStreamFramer() const;
private:
#define PES_FIND_START_CODE_STATE 0
#define PES_FIND_PES_BOUNDARY 1
#define PES_FIND_NAL_BOUNDARY 2

  uint32_t    fDemodId;
  uint32_t    fServiceId;
  uint8_t*    fpStartPtr;
  int32_t     fRemainLen;
  uint8_t*    fpOutputPtr;
  uint8_t*    fpParsePtr;
  int32_t     fBufferLen;
  int32_t     fState;
  int32_t     fbFirstTime;
  int32_t     fDuration;
  struct timeval fStartT;
  unsigned long long fPrevTime;
};

#endif
