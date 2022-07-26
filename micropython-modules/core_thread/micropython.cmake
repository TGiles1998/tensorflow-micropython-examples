# Create an INTERFACE library for our C module.
add_library(usermod_core_thread INTERFACE)

# Add our source files to the lib
target_sources(usermod_core_thread INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/my_thread.c
    ${CMAKE_CURRENT_LIST_DIR}/core_thread.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_core_thread INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_core_thread)