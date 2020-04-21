add_library(libpng STATIC IMPORTED GLOBAL)

set_target_properties(libpng PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
	IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libpng16_static.lib")

target_link_libraries(libpng INTERFACE zlib)
