cmake_minimum_required(VERSION 3.14)
project(NE VERSION 1.0)

set(CMAKE_CXX_FLAGS -O3)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

configure_file(gviz.py.in gviz.py)
configure_file(solve_PPH.py.in solve_PPH.py)
configure_file(process_cnf.py.in process_cnf.py)

set(OWNER_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
set(GROUP_PERMISSIONS GROUP_READ GROUP_WRITE GROUP_EXECUTE)
set(WORLD_PERMISSIONS WORLD_READ WORLD_WRITE WORLD_EXECUTE)

file(COPY graphs.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY start.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR} FILE_PERMISSIONS ${OWNER_PERMISSIONS} ${GROUP_PERMISSIONS} ${WORLD_PERMISSIONS})

include_directories(${PROJECT_SOURCE_DIR})

add_executable(to_PH to_PH.cpp)
add_executable(to_PPH to_PPH.cpp)
add_executable(graph_gen graph_gen.cpp)
add_executable(players_gen players_gen.cpp)
add_executable(filter_players filter_players.cpp)
add_executable(players_to_PPH players_to_PPH.cpp)
add_executable(players_gen_filtered players_gen_filtered.cpp)

