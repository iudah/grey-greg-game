set(C_WARNINGS "-Wall -Wextra -Wshadow -Wformat=2 -Wunused")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/memalloc/lib/cmake/memalloc"
     "${CMAKE_BINARY_DIR}/zot/lib/cmake/zot"
     "${CMAKE_BINARY_DIR}/grey/lib/cmake/grey")
