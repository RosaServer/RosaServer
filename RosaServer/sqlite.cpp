#include "sqlite.h"

SQLite::SQLite(const char* fileName) {
	int res = sqlite3_open(fileName, &handle);
	if (res != SQLITE_OK || handle == nullptr) {
		close();
		throw std::runtime_error(sqlite3_errstr(res));
	}
}

SQLite::~SQLite() { close(); }

void SQLite::close() {
	if (handle) {
		sqlite3_close(handle);
		handle = nullptr;
	}
}

std::tuple<sol::object, sol::object> SQLite::query(const char* sql,
                                                   sol::variadic_args arguments,
                                                   sol::this_state s) {
	sol::state_view lua(s);
	sqlite3_stmt* statement;

	{
		int res = SQLITE_ERROR;
		if (handle) {
			res = sqlite3_prepare_v2(handle, sql, -1, &statement, nullptr);
		}

		if (res != SQLITE_OK) {
			return std::make_tuple(
			    sol::make_object(lua, sol::nil),
			    sol::make_object(
			        lua, handle ? sqlite3_errmsg(handle) : sqlite3_errstr(res)));
		}
	}

	int index = 0;

	for (sol::object arg : arguments) {
		index++;

		int res;
		switch (arg.get_type()) {
			case sol::type::nil:
				res = sqlite3_bind_null(statement, index);
				break;
			case sol::type::string: {
				auto string = arg.as<std::string>();
				res = sqlite3_bind_text(statement, index, string.data(),
				                        string.length(), SQLITE_TRANSIENT);
			} break;
			case sol::type::number:
				res = sqlite3_bind_double(statement, index, arg.as<double>());
				break;
			case sol::type::boolean:
				res = sqlite3_bind_int(statement, index, arg.as<bool>() ? 1 : 0);
				break;
			default:
				res = SQLITE_OK;
				break;
		}

		if (res != SQLITE_OK) {
			sqlite3_finalize(statement);
			return std::make_tuple(sol::make_object(lua, sol::nil),
			                       sol::make_object(lua, sqlite3_errmsg(handle)));
		}
	}

	sol::table rows;
	int numColumns = sqlite3_column_count(statement);
	if (numColumns) {
		rows = lua.create_table();
	}

	while (true) {
		int res = sqlite3_step(statement);
		if (res == SQLITE_DONE) {
			break;
		}

		if (res != SQLITE_ROW) {
			sqlite3_finalize(statement);
			return std::make_tuple(sol::make_object(lua, sol::nil),
			                       sol::make_object(lua, sqlite3_errmsg(handle)));
		}

		sol::table row = lua.create_table(numColumns);
		for (int i = 0; i < numColumns; i++) {
			switch (sqlite3_column_type(statement, i)) {
				case SQLITE_INTEGER:
					row.set(i + 1, sqlite3_column_int64(statement, i));
					break;
				case SQLITE_FLOAT:
					row.set(i + 1, sqlite3_column_double(statement, i));
					break;
				case SQLITE_BLOB:
				case SQLITE_TEXT: {
					auto data = sqlite3_column_blob(statement, i);
					if (data) {
						auto size = sqlite3_column_bytes(statement, i);
						std::string blob(reinterpret_cast<const char*>(data), size);
						row.set(i + 1, blob);
					} else {
						row.set(i + 1, "");
					}
					break;
				}
				default:
					row.set(i + 1, sol::nil);
					break;
			}
		}
		rows.add(row);
	}

	sqlite3_finalize(statement);

	if (numColumns) {
		return std::make_tuple(sol::make_object(lua, rows),
		                       sol::make_object(lua, sol::nil));
	}

	return std::make_tuple(sol::make_object(lua, sqlite3_changes(handle)),
	                       sol::make_object(lua, sol::nil));
}