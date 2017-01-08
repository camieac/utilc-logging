#Compile the example executable
add_executable(utilc-logging-example utilc-logging-example.c)
include_directories(../include/)
target_link_libraries(utilc-logging-example utilc-logging)
