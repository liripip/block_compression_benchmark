add_executable(benchmark)

target_sources(benchmark
	PRIVATE
		main.cpp

		codec.hpp
		codec.cpp

		compressonator_codec.hpp
		compressonator_codec.cpp

		nvtt_codec.hpp
		nvtt_codec.cpp

		directxtex_codec.hpp
		directxtex_codec.cpp

		benchmark.hpp
		benchmark.cpp

		decompress_impl.hpp
		decompress_impl.cpp
)

target_link_libraries(benchmark
	PRIVATE
		D3D11
		DXGI
		argparse
		png_utils
		compressonator
		nvtt
		cuda
		DirectXTex)
