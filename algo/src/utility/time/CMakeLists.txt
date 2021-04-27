add_library(time stopwatch.cpp time_manager.cpp)
target_include_directories(time PUBLIC ${PROJECT_SOURCE_DIR}/utility/time)
