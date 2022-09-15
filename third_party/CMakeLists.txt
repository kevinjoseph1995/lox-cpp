add_subdirectory(fmt)

option(BUILD_GMOCK "Builds the googlemock subproject" OFF)
option(INSTALL_GTEST "Disable installing gtest" OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-restrict")
add_subdirectory(googletest)