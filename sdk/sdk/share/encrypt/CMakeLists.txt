INCLUDE_DIRECTORIES(
    ${CMAKE_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/sdk/share/encrypt/polarssl
    )

ADD_LIBRARY(encrypt STATIC
    aes.c
    des.c
    )