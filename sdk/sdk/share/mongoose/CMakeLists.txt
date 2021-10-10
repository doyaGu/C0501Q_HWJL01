include_directories(
    .
)

add_library(mongoose STATIC
    mg_locals.h
    mongoose.h
    mongoose.c
)
