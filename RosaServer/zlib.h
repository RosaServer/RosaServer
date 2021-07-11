#pragma once
#include <string>
#include "miniz.h"

namespace Lua {
namespace zlib {
std::string _compress(std::string_view input);
std::string _uncompress(std::string_view compressed, uLong uncompressedSize);
}  // namespace zlib
}  // namespace Lua