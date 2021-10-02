#pragma once
#include <string>

namespace Lua {
namespace crypto {
std::string sha256(std::string_view input);
}  // namespace crypto
}  // namespace Lua