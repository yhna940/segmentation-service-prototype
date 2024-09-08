cmake_minimum_required(VERSION 3.10)

# Set the project name
project(remote-segmentor-dispatcher VERSION 1.0 LANGUAGES CXX)
message(STATUS "Project: ${PROJECT_NAME}")

# Set the C++ standard to be used
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages for the project
find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(GDAL REQUIRED)
find_package(TritonCommon REQUIRED)
find_package(TritonClient REQUIRED)

# Set the source files for the project
set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/scene_inferencer.cpp
    ${PROJECT_SOURCE_DIR}/src/triton_client.cpp
    ${PROJECT_SOURCE_DIR}/src/gdal_image_loader.cpp
    ${PROJECT_SOURCE_DIR}/src/gdal_image_saver.cpp
    ${PROJECT_SOURCE_DIR}/src/service.cpp
    ${PROJECT_SOURCE_DIR}/src/worker_thread.cpp
)

# Define the executable target
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories for the project
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${GDAL_INCLUDE_DIRS}
    $ENV{TRITON_CLIENT_BUILD_DIR}/include
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/cpp-httplib
)

# Set the link directories for the target
target_link_directories(${PROJECT_NAME}
    PRIVATE
    $ENV{TRITON_CLIENT_BUILD_DIR}/lib
)

# Link the required libraries for the target
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    grpcclient
    httpclient
    ${OpenCV_LIBS}
    CURL::libcurl
    ${GDAL_LIBRARIES}
)

# Install required dependencies for building (optional)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
