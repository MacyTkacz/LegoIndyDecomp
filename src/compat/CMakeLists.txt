file(GLOB_RECURSE SOURCE_FILES *.cpp )
file(GLOB_RECURSE HEADER_FILES *.h )

add_library(compatibility STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_compile_options(compatibility PRIVATE ${INCLUDED_ERROR_FLAGS} ${EXCLUDED_ERROR_FLAGS} )
target_include_directories(compatibility PRIVATE ${INCLUDE_DIRECTORY} )