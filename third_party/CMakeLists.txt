#
# Catch 2
#
add_library(Catch2 catch_main.cpp)
target_include_directories(Catch2 PUBLIC "catch2/single_include")
set_target_properties(Catch2 PROPERTIES CXX_STANDARD 17)
add_library(Catch2::Catch2 ALIAS Catch2)

#
# Google.Benchmark
#
option(BENCHMARK_ENABLE_TESTING OFF)
option(BENCHMARK_ENABLE_EXCEPTIONS ON)
option(BENCHMARK_ENABLE_INSTALL OFF)
option(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF)
add_subdirectory(benchmark)
add_library(Google::Benchmark ALIAS benchmark)
