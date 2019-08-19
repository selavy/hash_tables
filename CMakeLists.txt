cmake_minimum_required(VERSION 3.5)

project(tables
    VERSION 0.0.1
    LANGUAGES C CXX
    )

add_executable(tables
    main.c
    MurmurHash3.c
    utils.h
    linear_open_addressing.h
    quadratic_open_addressing.h
    qtable.c
    )
set_target_properties(tables PROPERTIES C_STANDARD 11)
# cset_target_properties(tables PROPERTIES CXX_STANDARD 17)
