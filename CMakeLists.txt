cmake_minimum_required(VERSION 3.0)

project(brainfuckplus)
set(CMAKE_C_STANDARD 99)

set(SRCS
 src/main.c
 src/crash.h
 src/crash.c
 src/memory.h
 src/memory.c
 src/interpreter.h
 src/interpreter.c
 src/state.h
)

add_executable(brainfuckplus ${SRCS})

target_include_directories(brainfuckplus PRIVATE src)

# Add platform-specific flags
if (MSVC)
    # Flags specific to Visual Studio (MSVC)
    target_compile_options(brainfuckplus PRIVATE /W4 /WX-)
else()
    # Linux/GCC specific flags
    target_compile_options(brainfuckplus PRIVATE -Wall -Wextra -O2)
endif()