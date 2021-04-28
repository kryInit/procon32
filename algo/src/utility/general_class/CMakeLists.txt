add_library(general_class simulated_annealing.cpp)
target_link_libraries(general_class time random)
target_include_directories(general_class PUBLIC ${PROJECT_SOURCE_DIR}/utility/general_class)
