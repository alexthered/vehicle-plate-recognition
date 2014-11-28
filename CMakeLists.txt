#
# Project properties 
#
  
project(vehiclePlateRecongnition)
cmake_minimum_required( VERSION 2.8 )

#
# Project options
#
option( VERBOSE "Display intermediate results" ON )

#
# Project directory
#
set( 3RDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty" )
set( PLOTDRAWING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/PlotDrawing")

#OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_DIRS})

#Use QT4
find_package(Qt4)
set( QT_USE_QTGUI TRUE )  #use GUI
#set( CMAKE_AUTOMOC ON )
set( QT_USE_QTWIDGETS TRUE)
set( QT_USE_QTPrintSupport TRUE)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

#include source file in top folder
file (GLOB TOP_SRCS
     ${CMAKE_CURRENT_SOURCE_DIR}/*.h
     ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     )

#
# add PlotWindow module
#
SET(plotwindow_SOURCES ${PLOTDRAWING_DIR}/plotwindow.cpp ${3RDPARTY_DIR}/QCustomPlot/qcustomplot.cpp)
SET(plotwindow_HEADERS ${PLOTDRAWING_DIR}/plotwindow.h ${3RDPARTY_DIR}/QCustomPlot/qcustomplot.h)
SET(plotwindow_FORMS ${PLOTDRAWING_DIR}/mainwindow.ui)

QT4_WRAP_CPP(plotwindow_HEADERS_MOC ${plotwindow_HEADERS})
QT4_WRAP_UI(plotwindow_FORMS_HEADERS ${plotwindow_FORMS})

set(PLOTWINDOW_SRCS
    ${plotwindow_HEADERS_MOC}
    ${plotwindow_FORMS_HEADERS}
    ${plotwindow_SOURCES}
    ${plotwindow_HEADERS}
)
#include build directory since the ui_mainwindow.h will be generated here
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

#include source file in all folder
set(PROJECT_SRCS
    ${TOP_SRCS}
    ${3RD_PARTY_SRCS}
    ${PLOTWINDOW_SRCS}
    )
    
#set the place to generate the binary file
if( NOT CMAKE_BUILD_TYPE STREQUAL "Release" )
set( CMAKE_BUILD_TYPE "Debug" )
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Release" )
add_definitions( -DQT_NO_DEBUG )
add_definitions( -DQT_NO_DEBUG_OUTPUT )
add_definitions( -O3 )
elseif( CMAKE_BUILD_TYPE STREQUAL "Debug" )
# add_definitions( -DQT_DEBUG )
add_definitions( -DIPCAM_DEBUG )
endif()

# Create output folder with build type (release/debug)
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE} )

add_executable(${PROJECT_NAME} ${PROJECT_SRCS})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${QT_LIBRARIES})
