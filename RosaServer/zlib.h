#pragma once
#include <string>
#include "miniz.h"

namespace Lua {
namespace zlib {
std::string _compress(std::string input);
std::string _uncompress(std::string compressed, uLong uncompressedSize);
}  // namespace zlib
}  // namespace Lua