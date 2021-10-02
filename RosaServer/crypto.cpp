#include "crypto.h"

#include <openssl/sha.h>
#include <cstdio>
#include <iomanip>
#include <sstream>

namespace Lua {
namespace crypto {
std::string sha256(std::string_view input) {
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX context;
	SHA256_Init(&context);
	SHA256_Update(&context, input.data(), input.length());
	SHA256_Final(hash, &context);

	std::stringstream output;
	output << std::hex << std::setfill('0');

	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		output << std::setw(2) << +hash[i];
	}

	return output.str();
}
}  // namespace crypto
}  // namespace Lua