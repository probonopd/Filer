cmake_minimum_required(VERSION 3.5)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

add_executable(renamedisk
        main.cpp
        )

target_link_libraries(renamedisk PRIVATE Qt5::Core)