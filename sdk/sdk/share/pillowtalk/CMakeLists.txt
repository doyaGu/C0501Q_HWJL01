include_directories(
    .
    ${PROJECT_SOURCE_DIR}/sdk/share/
    ${PROJECT_SOURCE_DIR}/sdk/share/yajl/
)

SET(PILLOWTALK_MAJOR 0)
SET(PILLOWTALK_MINOR 3)
SET(PILLOWTALK_MICRO 0)

SET(pillowtalk_SRCS pillowtalk_impl.c)
SET(pillowtalk_HDRS pillowtalk.h pillowtalk_impl.h)

ADD_LIBRARY( pillowtalk STATIC ${pillowtalk_SRCS} ${pillowtalk_HDRS} )

#### setup shared library version number
SET_TARGET_PROPERTIES(pillowtalk PROPERTIES
                      SOVERSION ${PILLOWTALK_MAJOR}
                      VERSION ${PILLOWTALK_MAJOR}.${PILLOWTALK_MINOR}.${PILLOWTALK_MICRO})

TARGET_LINK_LIBRARIES(pillowtalk ${YAJL_LIBRARY} ${CURL_LIBRARY})

# Output Paths
SET (output_include ${CMAKE_CURRENT_BINARY_DIR}/../include)
FILE(MAKE_DIRECTORY ${output_include})

# Copy Public .h's into the output directory so the tests can get it
#SET (pillowtalk_PUB_HDRS pillowtalk.h)
#FOREACH (header ${pillowtalk_PUB_HDRS})
#  SET (header ${CMAKE_CURRENT_SOURCE_DIR}/${header})

#  EXEC_PROGRAM(${CMAKE_COMMAND} ARGS -E copy_if_different ${header} ${output_include})

#ENDFOREACH (header ${pillowtalk_PUB_HDRS})

# Installation Step
#INSTALL( TARGETS pillowtalk LIBRARY DESTINATION lib)
#INSTALL( FILES pillowtalk.h DESTINATION include)
