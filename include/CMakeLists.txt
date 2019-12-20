add_library(PLTables INTERFACE)
target_sources(PLTables
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/pltables/linear_open_address.h
    )
target_include_directories(PLTables
    INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
