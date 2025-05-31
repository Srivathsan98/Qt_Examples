cmake_minimum_required(VERSION 3.16)
project(QtCamStream)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS 
    Core
    Gui
    Widgets
    Network
)
find_package(OpenCV REQUIRED)

# Add source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    videorecorder.cpp
    videorecorder.h
    global_vars.cpp
)

# Create executable
add_executable(QtCamStream ${SOURCES})

# Link libraries
target_link_libraries(QtCamStream
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    ${OpenCV_LIBS}
)

# Include directories
target_include_directories(QtCamStream PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

# Set compiler flags
if(MSVC)
    target_compile_options(QtCamStream PRIVATE /W4)
else()
    target_compile_options(QtCamStream PRIVATE -Wall -Wextra)
endif()
