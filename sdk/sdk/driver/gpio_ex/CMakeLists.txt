include_directories(
    .
    ..
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/gpio_ex
    )

add_library(gpio_ex STATIC
        ${CFG_GPIO_EXPANDER_MODULE}
        ${CFG_SECOND_GPIO_EXPANDER_MODULE}
    )
