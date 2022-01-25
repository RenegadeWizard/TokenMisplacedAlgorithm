cmake_minimum_required(VERSION 3.20)
project(TokenMisplacedAlgorithm)

find_package(MPI REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall")

add_executable(TokenMisplacedAlgorithm main.cpp model/MessageType.h model/Message.h util/Logger.cpp util/Logger.h service/TokenCommunication.cpp service/TokenCommunication.h)
target_link_libraries(TokenMisplacedAlgorithm ${MPI_LIBRARIES})
include_directories(${MPI_INCLUDE_PATH})
if (MPI_COMPILE_FLAGS)
    set_target_properties(TokenMisplacedAlgorithm PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif ()
if (MPI_LINK_FLAGS)
    set_target_properties(TokenMisplacedAlgorithm PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
endif ()
