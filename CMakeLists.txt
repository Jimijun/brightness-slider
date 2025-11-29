cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME brightness-slider)
project(${PROJECT_NAME})

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(DDCUtil REQUIRED ddcutil)

set(CMAKE_AUTOMOC ON)

set(HEADERS
    DisplayInfo.h
    MainWindow.h
)

set(SOURCES
    main.cpp
    DisplayInfo.cpp
    MainWindow.cpp
)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets ${DDCUtil_LIBRARIES})
