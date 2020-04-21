#include <memory>

namespace PngUtils
{
enum class Format { RGBA, UNKNOWN };

struct Readback
{
	std::unique_ptr<unsigned char[]> data;
	Format format;
	size_t width;
	size_t height;
};

Readback readPng(const char fileName[]);
bool writePng(const char fileName[], Format format, size_t width, size_t height, unsigned char* data);
} // namespace PngUtils
