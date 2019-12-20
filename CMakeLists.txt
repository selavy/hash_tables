cmake_minimum_required(VERSION 3.5)

project(tables
    VERSION 0.0.1
    LANGUAGES C CXX
    )

add_subdirectory(third_party)
add_subdirectory(include)

# add_executable(tables
#     main.c
#     MurmurHash3.c
#     utils.h
#     linear_open_addressing.h
#     quadratic_open_addressing.h
#     qtable.c
#     )
# set_target_properties(tables PROPERTIES C_STANDARD 11)
# # cset_target_properties(tables PROPERTIES CXX_STANDARD 17)
# 
# add_executable(c++table
#     c++/main.cpp
#     c++/catch.hpp
#     c++/table_test.cpp
#     c++/table.h
#     )
# set_target_properties(c++table PROPERTIES CXX_STANDARD 17)
