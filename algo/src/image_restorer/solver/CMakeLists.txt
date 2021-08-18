add_library(solver side_beam_search.cpp)
target_link_libraries(solver common_lib image_restorer_lib utility)
target_include_directories(solver PUBLIC ${PROJECT_SOURCE_DIR}/image_restorer/solver)