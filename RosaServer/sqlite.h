#pragma once
#include "sol/sol.hpp"

#include <tuple>
#include "sqlite3.h"

class SQLite {
	sqlite3* handle;

 public:
	SQLite(const char* fileName);
	~SQLite();
	void close();
	std::tuple<sol::object, sol::object> query(const char* sql,
	                                           sol::variadic_args arguments,
	                                           sol::this_state s);
};