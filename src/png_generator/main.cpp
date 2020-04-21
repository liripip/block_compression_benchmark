#include <argparse.h>
#include <png_utils.hpp>

#include <string>
#include <memory>
#include <cstdlib>
#include <ctime>

namespace
{
std::unique_ptr<unsigned char[]> generateCheckerData(size_t width, size_t height)
{
	auto data = std::make_unique<unsigned char[]>(width * height * 4);
	
	size_t rowLength = width * 4;

	size_t checkerSize = width / 8;
	for (size_t y = 0; y < height; ++y)
	{
		auto row = data.get() + y * rowLength;

		for (size_t x = 0; x < width; ++x)
		{
			unsigned char color = (x / checkerSize % 2 == y / checkerSize % 2 ? 0 : 255);
			row[x * 4 + 0] = color;
			row[x * 4 + 1] = color;
			row[x * 4 + 2] = color;
			row[x * 4 + 3] = 255;
		}
	}

	return data;
}

std::unique_ptr<unsigned char[]> generateNoiseData(size_t width, size_t height)
{
	auto data = std::make_unique<unsigned char[]>(width * height * 4);

	size_t rowLength = width * 4;

	srand(time(nullptr));
	for (size_t y = 0; y < height; ++y)
	{
		auto row = data.get() + y * rowLength;
		for (size_t x = 0; x < width; ++x)
		{
			row[x * 4 + 0] = rand() % 256;
			row[x * 4 + 1] = rand() % 256;
			row[x * 4 + 2] = rand() % 256;
			row[x * 4 + 3] = 255;
		}
	}

	return data;
}

enum class TextureType
{
	CHECKER,
	NOISE,
};

std::unique_ptr<unsigned char[]> generateData(TextureType type, size_t width, size_t height)
{
	switch (type)
	{
	case TextureType::CHECKER: return generateCheckerData(width, height);
	case TextureType::NOISE: return generateNoiseData(width, height);
	default: return nullptr;
	}
}

} // namespace

int main(int argc, const char* argv[])
{
	argparse::ArgumentParser parser("Texture generator");
	parser.add_argument().name("--type").description("texture type [noise, checker]");
	parser.add_argument().name("--size").description("texture size").required(true);
	parser.add_argument().name("--output").description("texture file path").required(true);

	if (auto err = parser.parse(argc, argv))
	{
		std::cerr << err << std::endl;
		parser.print_help();
		return 1;
	}

	TextureType type = TextureType::CHECKER;
	if (parser.exists("type"))
	{
		auto value = parser.get<std::string>("type");
		if (value == "noise")
		{
			type = TextureType::NOISE;
		}
		else if (value == "checker")
		{
			type = TextureType::CHECKER;
		}
		else
		{
			std::cerr << "Unknown texture type " << value << std::endl;
			return 1;
		}
	}

	auto output = parser.get<std::string>("output");
	auto size = parser.get<size_t>("size");

	auto data = generateData(type, size, size);
	PngUtils::writePng(output.c_str(), PngUtils::Format::RGBA, size, size, data.get());

	return 0;
}