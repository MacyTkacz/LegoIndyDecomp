add_subdirectory(compat)
add_subdirectory(thread)

add_executable ( LegoIndyDecomp main.cpp )

set(INCLUDED_ERROR_FLAGS -Wall -Wextra -Werror )
set(EXCLUDED_ERROR_FLAGS -Wno-unused-parameter -Wno-comment )

# warnings/errors
target_compile_options(LegoIndyDecomp PRIVATE ${INCLUDED_ERROR_FLAGS} ${EXCLUDED_ERROR_FLAGS} -g )
target_include_directories(LegoIndyDecomp PRIVATE ${INCLUDE_DIRECTORY} )

# static libs
target_link_libraries(LegoIndyDecomp compatibility thread)

if (WIN32)
	target_link_options(LegoIndyDecomp PRIVATE -static-libstdc++ -static-libgcc)
endif()