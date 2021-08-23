add_library(bp_solver simple_solver.cpp kr_solver.cpp)
target_link_libraries(bp_solver common_lib build_procedure_lib utility)
target_include_directories(bp_solver PUBLIC ${PROJECT_SOURCE_DIR}/build_procedure/solver)