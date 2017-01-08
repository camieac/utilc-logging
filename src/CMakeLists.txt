# Bring in the headers and sources
include_directories(../include)
file(GLOB SOURCES "*.c")

# Generate the shared library from the sources
add_library(utilc-logging SHARED ${SOURCES})

# Compile with math library
target_link_libraries(utilc-logging m)

# Install system-wide
install(TARGETS utilc-logging DESTINATION /usr/lib)
