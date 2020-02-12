cmake_minimum_required(VERSION 3.16)

# Add header files. This doesn't affect the build but it ensures they are
# included in IDEs
set(HEADER_LIST "${gramConfig_SOURCE_DIR}/include/gramconfig/configurable.hpp"
                "${gramConfig_SOURCE_DIR}/include/gramconfig/configuration.hpp")

add_library(gramConfig ${HEADER_LIST} configuration.cpp)

# C++17 is needed for this library
target_compile_features(gramConfig PUBLIC cxx_std_17)

# Add the include folder as publically accessable
target_include_directories(gramConfig PUBLIC "${gramConfig_SOURCE_DIR}/include")

# Add TinyXml2 as an external project
include(FetchContent)
FetchContent_Declare(
  TinyXml2
  GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
  GIT_TAG master)
FetchContent_MakeAvailable(TinyXml2)
target_include_directories(gramConfig PUBLIC ${tinyxml2_SOURCE_DIR})

# None of the required libraries are exposed externally
target_link_libraries(gramConfig PRIVATE tinyxml2 stdc++fs)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
