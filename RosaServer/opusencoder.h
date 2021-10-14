#pragma once
#include <iostream>
#include "opus.h"
#include "sol/sol.hpp"

class LuaOpusEncoder {
	OpusEncoder* encoder;
	FILE* input = nullptr;

	int encodeFrameStringShorts(std::string_view inputBytes,
	                            unsigned char output[]) const;
	int encodeFrameStringFloats(std::string_view inputBytes,
	                            unsigned char output[]) const;

 public:
	LuaOpusEncoder();
	~LuaOpusEncoder();
	void setBitRate(opus_int32 bitRate) const;
	opus_int32 getBitRate() const;
	void close();
	void open(const char* fileName);
	void rewind() const;
	sol::object encodeFrame(sol::this_state s) const;
	std::string encodeFrameString(std::string_view inputBytes) const;
};