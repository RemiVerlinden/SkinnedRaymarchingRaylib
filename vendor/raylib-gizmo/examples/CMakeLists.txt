set(RESOURCES_PATH "${RGIZMO_ROOT_PATH}/examples/resources/")
set(INCLUDE_PATH "${RGIZMO_ROOT_PATH}/include")

function(add_example example_name source_file)
    add_executable(${example_name} ${source_file})
    target_link_libraries(${example_name} PRIVATE raylib raygizmo)
    target_compile_definitions(${example_name} PRIVATE RESOURCES_PATH="${RESOURCES_PATH}")
    target_include_directories(${example_name} PRIVATE ${RAYLIB_PATH} ${INCLUDE_PATH})
endfunction()

add_example(example_01 "examples/example_01_getting_started.c")
add_example(example_02 "examples/example_02_gizmo_types.c")

