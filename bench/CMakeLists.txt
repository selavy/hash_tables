add_executable(bench-loa bench_loa.cpp)
target_link_libraries(bench-loa
    PUBLIC
    PLTables
    Google::Benchmark
    )
