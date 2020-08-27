cmake_minimum_required(VERSION 3.14)
project(NE VERSION 1.0)

set(CMAKE_CXX_FLAGS -O3)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

if(DEFINED ENV{VIRTUAL_ENV} OR DEFINED ENV{CONDA_PREFIX})
  set(_pip_args)
else()
  set(_pip_args "--user")
endif()

execute_process(COMMAND ${Python3_EXECUTABLE} -m pip install graphviz ${_pip_args})
execute_process(COMMAND ${Python3_EXECUTABLE} -m pip install python-sat[pblib,aiger] ${_pip_args})

configure_file(gviz.py.in gviz.py @ONLY)
configure_file(solve_PPH.py.in solve_PPH.py @ONLY)
configure_file(process_cnf.py.in process_cnf.py @ONLY)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(to_PH to_PH.cpp)
add_executable(to_PPH to_PPH.cpp)
add_executable(graph_gen graph_gen.cpp)
add_executable(players_gen players_gen.cpp)
add_executable(filter_players filter_players.cpp)
add_executable(players_to_PPH players_to_PPH.cpp)
add_executable(players_gen_filtered players_gen_filtered.cpp)

install(FILES start.sh graphs.txt
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
