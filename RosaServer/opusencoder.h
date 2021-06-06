#pragma once
#include <iostream>
#include "opus.h"
#include "sol/sol.hpp"

class LuaOpusEncoder {
	OpusEncoder* encoder;
	FILE* input = nullptr;

 public:
	LuaOpusEncoder();
	~LuaOpusEncoder();
	void close();
	void open(const char* fileName);
	void rewind() const;
	sol::object encodeFrame(sol::this_state s) const;
};