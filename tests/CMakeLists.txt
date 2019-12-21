add_executable(unittest
    test_linear_open_address.cpp
    )
target_link_libraries(unittest PUBLIC Catch2 PLTables)

add_executable(stress stresstest.cpp)
target_link_libraries(stress PUBLIC PLTables)
