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
// A subclass of "RTSPServer" that creates "ServerMediaSession"s on demand,
// based on whether or not the specified stream name exists as a file
// Header file

#ifndef _WIFI_DISPLAY_SOURCE_HH
#define _WIFI_DISPLAY_SOURCE_HH

#ifndef _RTSP_SERVER_SUPPORTING_HTTP_STREAMING_HH
#include "RTSPServerSupportingHTTPStreaming.hh"
#endif

class WifiDisplaySource: public RTSPServerSupportingHTTPStreaming {
public:
  static WifiDisplaySource* createNew(UsageEnvironment& env, Port ourPort,
				      UserAuthenticationDatabase* authDatabase,
				      unsigned reclamationTestSeconds = 65);

protected:
  WifiDisplaySource(UsageEnvironment& env, int ourSocket, Port ourPort,
		    UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds);
  // called only by createNew();
  virtual ~WifiDisplaySource();

protected: // redefined virtual functions
  virtual ServerMediaSession* lookupServerMediaSession(char const* streamName);
  virtual RTSPClientSession* createNewClientSession(unsigned sessionId, int clientSocket, struct sockaddr_in clientAddr);

public: // should be protected, but some old compilers complain otherwise
  class WFDClientSession: public RTSPServerSupportingHTTPStreaming::RTSPClientSessionSupportingHTTPStreaming {
  public:
    WFDClientSession(RTSPServer& ourServer, unsigned sessionId,
                     int clientSocket, struct sockaddr_in clientAddr);
    virtual ~WFDClientSession();
    void sendM1();
    void sendM3();
    void sendM4();
    void sendM5();
    void sendM16();

  protected: // redefined virtual functions
  typedef void (responseHandler)(int resultCode, char* resultString);
				 

    class RequestRecord {
    public:
      RequestRecord(unsigned cseq, char const* commandName, responseHandler* handler, char const* contentStr = NULL);		  
      virtual ~RequestRecord();

      unsigned& cseq() { return fCSeq; }
      char const* commandName() const { return fCommandName; }
      char* contentStr() const { return fContentStr; }
      responseHandler*& handler() { return fHandler; }

    private:
      unsigned fCSeq;
      char const* fCommandName;
      char* fContentStr;
      responseHandler* fHandler;
    };

  protected:
    virtual void handleCmd_Response(unsigned responseCode, char const* responseStr);
	virtual void handleCmd_OPTIONS(char const* cseq);

    unsigned sendRequest(RequestRecord* request);
	void handleRequestError(RequestRecord* request);

  private:
    RequestRecord* fRequest;
	Authenticator* fAuthenticator;
    unsigned fCSeq;

  };
};

#endif
