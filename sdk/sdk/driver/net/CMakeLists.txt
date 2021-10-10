file(GLOB sub_dir RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

foreach (dir ${sub_dir})
    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
   
        string(TOUPPER ${dir} lib)

        if (DEFINED CFG_BUILD_${lib})
            add_subdirectory(${dir})
        endif()

    endif()

endforeach()
