add_library(PLTables INTERFACE)
target_sources(PLTables
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/pltables/linear_open_address.h
    )
target_compile_features(PLTables INTERFACE cxx_std_17)
target_include_directories(PLTables
    INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# add_library(KLib INTERFACE)
# target_sources(KLib
#     INTERFACE
#     ${CMAKE_CURRENT_SOURCE_DIR}/klib/khash.h
#     )
# target_compile_features(KLib INTERFACE cxx_std_17)
# target_include_directories(KLib
#     INTERFACE
#         $<INSTALL_INTERFACE:include>
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
# )
