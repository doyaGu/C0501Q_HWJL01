include_directories(
    .
    )

add_library(ebml STATIC
    ebmlbinary.c
    ebmlcrc.c
    ebmlcrc.h
    ebmldate.c
    ebmlelement.c
    ebmlmain.c
    ebmlmaster.c
    ebmlnumber.c
    ebmlstring.c
    ebmlvoid.c
    )
