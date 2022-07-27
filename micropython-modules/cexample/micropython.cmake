# Create an INTERFACE library for our C module.
add_library(usermod_cexample INTERFACE)

# Add our source files to the lib
target_sources(usermod_cexample INTERFACE
        examplemodule.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_cexample INTERFACE
        .
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_cexample)
