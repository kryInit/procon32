add_library(image_restorer_lib image.cpp answer.cpp)
target_link_libraries(image_restorer_lib common_lib utility)
target_include_directories(image_restorer_lib PUBLIC ${PROJECT_SOURCE_DIR}/image_restorer/lib)
