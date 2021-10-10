include_directories(
    .
    )

add_library(matroska STATIC
    matroska2_project.h
    matroskablock.c
    matroskamain.c
    matroska_sem.c
    )
