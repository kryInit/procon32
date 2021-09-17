add_subdirectory(lib)
 add_subdirectory(solver)

add_executable(build_procedure build_procedure.cpp)
target_link_libraries(build_procedure common_lib build_procedure_lib bp_solver)

set_target_properties( build_procedure
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../bin"
)
