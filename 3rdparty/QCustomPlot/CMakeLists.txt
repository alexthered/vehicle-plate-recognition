file (GLOB 3RD_PARTY_SRCS
     ${CMAKE_CURRENT_SOURCE_DIR}/*.h
     ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     )

#set the source files so that it can be accessed from parent folder
set( 3RD_PARTY_SRCS
    ${3RD_PARTY_SRCS}
    PARENT_SCOPE
)
