add_executable(image_restorer image_restorer.cu)
target_link_libraries(image_restorer time general_class)

set_target_properties( image_restorer
    PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    # RUNTIME_OUTPUT_DIRECTORY "../../bin/"
)



