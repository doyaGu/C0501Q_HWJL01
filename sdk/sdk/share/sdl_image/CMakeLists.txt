add_definitions(
    -DLOAD_BMP
    -DLOAD_JPG
    -DLOAD_PNG
    )

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/SDL
    ${PROJECT_SOURCE_DIR}/sdk/include
    )
    
add_library(sdl_image STATIC
	IMG.c
	IMG_bmp.c
	IMG_gif.c
    IMG_ite_jpg.c
	IMG_jpg.c
	IMG_lbm.c
	IMG_pcx.c
	IMG_png.c
	IMG_pnm.c
	IMG_tga.c
	IMG_tif.c
	IMG_xcf.c
	IMG_xpm.c
	IMG_xv.c	
	IMG_webp.c
    )
