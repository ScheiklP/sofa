cmake_minimum_required(VERSION 3.12)
project(SofaHeadlessGui)

if(UNIX AND NOT APPLE)
    message("HeadlessGui for linux")
else()
  message(FATAL_ERROR The HeadlessGui in "${PROJECT_NAME} is a Linux only feature.")
endif()

find_package(Sofa.GL REQUIRED)

# FFMPEG
find_package(FFMPEG_exec)

# X11
sofa_find_package(X11 QUIET)
if(X11_FOUND)
    message("Found X11 libraries")
    include_directories(${X11_INCLUDE_DIR})
else()
    message(SEND_ERROR "Can't find X11 libraries.")
endif()

set(SRC_ROOT src/${PROJECT_NAME})

set(HEADER_FILES
    ${SRC_ROOT}/HeadlessGui.h
    ${SRC_ROOT}/FrameManager.h
)

set(SOURCE_FILES
    ${SRC_ROOT}/HeadlessGui.cpp
    ${SRC_ROOT}/FrameManager.cpp
)

# Find X11
sofa_find_package(X11 QUIET)
if(X11_FOUND)
    message("Found X11 libraries")
    include_directories(${X11_INCLUDE_DIR})
else()
    message(SEND_ERROR "Can't find X11 libraries.")
endif()


add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

# target_link_libraries(${PROJECT_NAME} PUBLIC SofaCore SofaDefaultType SofaSimulationCore SofaSimulationGraph Sofa.Helper)
target_link_libraries(${PROJECT_NAME} PUBLIC SofaGuiCommon Sofa.GL)
target_link_libraries(${PROJECT_NAME} PUBLIC ${X11_LIBRARIES})


sofa_create_package_with_targets(
    PACKAGE_NAME ${PROJECT_NAME}
    PACKAGE_VERSION ${Sofa_VERSION}
    TARGETS ${PROJECT_NAME} AUTO_SET_TARGET_PROPERTIES
    INCLUDE_SOURCE_DIR "src"
    INCLUDE_INSTALL_DIR ${PROJECT_NAME}
    RELOCATABLE "plugins"
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "SofaGui")
