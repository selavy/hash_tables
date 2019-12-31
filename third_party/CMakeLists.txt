#
# Catch 2
#
add_library(Catch2 catch_main.cpp)
target_include_directories(Catch2 PUBLIC "catch2/single_include")
target_compile_features(Catch2 INTERFACE cxx_std_17)
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

#
# CGreen
#
option(USE_SYSTEM_CGREEN "Use system install version of CGreen" ON)
if (USE_SYSTEM_CGREEN)
	find_package(cgreen REQUIRED)
else ()
	add_subdirectory(cgreen)
	set(CGREEN_LIBRARIES cgreen)
    set(CGREEN_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/cgreen/include)
endif (USE_SYSTEM_CGREEN)

add_library(CGreen SHARED cgreen_main.c)
target_include_directories(CGreen PUBLIC ${CGREEN_INCLUDE_DIRS})
target_link_libraries(CGreen PUBLIC ${CGREEN_LIBRARIES})
