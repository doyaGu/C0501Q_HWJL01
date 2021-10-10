if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(
        -DDEBUG
        )
endif()

if (DEFINED CFG_BUILD_MEDIASTREAMER2)
    add_definitions(
        -DCFG_BUILD_MEDIASTREAMER2
    )
endif()

include_directories(
    BasicUsageEnvironment/include
    groupsock/include
    liveMedia/include
    liveMedia/WFD
    UsageEnvironment/include
    ${PROJECT_SOURCE_DIR}/sdk/share/ffmpeg
    ${PROJECT_SOURCE_DIR}/sdk/share/mediastreamer2
    )

# BasicUsageEnvironment for system env
FILE(GLOB BasicUsageEnvironment
    BasicUsageEnvironment/BasicHashTable.cpp
    BasicUsageEnvironment/BasicTaskScheduler.cpp
    BasicUsageEnvironment/BasicTaskScheduler0.cpp
    BasicUsageEnvironment/BasicUsageEnvironment.cpp
    BasicUsageEnvironment/BasicUsageEnvironment0.cpp
    BasicUsageEnvironment/DelayQueue.cpp
    BasicUsageEnvironment/include/BasicHashTable.hh
    BasicUsageEnvironment/include/BasicUsageEnvironment.hh
    BasicUsageEnvironment/include/BasicUsageEnvironment0.hh
    BasicUsageEnvironment/include/BasicUsageEnvironment_version.hh
    BasicUsageEnvironment/include/DelayQueue.hh
    BasicUsageEnvironment/include/HandlerSet.hh
    )
source_group("BasicUsageEnvironment" FILES ${BasicUsageEnvironment})

# UsageEnvironment for system env
FILE(GLOB UsageEnvironment
    UsageEnvironment/HashTable.cpp
    UsageEnvironment/strDup.cpp
    UsageEnvironment/UsageEnvironment.cpp
    UsageEnvironment/include/Boolean.hh
    UsageEnvironment/include/HashTable.hh
    UsageEnvironment/include/strDup.hh
    UsageEnvironment/include/UsageEnvironment.hh
    UsageEnvironment/include/UsageEnvironment_version.hh
    )
source_group("UsageEnvironment" FILES ${UsageEnvironment})

# groupsock for socket handle
FILE(GLOB groupsock
    groupsock/GroupEId.cpp
    groupsock/Groupsock.cpp
    groupsock/GroupsockHelper.cpp
    groupsock/inet.c
    groupsock/IOHandlers.cpp
    groupsock/NetAddress.cpp
    groupsock/NetInterface.cpp
    groupsock/include/GroupEId.hh
    groupsock/include/Groupsock.hh
    groupsock/include/GroupsockHelper.hh
    groupsock/include/groupsock_version.hh
    groupsock/include/IOHandlers.hh
    groupsock/include/NetAddress.hh
    groupsock/include/NetCommon.h
    groupsock/include/NetInterface.hh
    groupsock/include/TunnelEncaps.hh
    )
source_group("groupsock" FILES ${groupsock})

if( DEFINED CFG_WFD_SOURCE_DEVICE )
# wfd source device enter pointer
    FILE(GLOB mediaServer
        mediaServer/WFDSourceDevice.cpp
        mediaServer/version.hh
        )
elseif( DEFINED CFG_WFD_SINK_DEVICE )
# wfd sink device enter pointer
    FILE(GLOB mediaServer
        mediaServer/WFDSinkDevice.cpp
        mediaServer/version.hh
        )
else()
# mediaServer enter pointer
    FILE(GLOB mediaServer
        mediaServer/DynamicRTSPServer.cpp
        mediaServer/DynamicRTSPServer.hh
        mediaServer/live555MediaServer.cpp
        mediaServer/version.hh
        )
endif()
source_group("mediaServer" FILES ${mediaServer})

# mediaClient enter pointer
FILE(GLOB mediaClient
    mediaClient/live555MediaClient.cpp
    )
source_group("mediaClient" FILES ${mediaClient})    

# liveMedia for server/client behavior handle
FILE(GLOB liveMedia
    # liveMedia/AACAudioMatroskaFileServerMediaSubsession.cpp
    # liveMedia/AACAudioMatroskaFileServerMediaSubsession.hh
    # liveMedia/AC3AudioFileServerMediaSubsession.cpp
    # liveMedia/AC3AudioMatroskaFileServerMediaSubsession.cpp
    # liveMedia/AC3AudioMatroskaFileServerMediaSubsession.hh
    liveMedia/AC3AudioRTPSink.cpp
    liveMedia/AC3AudioRTPSource.cpp
    liveMedia/AC3AudioStreamFramer.cpp
    # liveMedia/ADTSAudioFileServerMediaSubsession.cpp
    # liveMedia/ADTSAudioFileSource.cpp
    # liveMedia/AMRAudioFileServerMediaSubsession.cpp
    # liveMedia/AMRAudioFileSink.cpp
    # liveMedia/AMRAudioFileSource.cpp
    # liveMedia/AMRAudioRTPSink.cpp
    liveMedia/AMRAudioRTPSource.cpp
	liveMedia/AMRAudioSource.cpp
    liveMedia/AudioInputDevice.cpp
    liveMedia/AudioRTPSink.cpp
    liveMedia/AVIFileSink.cpp
    liveMedia/Base64.cpp
    liveMedia/BasicUDPSink.cpp
    liveMedia/BasicUDPSource.cpp
    liveMedia/BitVector.cpp
    liveMedia/BitVector.hh
    liveMedia/ByteStreamFileSource.cpp
    liveMedia/ByteStreamMemoryBufferSource.cpp
    liveMedia/ByteStreamMultiFileSource.cpp
    liveMedia/DarwinInjector.cpp
    liveMedia/DeviceSource.cpp
    liveMedia/DigestAuthentication.cpp
    # liveMedia/DVVideoFileServerMediaSubsession.cpp
    # liveMedia/DVVideoRTPSink.cpp
    liveMedia/DVVideoRTPSource.cpp
    # liveMedia/DVVideoStreamFramer.cpp
    liveMedia/EBMLNumber.cpp
    liveMedia/EBMLNumber.hh
    liveMedia/FileServerMediaSubsession.cpp
    liveMedia/FileSink.cpp
    liveMedia/FramedFileSource.cpp
    liveMedia/FramedFilter.cpp
    liveMedia/FramedSource.cpp
    # liveMedia/GSMAudioRTPSink.cpp
    liveMedia/H261VideoRTPSource.cpp
    # liveMedia/H263plusVideoFileServerMediaSubsession.cpp
    # liveMedia/H263plusVideoRTPSink.cpp
    liveMedia/H263plusVideoRTPSource.cpp
    # liveMedia/H263plusVideoStreamFramer.cpp
    # liveMedia/H263plusVideoStreamParser.cpp
    # liveMedia/H263plusVideoStreamParser.hh
    liveMedia/H264VideoFileServerMediaSubsession.cpp
    liveMedia/H264VideoFileSink.cpp
    liveMedia/H264VideoMatroskaFileServerMediaSubsession.cpp
    liveMedia/H264VideoMatroskaFileServerMediaSubsession.hh
    liveMedia/H264VideoRTPSink.cpp
    liveMedia/H264VideoRTPSource.cpp
    liveMedia/H264VideoStreamDiscreteFramer.cpp
    liveMedia/H264VideoStreamFramer.cpp
    liveMedia/InputFile.cpp
    liveMedia/JPEGVideoRTPSink.cpp
    liveMedia/JPEGVideoRTPSource.cpp
    liveMedia/JPEGVideoSource.cpp
    liveMedia/Locale.cpp
    # liveMedia/MatroskaDemuxedTrack.cpp
    # liveMedia/MatroskaDemuxedTrack.hh
    # liveMedia/MatroskaFile.cpp
    # liveMedia/MatroskaFileParser.cpp
    # liveMedia/MatroskaFileParser.hh
    # liveMedia/MatroskaFileServerDemux.cpp
    liveMedia/Media.cpp
    liveMedia/MediaSession.cpp
    liveMedia/MediaSink.cpp
    liveMedia/MediaSource.cpp
    liveMedia/MP3ADU.cpp
    liveMedia/MP3ADUdescriptor.cpp
    liveMedia/MP3ADUdescriptor.hh
    liveMedia/MP3ADUinterleaving.cpp
    # liveMedia/MP3ADURTPSink.cpp
    liveMedia/MP3ADURTPSource.cpp
    # liveMedia/MP3ADUTranscoder.cpp
    # liveMedia/MP3AudioFileServerMediaSubsession.cpp
    # liveMedia/MP3AudioMatroskaFileServerMediaSubsession.cpp
    # liveMedia/MP3AudioMatroskaFileServerMediaSubsession.hh
    # liveMedia/MP3FileSource.cpp
    liveMedia/MP3Internals.cpp
    liveMedia/MP3Internals.hh
    liveMedia/MP3InternalsHuffman.cpp
    liveMedia/MP3InternalsHuffman.hh
    liveMedia/MP3InternalsHuffmanTable.cpp
    # liveMedia/MP3StreamState.cpp
    # liveMedia/MP3StreamState.hh
    # liveMedia/MP3Transcoder.cpp
    # liveMedia/MPEG1or2AudioRTPSink.cpp
    liveMedia/MPEG1or2AudioRTPSource.cpp
    # liveMedia/MPEG1or2AudioStreamFramer.cpp
    liveMedia/MPEG1or2Demux.cpp
    liveMedia/MPEG1or2DemuxedElementaryStream.cpp
    # liveMedia/MPEG1or2DemuxedServerMediaSubsession.cpp
    # liveMedia/MPEG1or2FileServerDemux.cpp
    # liveMedia/MPEG1or2VideoFileServerMediaSubsession.cpp
    # liveMedia/MPEG1or2VideoRTPSink.cpp
    liveMedia/MPEG1or2VideoRTPSource.cpp
    # liveMedia/MPEG1or2VideoStreamDiscreteFramer.cpp
    # liveMedia/MPEG1or2VideoStreamFramer.cpp
    liveMedia/MPEG2IndexFromTransportStream.cpp
    liveMedia/MPEG2TransportFileServerMediaSubsession.cpp
    liveMedia/MPEG2TransportStreamFramer.cpp
    liveMedia/MPEG2TransportStreamFromESSource.cpp
    liveMedia/MPEG2TransportStreamFromPESSource.cpp
    liveMedia/MPEG2TransportStreamIndexFile.cpp
    liveMedia/MPEG2TransportStreamMultiplexor.cpp
    liveMedia/MPEG2TransportStreamTrickModeFilter.cpp
    liveMedia/MPEG2TransportUDPServerMediaSubsession.cpp
    # liveMedia/MPEG4ESVideoRTPSink.cpp
    liveMedia/MPEG4ESVideoRTPSource.cpp
    # liveMedia/MPEG4GenericRTPSink.cpp
    liveMedia/MPEG4GenericRTPSource.cpp
    # liveMedia/MPEG4LATMAudioRTPSink.cpp
    liveMedia/MPEG4LATMAudioRTPSource.cpp
    # liveMedia/MPEG4VideoFileServerMediaSubsession.cpp
    # liveMedia/MPEG4VideoStreamDiscreteFramer.cpp
    # liveMedia/MPEG4VideoStreamFramer.cpp
    liveMedia/MPEGVideoStreamFramer.cpp
    liveMedia/MPEGVideoStreamParser.cpp
    liveMedia/MPEGVideoStreamParser.hh
    liveMedia/MultiFramedRTPSink.cpp
    liveMedia/MultiFramedRTPSource.cpp
    liveMedia/OnDemandServerMediaSubsession.cpp
    liveMedia/our_md5.c
    liveMedia/our_md5.h
    liveMedia/our_md5hl.c
    liveMedia/OutputFile.cpp
    liveMedia/PassiveServerMediaSubsession.cpp
    liveMedia/ProxyServerMediaSession.cpp
    liveMedia/QCELPAudioRTPSource.cpp
    # liveMedia/QuickTimeFileSink.cpp
    liveMedia/QuickTimeGenericRTPSource.cpp
    liveMedia/RTCP.cpp
    liveMedia/rtcp_from_spec.c
    liveMedia/rtcp_from_spec.h
    liveMedia/RTPInterface.cpp
    liveMedia/RTPSink.cpp
    liveMedia/RTPSource.cpp
    liveMedia/RTSPClient.cpp
    liveMedia/RTSPCommon.cpp
    liveMedia/RTSPServer.cpp
    liveMedia/RTSPServerSupportingHTTPStreaming.cpp
    liveMedia/ServerMediaSession.cpp
    liveMedia/SimpleRTPSink.cpp
    liveMedia/SimpleRTPSource.cpp
    liveMedia/SIPClient.cpp
    liveMedia/StreamParser.cpp
    liveMedia/StreamParser.hh
    liveMedia/StreamReplicator.cpp
    # liveMedia/T140TextMatroskaFileServerMediaSubsession.cpp
    # liveMedia/T140TextMatroskaFileServerMediaSubsession.hh
    # liveMedia/T140TextRTPSink.cpp
    liveMedia/TCPStreamSink.cpp
    # liveMedia/TextRTPSink.cpp
    # liveMedia/uLawAudioFilter.cpp
    liveMedia/VideoRTPSink.cpp
    # liveMedia/VorbisAudioMatroskaFileServerMediaSubsession.cpp
    # liveMedia/VorbisAudioMatroskaFileServerMediaSubsession.hh
    # liveMedia/VorbisAudioRTPSink.cpp
    liveMedia/VorbisAudioRTPSource.cpp
    # liveMedia/VP8VideoMatroskaFileServerMediaSubsession.cpp
    # liveMedia/VP8VideoMatroskaFileServerMediaSubsession.hh
    # liveMedia/VP8VideoRTPSink.cpp
    liveMedia/VP8VideoRTPSource.cpp
    # liveMedia/WAVAudioFileServerMediaSubsession.cpp
    # liveMedia/WAVAudioFileSource.cpp
    # liveMedia/include/AC3AudioFileServerMediaSubsession.hh
    liveMedia/include/AC3AudioRTPSink.hh
    liveMedia/include/AC3AudioRTPSource.hh
    liveMedia/include/AC3AudioStreamFramer.hh
    # liveMedia/include/ADTSAudioFileServerMediaSubsession.hh
    # liveMedia/include/ADTSAudioFileSource.hh
    # liveMedia/include/AMRAudioFileServerMediaSubsession.hh
    # liveMedia/include/AMRAudioFileSink.hh
    # liveMedia/include/AMRAudioFileSource.hh
    # liveMedia/include/AMRAudioRTPSink.hh
    liveMedia/include/AMRAudioRTPSource.hh
    liveMedia/include/AMRAudioSource.hh
    liveMedia/include/AudioInputDevice.hh
    liveMedia/include/AudioRTPSink.hh
    liveMedia/include/AVIFileSink.hh
    liveMedia/include/Base64.hh
    liveMedia/include/BasicUDPSink.hh
    liveMedia/include/BasicUDPSource.hh
    liveMedia/include/ByteStreamFileSource.hh
    liveMedia/include/ByteStreamMemoryBufferSource.hh
    liveMedia/include/ByteStreamMultiFileSource.hh
    liveMedia/include/DarwinInjector.hh
    liveMedia/include/DeviceSource.hh
    liveMedia/include/DigestAuthentication.hh
    # liveMedia/include/DVVideoFileServerMediaSubsession.hh
    # liveMedia/include/DVVideoRTPSink.hh
    liveMedia/include/DVVideoRTPSource.hh
    # liveMedia/include/DVVideoStreamFramer.hh
    liveMedia/include/FileServerMediaSubsession.hh
    liveMedia/include/FileSink.hh
    liveMedia/include/FramedFileSource.hh
    liveMedia/include/FramedFilter.hh
    liveMedia/include/FramedSource.hh
    # liveMedia/include/GSMAudioRTPSink.hh
    liveMedia/include/H261VideoRTPSource.hh
    # liveMedia/include/H263plusVideoFileServerMediaSubsession.hh
    # liveMedia/include/H263plusVideoRTPSink.hh
    liveMedia/include/H263plusVideoRTPSource.hh
    # liveMedia/include/H263plusVideoStreamFramer.hh
    liveMedia/include/H264VideoFileServerMediaSubsession.hh
    liveMedia/include/H264VideoFileSink.hh
    liveMedia/include/H264VideoRTPSink.hh
    liveMedia/include/H264VideoRTPSource.hh
    liveMedia/include/H264VideoStreamDiscreteFramer.hh
    liveMedia/include/H264VideoStreamFramer.hh
    liveMedia/include/InputFile.hh
    liveMedia/include/JPEGVideoRTPSink.hh
    liveMedia/include/JPEGVideoRTPSource.hh
    liveMedia/include/JPEGVideoSource.hh
    liveMedia/include/liveMedia.hh
    liveMedia/include/liveMedia_version.hh
    liveMedia/include/Locale.hh
    # liveMedia/include/MatroskaFile.hh
    # liveMedia/include/MatroskaFileServerDemux.hh
    liveMedia/include/Media.hh
    liveMedia/include/MediaSession.hh
    liveMedia/include/MediaSink.hh
    liveMedia/include/MediaSource.hh
    liveMedia/include/MP3ADU.hh
    liveMedia/include/MP3ADUinterleaving.hh
    # liveMedia/include/MP3ADURTPSink.hh
    liveMedia/include/MP3ADURTPSource.hh
    # liveMedia/include/MP3ADUTranscoder.hh
    # liveMedia/include/MP3AudioFileServerMediaSubsession.hh
    # liveMedia/include/MP3FileSource.hh
    # liveMedia/include/MP3Transcoder.hh
    # liveMedia/include/MPEG1or2AudioRTPSink.hh
    liveMedia/include/MPEG1or2AudioRTPSource.hh
    # liveMedia/include/MPEG1or2AudioStreamFramer.hh
    liveMedia/include/MPEG1or2Demux.hh
    liveMedia/include/MPEG1or2DemuxedElementaryStream.hh
    # liveMedia/include/MPEG1or2DemuxedServerMediaSubsession.hh
    # liveMedia/include/MPEG1or2FileServerDemux.hh
    # liveMedia/include/MPEG1or2VideoFileServerMediaSubsession.hh
    # liveMedia/include/MPEG1or2VideoRTPSink.hh
    liveMedia/include/MPEG1or2VideoRTPSource.hh
    # liveMedia/include/MPEG1or2VideoStreamDiscreteFramer.hh
    # liveMedia/include/MPEG1or2VideoStreamFramer.hh
    liveMedia/include/MPEG2IndexFromTransportStream.hh
    liveMedia/include/MPEG2TransportFileServerMediaSubsession.hh
    liveMedia/include/MPEG2TransportStreamFramer.hh
    liveMedia/include/MPEG2TransportStreamFromESSource.hh
    liveMedia/include/MPEG2TransportStreamFromPESSource.hh
    liveMedia/include/MPEG2TransportStreamIndexFile.hh
    liveMedia/include/MPEG2TransportStreamMultiplexor.hh
    liveMedia/include/MPEG2TransportStreamTrickModeFilter.hh
    liveMedia/include/MPEG2TransportUDPServerMediaSubsession.hh
    # liveMedia/include/MPEG4ESVideoRTPSink.hh
    liveMedia/include/MPEG4ESVideoRTPSource.hh
    # liveMedia/include/MPEG4GenericRTPSink.hh
    liveMedia/include/MPEG4GenericRTPSource.hh
    # liveMedia/include/MPEG4LATMAudioRTPSink.hh
    liveMedia/include/MPEG4LATMAudioRTPSource.hh
    # liveMedia/include/MPEG4VideoFileServerMediaSubsession.hh
    # liveMedia/include/MPEG4VideoStreamDiscreteFramer.hh
    # liveMedia/include/MPEG4VideoStreamFramer.hh
    liveMedia/include/MPEGVideoStreamFramer.hh
    liveMedia/include/MultiFramedRTPSink.hh
    liveMedia/include/MultiFramedRTPSource.hh
    liveMedia/include/OnDemandServerMediaSubsession.hh
    liveMedia/include/OutputFile.hh
    liveMedia/include/PassiveServerMediaSubsession.hh
    liveMedia/include/ProxyServerMediaSession.hh
    liveMedia/include/QCELPAudioRTPSource.hh
    # liveMedia/include/QuickTimeFileSink.hh
    liveMedia/include/QuickTimeGenericRTPSource.hh
    liveMedia/include/RTCP.hh
    liveMedia/include/RTPInterface.hh
    liveMedia/include/RTPSink.hh
    liveMedia/include/RTPSource.hh
    liveMedia/include/RTSPClient.hh
    liveMedia/include/RTSPCommon.hh
    liveMedia/include/RTSPServer.hh
    liveMedia/include/RTSPServerSupportingHTTPStreaming.hh
    liveMedia/include/ServerMediaSession.hh
    liveMedia/include/SimpleRTPSink.hh
    liveMedia/include/SimpleRTPSource.hh
    liveMedia/include/SIPClient.hh
    liveMedia/include/StreamReplicator.hh
    # liveMedia/include/T140TextRTPSink.hh
    liveMedia/include/TCPStreamSink.hh
    # liveMedia/include/TextRTPSink.hh
    # liveMedia/include/uLawAudioFilter.hh
    liveMedia/include/VideoRTPSink.hh
    # liveMedia/include/VorbisAudioRTPSink.hh
    liveMedia/include/VorbisAudioRTPSource.hh
    # liveMedia/include/VP8VideoRTPSink.hh
    liveMedia/include/VP8VideoRTPSource.hh
    # liveMedia/include/WAVAudioFileServerMediaSubsession.hh
    # liveMedia/include/WAVAudioFileSource.hh
    )
source_group("liveMedia" FILES ${liveMedia})

add_library(live555MediaServer STATIC
    ${BasicUsageEnvironment}
    ${UsageEnvironment}
    ${groupsock}
    ${mediaServer}
    ${liveMedia}
    )

if (DEFINED CFG_DEMOD_ENABLE)
    add_library(live555MediaServer STATIC
        liveMedia/IteAirTsSplitServerMediaSubsession.cpp
        liveMedia/include/IteAirTsSplitServerMediaSubsession.h
        liveMedia/ItePesSource.cpp
        liveMedia/include/ItePesSource.h
        liveMedia/IteEventServerMediaSubsession.cpp
        liveMedia/include/IteEventServerMediaSubsession.h
        #liveMedia/IteAirTsServerMediaSubsession.cpp
        #liveMedia/include/IteAirTsServerMediaSubsession.h
        #liveMedia/IteAirTsStreamSource.cpp
        #liveMedia/include/IteAirTsStreamSource.h
        )
endif()

if( DEFINED CFG_WFD_SOURCE_DEVICE OR DEFINED CFG_WFD_SINK_DEVICE )
    add_library(live555MediaServer STATIC
        liveMedia/WFD/WFDSinkSession.cpp
        liveMedia/WFD/WFDSinkSession.hh
        liveMedia/WFD/WifiDisplaySink.cpp
        liveMedia/WFD/wifidisplaysink.hh
        liveMedia/WFD/WifiDisplaySource.cpp
        liveMedia/WFD/WifiDisplaySource.hh
        liveMedia/WFD/TSFileSink.cpp
        liveMedia/WFD/TSFileSink.hh
    )

    if( DEFINED CFG_WFD_SOURCE_DEVICE )
        add_definitions(
            -DCFG_WFD_SOURCE_DEVICE
            )
    elseif( DEFINED CFG_WFD_SINK_DEVICE )
        add_definitions(
            -DCFG_WFD_SINK_DEVICE
            )
    endif()
endif()

add_library(live555MediaClient STATIC
    ${BasicUsageEnvironment}
    ${UsageEnvironment}
    ${groupsock}
    ${mediaClient}
    ${liveMedia}
    )

if (DEFINED CFG_BUILD_LWIP)
    target_link_libraries(live555MediaClient
        lwip
    )
endif()
       