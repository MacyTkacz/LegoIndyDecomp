file(GLOB_RECURSE SOURCE_FILES *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)

set(CMAKE_CXX_COMPILER g++)

# Add source to this project's executable.
add_executable (LegoIndyDecomp ${SOURCE_FILES} ${HEADER_FILES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
	target_compile_options(LegoIndyDecomp PRIVATE -std=c++20 -g)
endif()

target_include_directories(LegoIndyDecomp PRIVATE .)