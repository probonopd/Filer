cmake_minimum_required(VERSION 3.5)

# Build fileoperation/
add_subdirectory(fileoperation)

# Build renamedisk/
add_subdirectory(renamedisk)

# Do not show deprecated warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets DBus Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus Multimedia)

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
        AppGlobals.cpp AppGlobals.h
        ApplicationBundle.cpp ApplicationBundle.h
        CombinedIconCreator.cpp CombinedIconCreator.h
        CustomFileIconProvider.cpp CustomFileIconProvider.h
        CustomFileSystemModel.cpp CustomFileSystemModel.h
        CustomItemDelegate.cpp CustomItemDelegate.h
        CustomListView.cpp CustomListView.h
        CustomProxyModel.cpp CustomProxyModel.h
        DBusInterface.cpp DBusInterface.h
        DesktopFile.cpp DesktopFile.h
        ElfSizeCalculator.cpp ElfSizeCalculator.h
        ExtendedAttributes.cpp ExtendedAttributes.h
        FileManagerMainWindow.cpp FileManagerMainWindow.h
        InfoDialog.cpp InfoDialog.h
        LaunchDB.cpp LaunchDB.h
        SoundPlayer.cpp SoundPlayer.h
        SqshArchiveReader.cpp SqshArchiveReader.h
        TrashHandler.cpp TrashHandler.h
        FileOperationManager.cpp FileOperationManager.h
        VolumeWatcher.cpp VolumeWatcher.h
        )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Filer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(Filer
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(Filer PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Multimedia
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
        COMMENT "Copying Filer to bundle..."
)

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
    COMMENT "Copying Resources directory to bundle..."
)
# Until we have an icon for Filer, use the generic one
add_custom_command(TARGET Filer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${APPLICATION_BUNDLE_PATH}/Resources/Application.svg
        ${APPLICATION_BUNDLE_PATH}/Resources/Filer.svg
)

# Remove previous bin/ directory if it exists
add_custom_command(TARGET Filer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        ${APPLICATION_BUNDLE_PATH}/bin
)

# Create bin/ directory in the bundle
add_custom_command(TARGET Filer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${APPLICATION_BUNDLE_PATH}/bin
)

# Copy fileoperation binary into the bundle under bin/
add_custom_command(TARGET Filer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:fileoperation>
        ${APPLICATION_BUNDLE_PATH}/bin/fileoperation
        COMMENT "Copying fileoperation binary to bundle..."
)

# Copy renamedisk binary into the bundle under bin/
add_custom_command(TARGET Filer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:renamedisk>
        ${APPLICATION_BUNDLE_PATH}/bin/renamedisk
        COMMENT "Copying renamedisk binary to bundle..."
)

install(TARGETS Filer RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

# Install the application bundle in /System
install(DIRECTORY
        ${APPLICATION_BUNDLE_PATH}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/../
    )

# Set executable permissions
install(CODE "
    execute_process(
        COMMAND chmod +x ${CMAKE_INSTALL_PREFIX}/../${APPLICATION_BUNDLE_NAME}/Filer
    )
    execute_process(
        COMMAND chmod +x ${CMAKE_INSTALL_PREFIX}/../${APPLICATION_BUNDLE_NAME}/bin/fileoperation
        COMMAND chmod +x ${CMAKE_INSTALL_PREFIX}/../${APPLICATION_BUNDLE_NAME}/bin/renamedisk
    )
    ")