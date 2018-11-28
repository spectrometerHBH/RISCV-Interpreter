cmake_minimum_required(VERSION 3.12)
project(RISCV_Interpreter)

set(CMAKE_CXX_STANDARD 14)

add_executable(RISCV_Interpreter
        LICENSE
        README.md main.cpp program.hpp)
