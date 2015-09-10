cmake_minimum_required(VERSION 2.6)
project(pgtest)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

FIND_PACKAGE(PostgreSQL)

add_executable(pgexec exec.c)
add_executable(pgprobe probe.c)
add_executable(pgselect select.c)

if( PostgreSQL_FOUND )
  include_directories(${PostgreSQL_INCLUDE_DIRS})
  link_directories(${PostgreSQL_LIBRARY_DIRS})
endif( PostgreSQL_FOUND )

target_link_libraries(pgexec ${PostgreSQL_LIBRARIES} )
target_link_libraries(pgprobe ${PostgreSQL_LIBRARIES} )
target_link_libraries(pgselect ${PostgreSQL_LIBRARIES} )

install(TARGETS pgexec pgprobe pgselect RUNTIME DESTINATION bin)
