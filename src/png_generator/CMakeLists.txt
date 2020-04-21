add_executable(png_generator)

target_sources(png_generator PRIVATE main.cpp)

target_link_libraries(png_generator PRIVATE argparse png_utils)
