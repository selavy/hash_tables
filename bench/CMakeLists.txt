add_executable(bench-loa bench_loa.cpp)
target_link_libraries(bench-loa
    PUBLIC
    PLTables++
    Google::Benchmark
    )

add_executable(bench-vector bench_vector.cpp)
target_link_libraries(bench-vector
    PUBLIC
    PLTables++
    Google::Benchmark
    )
