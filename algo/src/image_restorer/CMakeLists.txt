add_subdirectory(lib)
add_subdirectory(adjacency_evaluator)
add_subdirectory(solver)

add_executable(image_restorer image_restorer.cpp)
target_link_libraries(image_restorer image_restorer_lib adjacency_evaluator solver time general_class)

set_target_properties( image_restorer
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/../bin"
)



