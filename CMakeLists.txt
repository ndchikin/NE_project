cmake_minimum_required(VERSION 3.14)
project(NE VERSION 1.0)

set(CMAKE_CXX_FLAGS -O3)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

execute_process(COMMAND pip install graphviz)
execute_process(COMMAND apt-get install graphviz)
execute_process(COMMAND pip install python-sat[pblib,aiger])

include_directories(${PROJECT_SOURCE_DIR})

add_executable(to_PH to_PH.cpp)
add_executable(to_PPH to_PPH.cpp)
add_executable(graph_gen graph_gen.cpp)
add_executable(players_gen players_gen.cpp)
add_executable(filter_players filter_players.cpp)
add_executable(players_to_PPH players_to_PPH.cpp)
add_executable(players_gen_filtered players_gen_filtered.cpp)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

install(FILES start.sh graphs.txt gviz.py process_cnf.py solve_PPH.py
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
