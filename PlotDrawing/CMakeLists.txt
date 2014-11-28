file (GLOB PLOT_WINDOW_SRCS
     ${CMAKE_CURRENT_SOURCE_DIR}/*.h
     ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*.ui  #the ui file as well
     )

#set the source files so that it can be accessed from parent folder
set( PLOT_WINDOW_SRCS
    ${PLOT_WINDOW_SRCS}
    PARENT_SCOPE
)
