add_library(compressonator STATIC IMPORTED GLOBAL)

set_target_properties(compressonator PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
	IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/compressonator.lib")