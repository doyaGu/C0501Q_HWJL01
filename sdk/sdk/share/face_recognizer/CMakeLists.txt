include_directories(
    .
	../opencv/cv/include
    ../opencv/cvaux/include
    ../opencv/cxcore/include
	../opencv/otherlibs/highgui
	../opencv/cv/src
    )
    
add_library(face_recognizer STATIC
    face_detect.cpp
    face_recognizer.cpp
	lbp.cpp
	lda.cpp
	pca.cpp
	weber_filter.cpp
	face_recognizer.h
	haarcascade_frontalface_alt.h
    )
