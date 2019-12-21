add_executable(unittest
    test_linear_open_address.cpp
    )
set_target_properties(unittest PROPERTIES CXX_STANDARD 17)
target_link_libraries(unittest
    PUBLIC
        Catch2
        PLTables
    )

add_executable(stress stresstest.cpp)
set_target_properties(stress PROPERTIES CXX_STANDARD 17)
target_link_libraries(stress PUBLIC PLTables)
