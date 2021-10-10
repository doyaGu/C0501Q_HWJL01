include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/SDL
    )

add_library(sdl_ttf STATIC
    SDL_ttf.c
    )
