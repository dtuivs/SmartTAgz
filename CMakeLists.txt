cmake_minimum_required(VERSION 3.16)

project(Register_App VERSION 0.7 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Qt6Core Qt6Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

find_path(ZINT_INCLUDE_DIR zint.h PATHS /usr/local/Cellar/zint/include)
find_library(ZINT_LIBRARY NAMES zint PATHS /usr/local/Cellar/zint/bin /usr/local/Cellar/zint/lib)

if(NOT ZINT_INCLUDE_DIR OR NOT ZINT_LIBRARY)
    message(FATAL_ERROR "Zint Library not found")
endif()

include_directories(${ZINT_INCLUDE_DIR})

set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
set_source_files_properties(icons/AppIcon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

set(TS_FILES Register_App_en_US.ts)
set(LIBS ${ZINT_LIBRARY})

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Register_App
        MACOSX_BUNDLE
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        icons.qrc
        settings.h settings.cpp settings.ui
        register_files.h register_files.cpp
        ticket_menu.h ticket_menu.cpp ticket_menu.ui
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Register_App APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(Register_App SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Register_App
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()
target_link_libraries(Register_App PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${LIBS} )

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Register_App)
endif()
set_target_properties(Register_App PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_sources(Register_App PRIVATE icons/AppIcon.icns)

# set_target_properties(Register_App PROPERTIES
#     MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/info.plist.in"
#     )

include(GNUInstallDirs)
install(TARGETS Register_App
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Register_App)
endif()
