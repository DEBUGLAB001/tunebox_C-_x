cmake_minimum_required(VERSION 3.20)
project(tunebox VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SEED_BUGS "Enable the 4 intentional bugs for candidate debugging" OFF)

if(SEED_BUGS)
    add_compile_definitions(SEED_BUGS)
    message(STATUS "TuneBox configured with SEED_BUGS=ON (intentional bugs active)")
else()
    message(STATUS "TuneBox configured with SEED_BUGS=OFF (clean/fixed mode)")
endif()

# Core include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Discover service source files
set(SERVICE_SOURCES
    src/service/library_service.cpp
    src/service/search_service.cpp
    src/service/playlist_service.cpp
    src/service/playback_queue.cpp
    src/service/playback_controller.cpp
)

# Target 1: core_services static library
add_library(core_services STATIC ${SERVICE_SOURCES})
target_include_directories(core_services PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(WIN32)
    target_link_libraries(core_services PUBLIC ws2_32 crypt32)
endif()

# Target 2: server executable
add_executable(server src/main.cpp)
target_link_libraries(server PRIVATE core_services)

if(WIN32)
    target_link_libraries(server PRIVATE ws2_32 crypt32)
endif()

# Target 3: challenge_tests executable
add_executable(challenge_tests tests/test_suite.cpp)
target_link_libraries(challenge_tests PRIVATE core_services)

if(WIN32)
    target_link_libraries(challenge_tests PRIVATE ws2_32 crypt32)
endif()
