cmake_minimum_required(VERSION 3.5)

# Do not show deprecated warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets DBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZSTD REQUIRED IMPORTED_TARGET libzstd)
pkg_check_modules(ZLIB REQUIRED IMPORTED_TARGET zlib)
pkg_check_modules(LIBSQSH REQUIRED IMPORTED_TARGET libsqsh)

# Find libsqsh.a; could be in /usr/local/lib or /usr/lib or /usr/local/lib/*/
find_library(LIBSQSH_STATIC_PATH libsqsh.a)

# Fix: fatal error: 'sqsh.h' file not found
include_directories(/usr/local/include/sqsh)

# Add the path to the libqxdg library
# include_directories(/usr/include/qt5xdg /usr/local/include/qt5xdg)

set(PROJECT_SOURCES
        main.cpp
        FileManagerMainWindow.cpp
        FileManagerMainWindow.h
        ApplicationBundle.h ApplicationBundle.cpp
        DesktopFile.h DesktopFile.cpp
        CustomItemDelegate.h CustomItemDelegate.cpp
        ExtendedAttributes.cpp ExtendedAttributes.h
        MoverCopier.cpp MoverCopier.h
        CustomFileIconProvider.cpp CustomFileIconProvider.h
        DBusInterface.cpp DBusInterface.h
        LaunchDB.cpp LaunchDB.h
        CombinedIconCreator.cpp CombinedIconCreator.h
        CustomFileSystemModel.cpp CustomFileSystemModel.h
        CustomListView.cpp CustomListView.h
        TrashHandler.cpp TrashHandler.h
        ElfSizeCalculator.cpp ElfSizeCalculator.h
        SqshArchiveReader.cpp SqshArchiveReader.h)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Filer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Filer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Filer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Filer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(Filer PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        ${LIBSQSH_STATIC_PATH}
        PkgConfig::ZSTD    # Link dynamically to libzstd
        PkgConfig::ZLIB    # Link dynamically to zlib
        pthread  # Link dynamically to pthread
        )

set_target_properties(Filer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS Filer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Filer)
endif()

# Create application bundle
set(APPLICATION_BUNDLE_NAME Filer.app)
set(APPLICATION_BUNDLE_PATH ${CMAKE_BINARY_DIR}/${APPLICATION_BUNDLE_NAME})

# Copy the application into the bundle
add_custom_command(TARGET Filer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:Filer>
        ${APPLICATION_BUNDLE_PATH}/Filer
)

# Copy resources into the bundle
# Remove previous Resources directory if it exists
add_custom_command(TARGET Filer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${APPLICATION_BUNDLE_PATH}/Resources
)
# Copy the Resources directory into the bundle
add_custom_command(TARGET Filer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Resources
        ${APPLICATION_BUNDLE_PATH}/Resources
)
# Until we have an icon for Filer, use the generic one
add_custom_command(TARGET Filer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${APPLICATION_BUNDLE_PATH}/Resources/Application.svg
        ${APPLICATION_BUNDLE_PATH}/Resources/Filer.svg
)

