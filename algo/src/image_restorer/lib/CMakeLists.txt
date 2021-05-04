add_library(image_restorer_lib settings.cpp image.cpp answer.cpp)
target_link_libraries(image_restorer_lib general_class)
target_include_directories(image_restorer_lib PUBLIC ${PROJECT_SOURCE_DIR}/image_restorer/lib)
