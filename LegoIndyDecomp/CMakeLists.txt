# file(GLOB_RECURSE SOURCE_FILES *.cpp)
# file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB_RECURSE TEST_SOURCE_FILES compat/filesystem/*.cpp)
file(GLOB_RECURSE TEST_HEADER_FILES compat/filesystem/*.h)

# Add source to this project's executable.
# add_executable (LegoIndyDecomp ${SOURCE_FILES} ${HEADER_FILES})
add_executable (LegoIndyDecomp main.cpp error.h utils.h ${TEST_SOURCE_FILES} ${TEST_HEADER_FILES} )

# warnings/errors
target_compile_options(LegoIndyDecomp PRIVATE -Wall -Wextra -Werror )
target_compile_options(LegoIndyDecomp PRIVATE -Wno-unused-parameter -Wno-comment )

target_compile_options(LegoIndyDecomp PRIVATE -g)

if (WIN32)
	target_link_options(LegoIndyDecomp PRIVATE -static-libstdc++ -static-libgcc)
endif()

target_include_directories(LegoIndyDecomp PRIVATE .)