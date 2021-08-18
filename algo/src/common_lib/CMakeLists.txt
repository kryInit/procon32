add_library(common_lib settings.cpp)
target_link_libraries(common_lib utility)
target_include_directories(common_lib PUBLIC ${PROJECT_SOURCE_DIR}/common_lib)
