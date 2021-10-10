include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/ucl
    )

add_library(ucl STATIC
	src/alloc.c  
	src/ucl_init.c  
	src/ucl_util.c 
	src/ucl_ptr.c  
	src/n2b_d.c  
	src/n2b_99.c 
	src/n2b_ds.c  
	src/n2b_to.c  
	src/n2d_d.c  
	src/n2d_99.c 
	src/n2d_ds.c  
	src/n2d_to.c  
	src/n2e_d.c  
	src/n2e_99.c 
	src/n2e_ds.c  
	src/n2e_to.c  
	src/ucl_crc.c  
    )
