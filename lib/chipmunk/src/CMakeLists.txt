file(GLOB chipmunk_source_files "*.c" "constraints/*.c")
file(GLOB chipmunk_public_header "${chipmunk_SOURCE_DIR}/include/chipmunk/*.h")
file(GLOB chipmunk_constraint_header "${chipmunk_SOURCE_DIR}/include/chipmunk/constraints/*.h")

include_directories(${chipmunk_SOURCE_DIR}/include/chipmunk)

add_library(chipmunk_static STATIC ${chipmunk_source_files})

# Sets chipmunk_static to output "libchipmunk.a" not "libchipmunk_static.a"
set_target_properties(chipmunk_static PROPERTIES OUTPUT_NAME chipmunk)

