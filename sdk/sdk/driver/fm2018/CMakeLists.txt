INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/fm2018
    )

ADD_LIBRARY(fm2018
    fortemedia_fm2018_io.c  
    fortemedia_fm2018_driver.c
    )


