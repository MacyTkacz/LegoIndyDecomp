file(GLOB_RECURSE SOURCE_FILES *.cpp )
file(GLOB_RECURSE HEADER_FILES *.h )

add_library(thread STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_compile_options(thread PRIVATE ${INCLUDED_ERROR_FLAGS} ${EXCLUDED_ERROR_FLAGS} )
target_include_directories(thread PRIVATE ${INCLUDE_DIRECTORY} )