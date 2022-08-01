add_library(usermod_enterprise INTERFACE)

target_sources(usermod_enterprise INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/enterprise.c
)

target_include_directories(usermod_enterprise INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(usermod INTERFACE usermod_enterprise)