#
# Catch 2
#
add_library(Catch2 catch_main.cpp)
target_include_directories(Catch2 PUBLIC "catch2/single_include")
set_target_properties(Catch2 PROPERTIES CXX_STANDARD 17)
