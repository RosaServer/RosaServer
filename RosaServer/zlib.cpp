#include "zlib.h"
#include <stdexcept>

namespace Lua {
namespace zlib {
std::string _compress(std::string_view input) {
	uLong compressedSize = compressBound(input.size());
	uint8_t* compressed = new uint8_t[compressedSize];

	int status =
	    compress(compressed, &compressedSize,
	             reinterpret_cast<const uint8_t*>(input.data()), input.size());
	if (status != Z_OK) {
		throw std::runtime_error(zError(status));
	}

	std::string compressedString(reinterpret_cast<const char*>(compressed),
	                             compressedSize);

	delete compressed;
	return compressedString;
}

std::string _uncompress(std::string_view compressed, uLong uncompressedSize) {
	uint8_t* uncompressed = new uint8_t[uncompressedSize];

	int status = uncompress(uncompressed, &uncompressedSize,
	                        reinterpret_cast<const uint8_t*>(compressed.data()),
	                        compressed.size());
	if (status != Z_OK) {
		throw std::runtime_error(zError(status));
	}

	std::string uncompressedString(reinterpret_cast<const char*>(uncompressed),
	                               uncompressedSize);

	delete uncompressed;
	return uncompressedString;
}
}  // namespace zlib
}  // namespace Lua