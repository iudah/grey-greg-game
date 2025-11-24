cmake_minimum_required(VERSION 3.15)

project(grey)

option(ENABLE_SANITIZERS "Enable the sanitizers" OFF)

add_subdirectory(source)
add_subdirectory(game)
