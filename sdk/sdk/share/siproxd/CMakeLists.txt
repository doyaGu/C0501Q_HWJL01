
if (DEFINED CFG_GCC_LTO)
    add_definitions("-fno-lto")
endif ()

add_library(siproxd STATIC
			siproxd.c
			proxy.c
			register.c
			sock.c
			utils.c
		  sip_utils.c
		  sip_layer.c
		  log.c
		  readconf.c
		  rtpproxy.c
		  rtpproxy_relay.c
		  accessctl.c
		  route_processing.c
		  security.c
		  auth.c
		  fwapi.c
		  resolve.c
		  dejitter.c
		  plugins.c
		  redirect_cache.c
    )
