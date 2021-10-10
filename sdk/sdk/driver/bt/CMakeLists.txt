
include_directories(
    .
    ..
    ${PROJECT_SOURCE_DIR}/sdk/include/bt    
    ${SMEDIA_INC_DIRS}
)

add_library(bt STATIC
        bluetooth.c       
    )    
