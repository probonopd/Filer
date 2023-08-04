cmake_minimum_required(VERSION 3.5)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(fileoperation
        main.cpp
        MainWindow.h
        MainWindow.cpp
        CopyThread.h
        CopyThread.cpp
        CopyProgressDialog.h
        CopyProgressDialog.cpp
        CopyManager.h
        CopyManager.cpp
        )

target_link_libraries(fileoperation PRIVATE Qt5::Widgets)