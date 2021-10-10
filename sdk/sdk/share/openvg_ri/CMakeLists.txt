if (DEFINED CFG_OPENVG_ENABLE AND DEFINED CFG_OPENVG_SW)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        add_definitions(
            -DDEBUG
            )
    endif()

    include_directories(
        .
        ${PROJECT_SOURCE_DIR}/sdk/include/EGL
        ${PROJECT_SOURCE_DIR}/sdk/include/VG
        )

    add_library(OpenVG STATIC
        riApi.cpp
        riArray.h
        riContext.cpp
        riContext.h
        riDefs.h
        riFont.cpp
        riFont.h
        riImage.cpp
        riImage.h
        riMath.cpp
        riMath.h
        riPath.cpp
        riPath.h
        riPixelPipe.cpp
        riPixelPipe.h
        riRasterizer.cpp
        riRasterizer.h
        riVGU.cpp
        riMiniEGL.cpp
        )

    if (DEFINED CFG_WIN32_SIMULATOR)
        add_library(OpenVG STATIC
            win32/riEGLOS.cpp
            )
    else()
        add_library(OpenVG STATIC
            openrtos/riEGLOS.cpp
            )
    endif()
    
endif()
