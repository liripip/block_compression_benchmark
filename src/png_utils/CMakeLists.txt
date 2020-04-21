add_library(png_utils STATIC)

target_sources(png_utils PRIVATE
	png_utils.cpp
	png_utils.hpp
)

target_include_directories(png_utils PUBLIC "")
target_link_libraries(png_utils PRIVATE libpng)
