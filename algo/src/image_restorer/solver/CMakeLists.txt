add_library(solver side_beam_search.cpp)
target_link_libraries(solver image_restorer_lib general_class)
target_include_directories(solver PUBLIC ${PROJECT_SOURCE_DIR}/image_restorer/solver)
