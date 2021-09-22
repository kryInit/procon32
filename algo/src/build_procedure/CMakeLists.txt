add_subdirectory(lib)
 add_subdirectory(solver)

add_executable(simple_build_procedure simple_build_procedure.cpp)
target_link_libraries(simple_build_procedure common_lib build_procedure_lib bp_solver)

set_target_properties( simple_build_procedure
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../bin"
)
