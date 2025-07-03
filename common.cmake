set(C_WARNINGS "-Wall -Wextra -Wshadow -Wformat=2 -Wunused")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG -ggdb -O0 -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/memalloc/lib/cmake/memalloc"
    "${CMAKE_BINARY_DIR}/zot/lib/cmake/zot" "${CMAKE_BINARY_DIR}/grey/lib/cmake/grey")

