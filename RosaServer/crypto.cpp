#include "crypto.h"

#include <openssl/md5.h>
#include <openssl/sha.h>
#include <cstdio>
#include <iomanip>
#include <sstream>

namespace Lua {
namespace crypto {
static void digest(std::stringstream& output, unsigned char* hash,
                   size_t hashLength) {
	output << std::hex << std::setfill('0');

	for (int i = 0; i < hashLength; i++) {
		output << std::setw(2) << +hash[i];
	}
}

std::string md5(std::string_view input) {
	unsigned char hash[MD5_DIGEST_LENGTH];
	MD5_CTX context;
	MD5_Init(&context);
	MD5_Update(&context, input.data(), input.length());
	MD5_Final(hash, &context);

	std::stringstream output;
	digest(output, hash, MD5_DIGEST_LENGTH);
	return output.str();
}

std::string sha256(std::string_view input) {
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX context;
	SHA256_Init(&context);
	SHA256_Update(&context, input.data(), input.length());
	SHA256_Final(hash, &context);

	std::stringstream output;
	digest(output, hash, SHA256_DIGEST_LENGTH);
	return output.str();
}
}  // namespace crypto
}  // namespace Lua