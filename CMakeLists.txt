cmake_minimum_required(VERSION 3.16)

# Setup list of configurations (must be done before any project() calls)
set(CMAKE_CONFIGURATION_TYPES "Release")

project(compression_tests)

# Use solution folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Configure output paths
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/.bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/.lib")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Global preprocessor definitions
add_compile_definitions(
	# Use Unicode character set
	_UNICODE
	UNICODE

	# Disable MSVC specific security warnings
	_CRT_SECURE_NO_DEPRECATE
	_CRT_NONSTDC_NO_DEPRECATE

	# Never bring useless stuff from windows.h
	WIN32_LEAN_AND_MEAN
	NOMINMAX
)

# Global compiler options
# Note that by default, Debug build will have:
#  /Od   [optimization disabled]
#  /RTC1 [runtime checks for uninit variables and stack usage]
#  /MDd  [use debug runtime]
# Release build will have:
#  /O2   [optimization for max speed]
#  /Ob2  [full inline expansion]
#  /MD   [use release runtime]
add_compile_options(
	/W4				# Warning level 4
	/MP				# Enable parallel builds
	/Gy				# Enable function level linking
	/Gw				# Optimize Global Data
	/GR-			# Disable Run-Time Type Information
	/GS-			# Disable Security Check
	/Oi				# Enable intrinsics
)

# Global linker options
add_link_options(
	/WX				# Enable warnings as errors
)

# external packages
set(THIRD_PARTY_ROOT_DIR ${CMAKE_SOURCE_DIR}/extern)
add_subdirectory(extern/argparse)
add_subdirectory(extern/zlib)
add_subdirectory(extern/libpng)
add_subdirectory(extern/cuda)
add_subdirectory(extern/compressonator)
add_subdirectory(extern/nvtt)
add_subdirectory(extern/DirectXTex)

# our projects
add_subdirectory(src/png_utils)
add_subdirectory(src/png_generator)
add_subdirectory(src/benchmark)
