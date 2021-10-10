
if (DEFINED CFG_CHIP_PKG_IT9910)
    ADD_LIBRARY(video_encoder STATIC
	    video_encoder_it9910.c	
	)	
	
	  TARGET_LINK_LIBRARIES(video_encoder
	     encoder
       sensor
       capture
		   vp
	)
else()
    ADD_LIBRARY(video_encoder STATIC
	    video_encoder.c	
	)
endif()

