if (DEFINED CFG_OPENVG_ENABLE AND NOT DEFINED CFG_OPENVG_SW)
	if (CMAKE_BUILD_TYPE STREQUAL Debug)
		add_definitions(
			-DDEBUG
			)
	endif()

	include_directories(
        .
        ${PROJECT_SOURCE_DIR}/sdk/include/EGL
        ${PROJECT_SOURCE_DIR}/sdk/include/VG
        ${PROJECT_SOURCE_DIR}/sdk/share/nedmalloc
		)
		
	if ( DEFINED CFG_WIN32_SIMULATOR )
		set(EGLOS_SRCS win32/iteEGLOS.c)
	else()
		set(EGLOS_SRCS openrtos/iteEGLOS.c)
	endif()

	add_library(OpenVG STATIC
		iteArrayBase.h
		iteArrays.c
		iteArrays.h
		iteContext.c
		iteContext.h
		iteDefs.h
		iteFont.c
		iteFont.h
		iteGeometry.c
		iteGeometry.h
		iteGlyph.c
		iteGlyph.h
		iteHardware.c
		iteHardware.h
		iteImage.c
		iteImage.h
		iteMask.c
		itePaint.c
		itePaint.h
		iteParams.c
		itePath.c
		itePath.h
		iteUtility.c
		iteUtility.h
		iteVectors.c
		iteVectors.h
		iteExtendAPI.c
    convert_path.c
    convert_path.h
		iteVgu.c
		vgmem.c
		vgmem.h
		#$ENV{CFG_PLATFORM}/iteEGLOS.c
		${EGLOS_SRCS}
		iteEGL.c
		)
endif()
