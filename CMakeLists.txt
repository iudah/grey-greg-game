cmake_minimum_required(VERSION 3.15)

project(gray VERSION 2.0 DESCRIPTION "gray")

set(TARGET gray)

add_subdirectory(source game)


# Install targets
# install(TARGETS ${TARGET} DESTINATION bin)
# install(FILES ${HEADER_FILES} DESTINATION include)