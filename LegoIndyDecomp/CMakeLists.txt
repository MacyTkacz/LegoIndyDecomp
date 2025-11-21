# CMakeList.txt : CMake project for LegoIndyDecomp, include source and define
# project specific logic here.
#

file(GLOB_RECURSE SOURCE_FILES *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)

# Add source to this project's executable.
add_executable (LegoIndyDecomp ${SOURCE_FILES} ${HEADER_FILES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LegoIndyDecomp PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(LegoIndyDecomp PRIVATE .)

# TODO: Add tests and install targets if needed.
