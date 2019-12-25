add_executable(unittest
    test_linear_open_address.cpp
    test_klibtable.cpp
    )
target_link_libraries(unittest PUBLIC Catch2 PLTables++)

add_executable(stress stresstest.cpp)
target_link_libraries(stress PUBLIC PLTables++)

add_executable(ctests
    test_qoatable.c
    cgreen_tests.c
    )
target_link_libraries(ctests PUBLIC CGreen PLTables)
