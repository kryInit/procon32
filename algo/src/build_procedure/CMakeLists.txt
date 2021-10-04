add_subdirectory(lib)
add_subdirectory(solver)

add_executable(build_procedure build_procedure.cpp)
add_executable(simple_build_procedure simple_build_procedure.cpp)
add_executable(procedure_checker procedure_checker.cpp)
add_executable(procs_grader procs_grader.cpp)
target_link_libraries(build_procedure common_lib build_procedure_lib bp_solver)
target_link_libraries(simple_build_procedure common_lib build_procedure_lib bp_solver)
target_link_libraries(procedure_checker common_lib build_procedure_lib bp_solver)
target_link_libraries(procs_grader common_lib build_procedure_lib bp_solver)

set_target_properties( build_procedure
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../bin"
)
set_target_properties( simple_build_procedure
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../bin"
)
set_target_properties( procedure_checker
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../bin"
)
set_target_properties( procs_grader
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../bin"
)
