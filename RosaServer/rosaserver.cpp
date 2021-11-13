#include "rosaserver.h"

#include <cxxabi.h>
#include <execinfo.h>
#include <sys/mman.h>

#include <cerrno>

static Server* server;

static void pryMemory(void* address, size_t numPages) {
	size_t pageSize = sysconf(_SC_PAGE_SIZE);

	uintptr_t page = (uintptr_t)address;
	page -= (page % pageSize);

	if (mprotect((void*)page, pageSize * numPages, PROT_WRITE | PROT_READ) == 0) {
		std::ostringstream stream;

		stream << RS_PREFIX "Successfully pried open page at ";
		stream << std::showbase << std::hex;
		stream << static_cast<uintptr_t>(page);
		stream << "\n";

		Console::log(stream.str());
	} else {
		throw std::runtime_error(strerror(errno));
	}
}

// https://github.com/moonjit/moonjit/blob/master/doc/c_api.md#luajit_setmodel-idx-luajit_mode_wrapcfuncflag
static int wrapExceptions(lua_State* L, lua_CFunction f) {
	try {
		return f(L);
	} catch (const char* s) {
		lua_pushstring(L, s);
	} catch (std::exception& e) {
		lua_pushstring(L, e.what());
	} catch (...) {
		lua_pushliteral(L, "caught (...)");
	}
	return lua_error(L);
}

void defineThreadSafeAPIs(sol::state* state) {
	lua_pushlightuserdata(*state, (void*)wrapExceptions);
	luaJIT_setmode(*state, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON);
	lua_pop(*state, 1);

	state->open_libraries(sol::lib::base);
	state->open_libraries(sol::lib::package);
	state->open_libraries(sol::lib::coroutine);
	state->open_libraries(sol::lib::string);
	state->open_libraries(sol::lib::os);
	state->open_libraries(sol::lib::math);
	state->open_libraries(sol::lib::table);
	state->open_libraries(sol::lib::debug);
	state->open_libraries(sol::lib::bit32);
	state->open_libraries(sol::lib::io);
	state->open_libraries(sol::lib::ffi);
	state->open_libraries(sol::lib::jit);

	{
		auto meta = state->new_usertype<Vector>("new", sol::no_constructor);
		meta["x"] = &Vector::x;
		meta["y"] = &Vector::y;
		meta["z"] = &Vector::z;

		meta["class"] = sol::property(&Vector::getClass);
		meta["__tostring"] = &Vector::__tostring;
		meta["__add"] = &Vector::__add;
		meta["__sub"] = &Vector::__sub;
		meta["__mul"] = sol::overload(&Vector::__mul, &Vector::__mul_RotMatrix);
		meta["__div"] = &Vector::__div;
		meta["__unm"] = &Vector::__unm;
		meta["add"] = &Vector::add;
		meta["mult"] = &Vector::mult;
		meta["set"] = &Vector::set;
		meta["clone"] = &Vector::clone;
		meta["dist"] = &Vector::dist;
		meta["distSquare"] = &Vector::distSquare;
		meta["length"] = &Vector::length;
		meta["lengthSquare"] = &Vector::lengthSquare;
		meta["dot"] = &Vector::dot;
		meta["getBlockPos"] = &Vector::getBlockPos;
		meta["normalize"] = &Vector::normalize;
	}

	{
		auto meta = state->new_usertype<RotMatrix>("new", sol::no_constructor);
		meta["x1"] = &RotMatrix::x1;
		meta["y1"] = &RotMatrix::y1;
		meta["z1"] = &RotMatrix::z1;
		meta["x2"] = &RotMatrix::x2;
		meta["y2"] = &RotMatrix::y2;
		meta["z2"] = &RotMatrix::z2;
		meta["x3"] = &RotMatrix::x3;
		meta["y3"] = &RotMatrix::y3;
		meta["z3"] = &RotMatrix::z3;

		meta["class"] = sol::property(&RotMatrix::getClass);
		meta["__tostring"] = &RotMatrix::__tostring;
		meta["__mul"] = &RotMatrix::__mul;
		meta["set"] = &RotMatrix::set;
		meta["clone"] = &RotMatrix::clone;
		meta["getForward"] = &RotMatrix::getForward;
		meta["getUp"] = &RotMatrix::getUp;
		meta["getRight"] = &RotMatrix::getRight;
	}

	{
		auto meta = state->new_usertype<Image>("Image");
		meta["width"] = sol::property(&Image::getWidth);
		meta["height"] = sol::property(&Image::getHeight);
		meta["numChannels"] = sol::property(&Image::getNumChannels);
		meta["free"] = &Image::free;
		meta["loadFromFile"] = &Image::loadFromFile;
		meta["loadBlank"] = &Image::loadBlank;
		meta["getRGB"] = &Image::getRGB;
		meta["getRGBA"] = &Image::getRGBA;
		meta["setPixel"] = sol::overload(&Image::setRGB, &Image::setRGBA);
		meta["getPNG"] = &Image::getPNG;
	}

	{
		auto meta = state->new_usertype<LuaOpusEncoder>("OpusEncoder");
		meta["bitRate"] =
		    sol::property(&LuaOpusEncoder::getBitRate, &LuaOpusEncoder::setBitRate);
		meta["close"] = &LuaOpusEncoder::close;
		meta["open"] = &LuaOpusEncoder::open;
		meta["rewind"] = &LuaOpusEncoder::rewind;
		meta["encodeFrame"] = sol::overload(&LuaOpusEncoder::encodeFrame,
		                                    &LuaOpusEncoder::encodeFrameString);
	}

	{
		auto meta = state->new_usertype<PointGraph>(
		    "PointGraph", sol::constructors<PointGraph(unsigned int)>());
		meta["getSize"] = &PointGraph::getSize;
		meta["addNode"] = &PointGraph::addNode;
		meta["getNodePoint"] = &PointGraph::getNodePoint;
		meta["addLink"] = &PointGraph::addLink;
		meta["getNodeByPoint"] = &PointGraph::getNodeByPoint;
		meta["findShortestPath"] = &PointGraph::findShortestPath;
	}

	{
		auto meta = state->new_usertype<FileWatcher>("FileWatcher");
		meta["addWatch"] = &FileWatcher::addWatch;
		meta["removeWatch"] = &FileWatcher::removeWatch;
		meta["receiveEvent"] = &FileWatcher::receiveEvent;
	}

	{
		auto meta = state->new_usertype<SQLite>(
		    "SQLite", sol::constructors<SQLite(const char*)>());
		meta["close"] = &SQLite::close;
		meta["query"] = &SQLite::query;
	}

	(*state)["print"] = Lua::print;

	(*state)["Vector"] = sol::overload(Lua::Vector_, Lua::Vector_3f);
	(*state)["RotMatrix"] = Lua::RotMatrix_;

	(*state)["os"]["listDirectory"] = Lua::os::listDirectory;
	(*state)["os"]["createDirectory"] = Lua::os::createDirectory;
	(*state)["os"]["realClock"] = Lua::os::realClock;
	(*state)["os"]["getLastWriteTime"] = Lua::os::getLastWriteTime;
	(*state)["os"]["exit"] = sol::overload(Lua::os::exit, Lua::os::exitCode);

	{
		auto httpTable = state->create_table();
		(*state)["http"] = httpTable;
		httpTable["getSync"] = Lua::http::getSync;
		httpTable["postSync"] = Lua::http::postSync;
	}

	{
		auto zlibTable = state->create_table();
		(*state)["zlib"] = zlibTable;
		zlibTable["compress"] = Lua::zlib::_compress;
		zlibTable["uncompress"] = Lua::zlib::_uncompress;
	}

	{
		auto cryptoTable = state->create_table();
		(*state)["crypto"] = cryptoTable;
		cryptoTable["md5"] = Lua::crypto::md5;
		cryptoTable["sha256"] = Lua::crypto::sha256;
	}

	(*state)["FILE_WATCH_ACCESS"] = IN_ACCESS;
	(*state)["FILE_WATCH_ATTRIB"] = IN_ATTRIB;
	(*state)["FILE_WATCH_CLOSE_WRITE"] = IN_CLOSE_WRITE;
	(*state)["FILE_WATCH_CLOSE_NOWRITE"] = IN_CLOSE_NOWRITE;
	(*state)["FILE_WATCH_CREATE"] = IN_CREATE;
	(*state)["FILE_WATCH_DELETE"] = IN_DELETE;
	(*state)["FILE_WATCH_DELETE_SELF"] = IN_DELETE_SELF;
	(*state)["FILE_WATCH_MODIFY"] = IN_MODIFY;
	(*state)["FILE_WATCH_MOVE_SELF"] = IN_MOVE_SELF;
	(*state)["FILE_WATCH_MOVED_FROM"] = IN_MOVED_FROM;
	(*state)["FILE_WATCH_MOVED_TO"] = IN_MOVED_TO;
	(*state)["FILE_WATCH_OPEN"] = IN_OPEN;
	(*state)["FILE_WATCH_MOVE"] = IN_MOVE;
	(*state)["FILE_WATCH_CLOSE"] = IN_CLOSE;
	(*state)["FILE_WATCH_DONT_FOLLOW"] = IN_DONT_FOLLOW;
	(*state)["FILE_WATCH_EXCL_UNLINK"] = IN_EXCL_UNLINK;
	(*state)["FILE_WATCH_MASK_ADD"] = IN_MASK_ADD;
	(*state)["FILE_WATCH_ONESHOT"] = IN_ONESHOT;
	(*state)["FILE_WATCH_ONLYDIR"] = IN_ONLYDIR;
	(*state)["FILE_WATCH_IGNORED"] = IN_IGNORED;
	(*state)["FILE_WATCH_ISDIR"] = IN_ISDIR;
	(*state)["FILE_WATCH_Q_OVERFLOW"] = IN_Q_OVERFLOW;
	(*state)["FILE_WATCH_UNMOUNT"] = IN_UNMOUNT;
}

void luaInit(bool redo) {
	std::lock_guard<std::mutex> guard(stateResetMutex);

	Hooks::run = sol::nil;

	if (redo) {
		Console::log(LUA_PREFIX "Resetting state...\n");
		delete server;

		for (int i = 0; i < maxNumberOfAccounts; i++) {
			if (accountDataTables[i]) {
				delete accountDataTables[i];
				accountDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfPlayers; i++) {
			if (playerDataTables[i]) {
				delete playerDataTables[i];
				playerDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfHumans; i++) {
			if (humanDataTables[i]) {
				delete humanDataTables[i];
				humanDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfItems; i++) {
			if (itemDataTables[i]) {
				delete itemDataTables[i];
				itemDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfVehicles; i++) {
			if (vehicleDataTables[i]) {
				delete vehicleDataTables[i];
				vehicleDataTables[i] = nullptr;
			}
		}

		for (int i = 0; i < maxNumberOfRigidBodies; i++) {
			if (bodyDataTables[i]) {
				delete bodyDataTables[i];
				bodyDataTables[i] = nullptr;
			}
		}

		delete lua;
	} else {
		Console::log(LUA_PREFIX "Initializing state...\n");
	}

	lua = new sol::state();

	Console::log(LUA_PREFIX "Defining...\n");
	defineThreadSafeAPIs(lua);

	{
		auto meta = lua->new_usertype<Server>("new", sol::no_constructor);
		meta["TPS"] = &Server::TPS;

		meta["class"] = sol::property(&Server::getClass);
		meta["port"] = sol::property(&Server::getPort);
		meta["name"] = sol::property(&Server::getName, &Server::setName);
		meta["maxBytesPerSecond"] = sol::property(&Server::getMaxBytesPerSecond,
		                                          &Server::setMaxBytesPerSecond);
		meta["adminPassword"] =
		    sol::property(&Server::getAdminPassword, &Server::setAdminPassword);
		meta["password"] =
		    sol::property(&Server::getPassword, &Server::setPassword);
		meta["maxPlayers"] =
		    sol::property(&Server::getMaxPlayers, &Server::setMaxPlayers);

		meta["worldTraffic"] =
		    sol::property(&Server::getWorldTraffic, &Server::setWorldTraffic);
		meta["worldStartCash"] =
		    sol::property(&Server::getWorldStartCash, &Server::setWorldStartCash);
		meta["worldMinCash"] =
		    sol::property(&Server::getWorldMinCash, &Server::setWorldMinCash);
		meta["worldShowJoinExit"] = sol::property(&Server::getWorldShowJoinExit,
		                                          &Server::setWorldShowJoinExit);
		meta["worldRespawnTeam"] = sol::property(&Server::getWorldRespawnTeam,
		                                         &Server::setWorldRespawnTeam);
		meta["worldCrimeCivCiv"] = sol::property(&Server::getWorldCrimeCivCiv,
		                                         &Server::setWorldCrimeCivCiv);
		meta["worldCrimeCivTeam"] = sol::property(&Server::getWorldCrimeCivTeam,
		                                          &Server::setWorldCrimeCivTeam);
		meta["worldCrimeTeamCiv"] = sol::property(&Server::getWorldCrimeTeamCiv,
		                                          &Server::setWorldCrimeTeamCiv);
		meta["worldCrimeTeamTeam"] = sol::property(&Server::getWorldCrimeTeamTeam,
		                                           &Server::setWorldCrimeTeamTeam);
		meta["worldCrimeTeamTeamInBase"] =
		    sol::property(&Server::getWorldCrimeTeamTeamInBase,
		                  &Server::setWorldCrimeTeamTeamInBase);
		meta["worldCrimeNoSpawn"] = sol::property(&Server::getWorldCrimeNoSpawn,
		                                          &Server::setWorldCrimeNoSpawn);

		meta["roundRoundTime"] =
		    sol::property(&Server::getRoundRoundTime, &Server::setRoundRoundTime);
		meta["roundStartCash"] =
		    sol::property(&Server::getRoundStartCash, &Server::setRoundStartCash);
		meta["roundIsWeekly"] =
		    sol::property(&Server::getRoundIsWeekly, &Server::setRoundIsWeekly);
		meta["roundHasBonusRatio"] = sol::property(&Server::getRoundHasBonusRatio,
		                                           &Server::setRoundHasBonusRatio);
		meta["roundTeamDamage"] =
		    sol::property(&Server::getRoundTeamDamage, &Server::setRoundTeamDamage);
		meta["roundWeekDay"] =
		    sol::property(&Server::getRoundWeekDay, &Server::setRoundWeekDay);

		meta["type"] = sol::property(&Server::getType, &Server::setType);
		meta["levelToLoad"] =
		    sol::property(&Server::getLevelName, &Server::setLevelName);
		meta["loadedLevel"] = sol::property(&Server::getLoadedLevelName);
		meta["isLevelLoaded"] =
		    sol::property(&Server::getIsLevelLoaded, &Server::setIsLevelLoaded);
		meta["gravity"] = sol::property(&Server::getGravity, &Server::setGravity);
		meta["defaultGravity"] = sol::property(&Server::getDefaultGravity);
		meta["state"] = sol::property(&Server::getState, &Server::setState);
		meta["time"] = sol::property(&Server::getTime, &Server::setTime);
		meta["sunTime"] = sol::property(&Server::getSunTime, &Server::setSunTime);
		meta["version"] = sol::property(&Server::getVersion);
		meta["versionMajor"] = sol::property(&Server::getVersionMajor);
		meta["versionMinor"] = sol::property(&Server::getVersionMinor);

		meta["setConsoleTitle"] = &Server::setConsoleTitle;
		meta["reset"] = &Server::reset;
	}

	server = new Server();
	(*lua)["server"] = server;

	{
		auto meta = lua->new_usertype<Connection>("new", sol::no_constructor);
		meta["port"] = &Connection::port;
		meta["timeoutTime"] = &Connection::timeoutTime;

		meta["class"] = sol::property(&Connection::getClass);
		meta["address"] = sol::property(&Connection::getAddress);
		meta["adminVisible"] = sol::property(&Connection::getAdminVisible,
		                                     &Connection::setAdminVisible);
		meta["player"] =
		    sol::property(&Connection::getPlayer, &Connection::setPlayer);
		meta["spectatingHuman"] = sol::property(&Connection::getSpectatingHuman);

		meta["getEarShot"] = &Connection::getEarShot;
		meta["hasReceivedEvent"] = &Connection::hasReceivedEvent;
	}

	{
		auto meta = lua->new_usertype<Account>("new", sol::no_constructor);
		meta["subRosaID"] = &Account::subRosaID;
		meta["phoneNumber"] = &Account::phoneNumber;
		meta["money"] = &Account::money;
		meta["corporateRating"] = &Account::corporateRating;
		meta["criminalRating"] = &Account::criminalRating;
		meta["spawnTimer"] = &Account::spawnTimer;
		meta["playTime"] = &Account::playTime;
		meta["banTime"] = &Account::banTime;

		meta["class"] = sol::property(&Account::getClass);
		meta["__tostring"] = &Account::__tostring;
		meta["index"] = sol::property(&Account::getIndex);
		meta["data"] = sol::property(&Account::getDataTable);
		meta["name"] = sol::property(&Account::getName);
		meta["steamID"] = sol::property(&Account::getSteamID);
	}

	{
		auto meta = lua->new_usertype<Voice>("new", sol::no_constructor);
		meta["volumeLevel"] = &Voice::volumeLevel;
		meta["currentFrame"] = &Voice::currentFrame;

		meta["class"] = sol::property(&Voice::getClass);
		meta["isSilenced"] =
		    sol::property(&Voice::getIsSilenced, &Voice::setIsSilenced);

		meta["getFrame"] = &Voice::getFrame;
		meta["setFrame"] = &Voice::setFrame;
	}

	{
		auto meta = lua->new_usertype<Player>("new", sol::no_constructor);
		meta["subRosaID"] = &Player::subRosaID;
		meta["phoneNumber"] = &Player::phoneNumber;
		meta["money"] = &Player::money;
		meta["teamMoney"] = &Player::teamMoney;
		meta["budget"] = &Player::budget;
		meta["corporateRating"] = &Player::corporateRating;
		meta["criminalRating"] = &Player::criminalRating;
		meta["team"] = &Player::team;
		meta["teamSwitchTimer"] = &Player::teamSwitchTimer;
		meta["stocks"] = &Player::stocks;
		meta["spawnTimer"] = &Player::spawnTimer;
		meta["gearX"] = &Player::gearX;
		meta["leftRightInput"] = &Player::leftRightInput;
		meta["gearY"] = &Player::gearY;
		meta["forwardBackInput"] = &Player::forwardBackInput;
		meta["viewYawDelta"] = &Player::viewYawDelta;
		meta["viewPitch"] = &Player::viewPitch;
		meta["freeLookYaw"] = &Player::freeLookYaw;
		meta["freeLookPitch"] = &Player::freeLookPitch;
		meta["viewYaw"] = &Player::viewYaw;
		meta["viewPitchDelta"] = &Player::viewPitchDelta;
		meta["inputFlags"] = &Player::inputFlags;
		meta["lastInputFlags"] = &Player::lastInputFlags;
		meta["zoomLevel"] = &Player::zoomLevel;
		meta["inputType"] = &Player::inputType;
		meta["menuTab"] = &Player::menuTab;
		meta["numActions"] = &Player::numActions;
		meta["lastNumActions"] = &Player::lastNumActions;
		meta["numMenuButtons"] = &Player::numMenuButtons;
		meta["gender"] = &Player::gender;
		meta["skinColor"] = &Player::skinColor;
		meta["hairColor"] = &Player::hairColor;
		meta["hair"] = &Player::hair;
		meta["eyeColor"] = &Player::eyeColor;
		meta["model"] = &Player::model;
		meta["suitColor"] = &Player::suitColor;
		meta["tieColor"] = &Player::tieColor;
		meta["head"] = &Player::head;
		meta["necklace"] = &Player::necklace;

		meta["class"] = sol::property(&Player::getClass);
		meta["__tostring"] = &Player::__tostring;
		meta["index"] = sol::property(&Player::getIndex);
		meta["isActive"] =
		    sol::property(&Player::getIsActive, &Player::setIsActive);
		meta["data"] = sol::property(&Player::getDataTable);
		meta["name"] = sol::property(&Player::getName, &Player::setName);
		meta["isAdmin"] = sol::property(&Player::getIsAdmin, &Player::setIsAdmin);
		meta["isReady"] = sol::property(&Player::getIsReady, &Player::setIsReady);
		meta["isBot"] = sol::property(&Player::getIsBot, &Player::setIsBot);
		meta["isZombie"] =
		    sol::property(&Player::getIsZombie, &Player::setIsZombie);
		meta["human"] = sol::property(&Player::getHuman, &Player::setHuman);
		meta["connection"] = sol::property(&Player::getConnection);
		meta["account"] = sol::property(&Player::getAccount, &Player::setAccount);
		meta["voice"] = sol::property(&Player::getVoice);
		meta["botDestination"] =
		    sol::property(&Player::getBotDestination, &Player::setBotDestination);

		meta["getAction"] = &Player::getAction;
		meta["getMenuButton"] = &Player::getMenuButton;
		meta["update"] = &Player::update;
		meta["updateFinance"] = &Player::updateFinance;
		meta["remove"] = &Player::remove;
		meta["sendMessage"] = &Player::sendMessage;
	}

	{
		auto meta = lua->new_usertype<Human>("new", sol::no_constructor);
		meta["stamina"] = &Human::stamina;
		meta["maxStamina"] = &Human::maxStamina;
		meta["vehicleSeat"] = &Human::vehicleSeat;
		meta["despawnTime"] = &Human::despawnTime;
		meta["movementState"] = &Human::movementState;
		meta["zoomLevel"] = &Human::zoomLevel;
		meta["damage"] = &Human::damage;
		meta["pos"] = &Human::pos;
		meta["viewYaw"] = &Human::viewYaw;
		meta["viewPitch"] = &Human::viewPitch;
		meta["viewYaw2"] = &Human::viewYaw2;
		meta["strafeInput"] = &Human::strafeInput;
		meta["walkInput"] = &Human::walkInput;
		meta["viewPitch2"] = &Human::viewPitch2;
		meta["inputFlags"] = &Human::inputFlags;
		meta["lastInputFlags"] = &Human::lastInputFlags;
		meta["health"] = &Human::health;
		meta["bloodLevel"] = &Human::bloodLevel;
		meta["chestHP"] = &Human::chestHP;
		meta["headHP"] = &Human::headHP;
		meta["leftArmHP"] = &Human::leftArmHP;
		meta["rightArmHP"] = &Human::rightArmHP;
		meta["leftLegHP"] = &Human::leftLegHP;
		meta["rightLegHP"] = &Human::rightLegHP;
		meta["progressBar"] = &Human::progressBar;
		meta["inventoryAnimationFlags"] = &Human::inventoryAnimationFlags;
		meta["inventoryAnimationProgress"] = &Human::inventoryAnimationProgress;
		meta["inventoryAnimationDuration"] = &Human::inventoryAnimationDuration;
		meta["inventoryAnimationHand"] = &Human::inventoryAnimationHand;
		meta["inventoryAnimationSlot"] = &Human::inventoryAnimationSlot;
		meta["inventoryAnimationCounterFinished"] =
		    &Human::inventoryAnimationCounterFinished;
		meta["inventoryAnimationCounter"] = &Human::inventoryAnimationCounter;
		meta["gender"] = &Human::gender;
		meta["head"] = &Human::head;
		meta["skinColor"] = &Human::skinColor;
		meta["hairColor"] = &Human::hairColor;
		meta["hair"] = &Human::hair;
		meta["eyeColor"] = &Human::eyeColor;
		meta["model"] = &Human::model;
		meta["suitColor"] = &Human::suitColor;
		meta["tieColor"] = &Human::tieColor;
		meta["necklace"] = &Human::necklace;
		meta["lastUpdatedWantedGroup"] = &Human::lastUpdatedWantedGroup;

		meta["class"] = sol::property(&Human::getClass);
		meta["__tostring"] = &Human::__tostring;
		meta["index"] = sol::property(&Human::getIndex);
		meta["isActive"] = sol::property(&Human::getIsActive, &Human::setIsActive);
		meta["data"] = sol::property(&Human::getDataTable);
		meta["isAlive"] = sol::property(&Human::getIsAlive, &Human::setIsAlive);
		meta["isImmortal"] =
		    sol::property(&Human::getIsImmortal, &Human::setIsImmortal);
		meta["isOnGround"] = sol::property(&Human::getIsOnGround);
		meta["isStanding"] = sol::property(&Human::getIsStanding);
		meta["isBleeding"] =
		    sol::property(&Human::getIsBleeding, &Human::setIsBleeding);
		meta["player"] = sol::property(&Human::getPlayer, &Human::setPlayer);
		meta["account"] = sol::property(&Human::getAccount, &Human::setAccount);
		meta["vehicle"] = sol::property(&Human::getVehicle, &Human::setVehicle);

		meta["remove"] = &Human::remove;
		meta["teleport"] = &Human::teleport;
		meta["speak"] = &Human::speak;
		meta["arm"] = &Human::arm;
		meta["getBone"] = &Human::getBone;
		meta["getRigidBody"] = &Human::getRigidBody;
		meta["getInventorySlot"] = &Human::getInventorySlot;
		meta["setVelocity"] = &Human::setVelocity;
		meta["addVelocity"] = &Human::addVelocity;
		meta["mountItem"] = &Human::mountItem;
		meta["applyDamage"] = &Human::applyDamage;
	}

	{
		auto meta = lua->new_usertype<ItemType>("new", sol::no_constructor);
		meta["price"] = &ItemType::price;
		meta["mass"] = &ItemType::mass;
		meta["fireRate"] = &ItemType::fireRate;
		meta["magazineAmmo"] = &ItemType::magazineAmmo;
		meta["bulletType"] = &ItemType::bulletType;
		meta["bulletVelocity"] = &ItemType::bulletVelocity;
		meta["bulletSpread"] = &ItemType::bulletSpread;
		meta["numHands"] = &ItemType::numHands;
		meta["rightHandPos"] = &ItemType::rightHandPos;
		meta["leftHandPos"] = &ItemType::leftHandPos;
		meta["primaryGripStiffness"] = &ItemType::primaryGripStiffness;
		meta["primaryGripRotation"] = &ItemType::primaryGripRotation;
		meta["secondaryGripStiffness"] = &ItemType::secondaryGripStiffness;
		meta["secondaryGripRotation"] = &ItemType::secondaryGripRotation;
		meta["boundsCenter"] = &ItemType::boundsCenter;
		meta["gunHoldingPos"] = &ItemType::gunHoldingPos;

		meta["class"] = sol::property(&ItemType::getClass);
		meta["__tostring"] = &ItemType::__tostring;
		meta["index"] = sol::property(&ItemType::getIndex);
		meta["name"] = sol::property(&ItemType::getName, &ItemType::setName);
		meta["isGun"] = sol::property(&ItemType::getIsGun, &ItemType::setIsGun);

		meta["getCanMountTo"] = &ItemType::getCanMountTo;
		meta["setCanMountTo"] = &ItemType::setCanMountTo;
	}

	{
		auto meta = lua->new_usertype<Item>("new", sol::no_constructor);
		meta["physicsSettledTimer"] = &Item::physicsSettledTimer;
		meta["despawnTime"] = &Item::despawnTime;
		meta["parentSlot"] = &Item::parentSlot;
		meta["pos"] = &Item::pos;
		meta["vel"] = &Item::vel;
		meta["rot"] = &Item::rot;
		meta["bullets"] = &Item::bullets;
		meta["cooldown"] = &Item::cooldown;
		meta["cashSpread"] = &Item::cashSpread;
		meta["cashAmount"] = &Item::cashBillAmount;
		meta["cashPureValue"] = &Item::cashPureValue;
		meta["phoneNumber"] = &Item::phoneNumber;
		meta["displayPhoneNumber"] = &Item::displayPhoneNumber;
		meta["enteredPhoneNumber"] = &Item::enteredPhoneNumber;
		meta["phoneTexture"] = &Item::phoneTexture;
		meta["computerCurrentLine"] = &Item::computerCurrentLine;
		meta["computerTopLine"] = &Item::computerTopLine;
		meta["computerCursor"] = &Item::computerCursor;

		meta["class"] = sol::property(&Item::getClass);
		meta["__tostring"] = &Item::__tostring;
		meta["index"] = sol::property(&Item::getIndex);
		meta["isActive"] = sol::property(&Item::getIsActive, &Item::setIsActive);
		meta["data"] = sol::property(&Item::getDataTable);
		meta["hasPhysics"] =
		    sol::property(&Item::getHasPhysics, &Item::setHasPhysics);
		meta["physicsSettled"] =
		    sol::property(&Item::getPhysicsSettled, &Item::setPhysicsSettled);
		meta["isStatic"] = sol::property(&Item::getIsStatic, &Item::setIsStatic);
		meta["type"] = sol::property(&Item::getType, &Item::setType);
		meta["rigidBody"] = sol::property(&Item::getRigidBody);
		meta["connectedPhone"] =
		    sol::property(&Item::getConnectedPhone, &Item::setConnectedPhone);
		meta["vehicle"] = sol::property(&Item::getVehicle, &Item::setVehicle);
		meta["grenadePrimer"] =
		    sol::property(&Item::getGrenadePrimer, &Item::setGrenadePrimer);
		meta["parentHuman"] = sol::property(&Item::getParentHuman);
		meta["parentItem"] = sol::property(&Item::getParentItem);

		meta["remove"] = &Item::remove;
		meta["mountItem"] = &Item::mountItem;
		meta["unmount"] = &Item::unmount;
		meta["speak"] = &Item::speak;
		meta["explode"] = &Item::explode;
		meta["setMemo"] = &Item::setMemo;
		meta["computerTransmitLine"] = &Item::computerTransmitLine;
		meta["computerIncrementLine"] = &Item::computerIncrementLine;
		meta["computerSetLine"] = &Item::computerSetLine;
		meta["computerSetLineColors"] = &Item::computerSetLineColors;
		meta["computerSetColor"] = &Item::computerSetColor;
		meta["cashAddBill"] = &Item::cashAddBill;
		meta["cashRemoveBill"] = &Item::cashRemoveBill;
		meta["cashGetBillValue"] = &Item::cashGetBillValue;
	}

	{
		auto meta = lua->new_usertype<VehicleType>("new", sol::no_constructor);
		meta["controllableState"] = &VehicleType::controllableState;
		meta["price"] = &VehicleType::price;
		meta["mass"] = &VehicleType::mass;

		meta["class"] = sol::property(&VehicleType::getClass);
		meta["__tostring"] = &VehicleType::__tostring;
		meta["index"] = sol::property(&VehicleType::getIndex);
		meta["name"] = sol::property(&VehicleType::getName, &VehicleType::setName);
		meta["usesExternalModel"] =
		    sol::property(&VehicleType::getUsesExternalModel);
	}

	{
		auto meta = lua->new_usertype<Vehicle>("new", sol::no_constructor);
		meta["controllableState"] = &Vehicle::controllableState;
		meta["health"] = &Vehicle::health;
		meta["color"] = &Vehicle::color;
		meta["pos"] = &Vehicle::pos;
		meta["pos2"] = &Vehicle::pos2;
		meta["rot"] = &Vehicle::rot;
		meta["vel"] = &Vehicle::vel;
		meta["gearX"] = &Vehicle::gearX;
		meta["steerControl"] = &Vehicle::steerControl;
		meta["gearY"] = &Vehicle::gearY;
		meta["gasControl"] = &Vehicle::gasControl;
		meta["engineRPM"] = &Vehicle::engineRPM;
		meta["bladeBodyID"] = &Vehicle::bladeBodyID;
		meta["numSeats"] = &Vehicle::numSeats;

		meta["class"] = sol::property(&Vehicle::getClass);
		meta["__tostring"] = &Vehicle::__tostring;
		meta["index"] = sol::property(&Vehicle::getIndex);
		meta["isActive"] =
		    sol::property(&Vehicle::getIsActive, &Vehicle::setIsActive);
		meta["type"] = sol::property(&Vehicle::getType, &Vehicle::setType);
		meta["isLocked"] =
		    sol::property(&Vehicle::getIsLocked, &Vehicle::setIsLocked);
		meta["data"] = sol::property(&Vehicle::getDataTable);
		meta["lastDriver"] = sol::property(&Vehicle::getLastDriver);
		meta["rigidBody"] = sol::property(&Vehicle::getRigidBody);
		meta["trafficCar"] =
		    sol::property(&Vehicle::getTrafficCar, &Vehicle::setTrafficCar);

		meta["updateType"] = &Vehicle::updateType;
		meta["updateDestruction"] = &Vehicle::updateDestruction;
		meta["remove"] = &Vehicle::remove;
		meta["getIsWindowBroken"] = &Vehicle::getIsWindowBroken;
		meta["setIsWindowBroken"] = &Vehicle::setIsWindowBroken;
	}

	{
		auto meta = lua->new_usertype<Bullet>("new", sol::no_constructor);
		meta["type"] = &Bullet::type;
		meta["time"] = &Bullet::time;
		meta["lastPos"] = &Bullet::lastPos;
		meta["pos"] = &Bullet::pos;
		meta["vel"] = &Bullet::vel;

		meta["class"] = sol::property(&Bullet::getClass);
		meta["player"] = sol::property(&Bullet::getPlayer);
	}

	{
		auto meta = lua->new_usertype<Bone>("new", sol::no_constructor);
		meta["pos"] = &Bone::pos;
		meta["pos2"] = &Bone::pos2;
		meta["vel"] = &Bone::vel;
		meta["rot"] = &Bone::rot;

		meta["class"] = sol::property(&Bone::getClass);
	}

	{
		auto meta = lua->new_usertype<RigidBody>("new", sol::no_constructor);
		meta["type"] = &RigidBody::type;
		meta["unk0"] = &RigidBody::unk0;
		meta["mass"] = &RigidBody::mass;
		meta["pos"] = &RigidBody::pos;
		meta["vel"] = &RigidBody::vel;
		meta["rot"] = &RigidBody::rot;
		meta["rotVel"] = &RigidBody::rotVel;

		meta["class"] = sol::property(&RigidBody::getClass);
		meta["__tostring"] = &RigidBody::__tostring;
		meta["index"] = sol::property(&RigidBody::getIndex);
		meta["isActive"] =
		    sol::property(&RigidBody::getIsActive, &RigidBody::setIsActive);
		meta["data"] = sol::property(&RigidBody::getDataTable);
		meta["isSettled"] =
		    sol::property(&RigidBody::getIsSettled, &RigidBody::setIsSettled);

		meta["bondTo"] = &RigidBody::bondTo;
		meta["bondRotTo"] = &RigidBody::bondRotTo;
		meta["bondToLevel"] = &RigidBody::bondToLevel;
		meta["collideLevel"] = &RigidBody::collideLevel;
	}

	{
		auto meta = lua->new_usertype<InventorySlot>("new", sol::no_constructor);
		meta["count"] = &InventorySlot::count;

		meta["class"] = sol::property(&InventorySlot::getClass);

		meta["primaryItem"] = sol::property(&InventorySlot::getPrimaryItem);
		meta["secondaryItem"] = sol::property(&InventorySlot::getSecondaryItem);
	}

	{
		auto meta = lua->new_usertype<Bond>("new", sol::no_constructor);
		meta["type"] = &Bond::type;
		meta["despawnTime"] = &Bond::despawnTime;
		meta["globalPos"] = &Bond::globalPos;
		meta["localPos"] = &Bond::localPos;
		meta["otherLocalPos"] = &Bond::otherLocalPos;

		meta["class"] = sol::property(&Bond::getClass);
		meta["__tostring"] = &Bond::__tostring;
		meta["index"] = sol::property(&Bond::getIndex);
		meta["isActive"] = sol::property(&Bond::getIsActive, &Bond::setIsActive);
		meta["body"] = sol::property(&Bond::getBody);
		meta["otherBody"] = sol::property(&Bond::getOtherBody);
	}

	{
		auto meta = lua->new_usertype<Action>("new", sol::no_constructor);
		meta["type"] = &Action::type;
		meta["a"] = &Action::a;
		meta["b"] = &Action::b;
		meta["c"] = &Action::c;
		meta["d"] = &Action::d;

		meta["class"] = sol::property(&Action::getClass);
	}

	{
		auto meta = lua->new_usertype<MenuButton>("new", sol::no_constructor);
		meta["id"] = &MenuButton::id;
		meta["text"] = sol::property(&MenuButton::getText, &MenuButton::setText);

		meta["class"] = sol::property(&MenuButton::getClass);
	}

	{
		auto meta = lua->new_usertype<EarShot>("new", sol::no_constructor);
		meta["distance"] = &EarShot::distance;
		meta["volume"] = &EarShot::volume;

		meta["class"] = sol::property(&EarShot::getClass);
		meta["isActive"] =
		    sol::property(&EarShot::getIsActive, &EarShot::setIsActive);
		meta["player"] = sol::property(&EarShot::getPlayer, &EarShot::setPlayer);
		meta["human"] = sol::property(&EarShot::getHuman, &EarShot::setHuman);
		meta["receivingItem"] =
		    sol::property(&EarShot::getReceivingItem, &EarShot::setReceivingItem);
		meta["transmittingItem"] = sol::property(&EarShot::getTransmittingItem,
		                                         &EarShot::setTransmittingItem);
	}

	{
		auto meta = lua->new_usertype<Worker>(
		    "Worker", sol::constructors<Worker(std::string)>());
		meta["stop"] = &Worker::stop;
		meta["sendMessage"] = &Worker::sendMessage;
		meta["receiveMessage"] = &Worker::receiveMessage;
	}

	{
		auto meta = lua->new_usertype<ChildProcess>(
		    "ChildProcess", sol::constructors<ChildProcess(const char*)>());
		meta["isRunning"] = &ChildProcess::isRunning;
		meta["terminate"] = &ChildProcess::terminate;
		meta["getExitCode"] = &ChildProcess::getExitCode;
		meta["receiveMessage"] = &ChildProcess::receiveMessage;
		meta["sendMessage"] = &ChildProcess::sendMessage;
		meta["setCPULimit"] = &ChildProcess::setCPULimit;
		meta["setMemoryLimit"] = &ChildProcess::setMemoryLimit;
		meta["setFileSizeLimit"] = &ChildProcess::setFileSizeLimit;
		meta["getPriority"] = &ChildProcess::getPriority;
		meta["setPriority"] = &ChildProcess::setPriority;
	}

	{
		auto meta = lua->new_usertype<StreetLane>("new", sol::no_constructor);
		meta["direction"] = &StreetLane::direction;
		meta["posA"] = &StreetLane::posA;
		meta["posB"] = &StreetLane::posB;

		meta["class"] = sol::property(&StreetLane::getClass);
	}

	{
		auto meta = lua->new_usertype<Street>("new", sol::no_constructor);
		meta["trafficCuboidA"] = &Street::trafficCuboidA;
		meta["trafficCuboidB"] = &Street::trafficCuboidB;
		meta["numTraffic"] = &Street::numTraffic;

		meta["class"] = sol::property(&Street::getClass);
		meta["__tostring"] = &Street::__tostring;
		meta["index"] = sol::property(&Street::getIndex);
		meta["name"] = sol::property(&Street::getName);
		meta["intersectionA"] = sol::property(&Street::getIntersectionA);
		meta["intersectionB"] = sol::property(&Street::getIntersectionB);
		meta["numLanes"] = sol::property(&Street::getNumLanes);

		meta["getLane"] = &Street::getLane;
	}

	{
		auto meta =
		    lua->new_usertype<StreetIntersection>("new", sol::no_constructor);
		meta["pos"] = &StreetIntersection::pos;
		meta["lightsState"] = &StreetIntersection::lightsState;
		meta["lightsTimer"] = &StreetIntersection::lightsTimer;
		meta["lightsTimerMax"] = &StreetIntersection::lightsTimerMax;
		meta["lightEast"] = &StreetIntersection::lightEast;
		meta["lightSouth"] = &StreetIntersection::lightSouth;
		meta["lightWest"] = &StreetIntersection::lightWest;
		meta["lightNorth"] = &StreetIntersection::lightNorth;

		meta["class"] = sol::property(&StreetIntersection::getClass);
		meta["__tostring"] = &StreetIntersection::__tostring;
		meta["index"] = sol::property(&StreetIntersection::getIndex);
		meta["streetEast"] = sol::property(&StreetIntersection::getStreetEast);
		meta["streetSouth"] = sol::property(&StreetIntersection::getStreetSouth);
		meta["streetWest"] = sol::property(&StreetIntersection::getStreetWest);
		meta["streetNorth"] = sol::property(&StreetIntersection::getStreetNorth);
	}

	{
		auto meta = lua->new_usertype<TrafficCar>("new", sol::no_constructor);
		meta["pos"] = &TrafficCar::pos;
		meta["vel"] = &TrafficCar::vel;
		meta["yaw"] = &TrafficCar::yaw;
		meta["rot"] = &TrafficCar::rot;
		meta["color"] = &TrafficCar::color;
		meta["state"] = &TrafficCar::state;

		meta["class"] = sol::property(&TrafficCar::getClass);
		meta["__tostring"] = &TrafficCar::__tostring;
		meta["index"] = sol::property(&TrafficCar::getIndex);
		meta["type"] = sol::property(&TrafficCar::getType, &TrafficCar::setType);
		meta["human"] = sol::property(&TrafficCar::getHuman, &TrafficCar::setHuman);
		meta["isBot"] = sol::property(&TrafficCar::getIsBot, &TrafficCar::setIsBot);
		meta["isAggressive"] = sol::property(&TrafficCar::getIsAggressive,
		                                     &TrafficCar::setIsAggressive);
		meta["vehicle"] =
		    sol::property(&TrafficCar::getVehicle, &TrafficCar::setVehicle);
	}

	{
		auto meta = lua->new_usertype<ShopCar>("new", sol::no_constructor);
		meta["price"] = &ShopCar::price;
		meta["color"] = &ShopCar::color;

		meta["class"] = sol::property(&ShopCar::getClass);
		meta["type"] = sol::property(&ShopCar::getType, &ShopCar::setType);
	}

	{
		auto meta = lua->new_usertype<Building>("new", sol::no_constructor);
		meta["type"] = &Building::type;
		meta["pos"] = &Building::pos;
		meta["spawnRot"] = &Building::spawnRot;
		meta["interiorCuboidA"] = &Building::interiorCuboidA;
		meta["interiorCuboidB"] = &Building::interiorCuboidB;
		meta["numShopCars"] = &Building::numShopCars;
		meta["shopCarSales"] = &Building::shopCarSales;

		meta["class"] = sol::property(&Building::getClass);
		meta["__tostring"] = &Building::__tostring;
		meta["index"] = sol::property(&Building::getIndex);

		meta["getShopCar"] = &Building::getShopCar;
	}

	{
		auto meta = lua->new_usertype<Event>("new", sol::no_constructor);
		meta["type"] = &Event::type;
		meta["tickCreated"] = &Event::tickCreated;
		meta["vectorA"] = &Event::vectorA;
		meta["vectorB"] = &Event::vectorB;
		meta["a"] = &Event::a;
		meta["b"] = &Event::b;
		meta["c"] = &Event::c;
		meta["d"] = &Event::d;
		meta["floatA"] = &Event::floatA;
		meta["floatB"] = &Event::floatB;

		meta["class"] = sol::property(&Event::getClass);
		meta["__tostring"] = &Event::__tostring;
		meta["index"] = sol::property(&Event::getIndex);
		meta["message"] = sol::property(&Event::getMessage, &Event::setMessage);
	}

	{
		auto meta = lua->new_usertype<Hooks::Float>("new", sol::no_constructor);
		meta["value"] = &Hooks::Float::value;
	}

	{
		auto meta = lua->new_usertype<Hooks::Integer>("new", sol::no_constructor);
		meta["value"] = &Hooks::Integer::value;
	}

	{
		auto meta =
		    lua->new_usertype<Hooks::UnsignedInteger>("new", sol::no_constructor);
		meta["value"] = &Hooks::UnsignedInteger::value;
	}

	(*lua)["flagStateForReset"] = Lua::flagStateForReset;

	{
		auto hookTable = lua->create_table();
		(*lua)["hook"] = hookTable;
		hookTable["persistentMode"] = hookMode;
		hookTable["enable"] = Lua::hook::enable;
		hookTable["disable"] = Lua::hook::disable;
		hookTable["clear"] = Lua::hook::clear;
		Lua::hook::clear();
	}

	{
		auto physicsTable = lua->create_table();
		(*lua)["physics"] = physicsTable;
		physicsTable["lineIntersectLevel"] = Lua::physics::lineIntersectLevel;
		physicsTable["lineIntersectHuman"] = Lua::physics::lineIntersectHuman;
		physicsTable["lineIntersectVehicle"] = Lua::physics::lineIntersectVehicle;
		physicsTable["lineIntersectLevelQuick"] =
		    Lua::physics::lineIntersectLevelQuick;
		physicsTable["lineIntersectHumanQuick"] =
		    Lua::physics::lineIntersectHumanQuick;
		physicsTable["lineIntersectVehicleQuick"] =
		    Lua::physics::lineIntersectVehicleQuick;
		physicsTable["lineIntersectAnyQuick"] = Lua::physics::lineIntersectAnyQuick;
		physicsTable["lineIntersectTriangle"] = Lua::physics::lineIntersectTriangle;
		physicsTable["garbageCollectBullets"] = Lua::physics::garbageCollectBullets;
		physicsTable["createBlock"] = Lua::physics::createBlock;
		physicsTable["deleteBlock"] = Lua::physics::deleteBlock;
	}

	{
		auto chatTable = lua->create_table();
		(*lua)["chat"] = chatTable;
		chatTable["announce"] = Lua::chat::announce;
		chatTable["tellAdmins"] = Lua::chat::tellAdmins;
	}

	{
		auto accountsTable = lua->create_table();
		(*lua)["accounts"] = accountsTable;
		accountsTable["save"] = Lua::accounts::save;
		accountsTable["getCount"] = Lua::accounts::getCount;
		accountsTable["getAll"] = Lua::accounts::getAll;
		accountsTable["getByPhone"] = Lua::accounts::getByPhone;

		sol::table _meta = lua->create_table();
		accountsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::accounts::getCount;
		_meta["__index"] = Lua::accounts::getByIndex;
	}

	{
		auto playersTable = lua->create_table();
		(*lua)["players"] = playersTable;
		playersTable["getCount"] = Lua::players::getCount;
		playersTable["getAll"] = Lua::players::getAll;
		playersTable["getByPhone"] = Lua::players::getByPhone;
		playersTable["getNonBots"] = Lua::players::getNonBots;
		playersTable["getBots"] = Lua::players::getBots;
		playersTable["createBot"] = Lua::players::createBot;

		sol::table _meta = lua->create_table();
		playersTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::players::getCount;
		_meta["__index"] = Lua::players::getByIndex;
	}

	{
		auto humansTable = lua->create_table();
		(*lua)["humans"] = humansTable;
		humansTable["getCount"] = Lua::humans::getCount;
		humansTable["getAll"] = Lua::humans::getAll;
		humansTable["create"] = Lua::humans::create;

		sol::table _meta = lua->create_table();
		humansTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::humans::getCount;
		_meta["__index"] = Lua::humans::getByIndex;
	}

	{
		auto itemTypesTable = lua->create_table();
		(*lua)["itemTypes"] = itemTypesTable;
		itemTypesTable["getCount"] = Lua::itemTypes::getCount;
		itemTypesTable["getAll"] = Lua::itemTypes::getAll;
		itemTypesTable["getByName"] = Lua::itemTypes::getByName;

		sol::table _meta = lua->create_table();
		itemTypesTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::itemTypes::getCount;
		_meta["__index"] = Lua::itemTypes::getByIndex;
	}

	{
		auto itemsTable = lua->create_table();
		(*lua)["items"] = itemsTable;
		itemsTable["getCount"] = Lua::items::getCount;
		itemsTable["getAll"] = Lua::items::getAll;
		itemsTable["create"] =
		    sol::overload(Lua::items::create, Lua::items::createVel);
		itemsTable["createRope"] = Lua::items::createRope;

		sol::table _meta = lua->create_table();
		itemsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::items::getCount;
		_meta["__index"] = Lua::items::getByIndex;
	}

	{
		auto vehicleTypesTable = lua->create_table();
		(*lua)["vehicleTypes"] = vehicleTypesTable;
		vehicleTypesTable["getCount"] = Lua::vehicleTypes::getCount;
		vehicleTypesTable["getAll"] = Lua::vehicleTypes::getAll;
		vehicleTypesTable["getByName"] = Lua::vehicleTypes::getByName;

		sol::table _meta = lua->create_table();
		vehicleTypesTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::vehicleTypes::getCount;
		_meta["__index"] = Lua::vehicleTypes::getByIndex;
	}

	{
		auto vehiclesTable = lua->create_table();
		(*lua)["vehicles"] = vehiclesTable;
		vehiclesTable["getCount"] = Lua::vehicles::getCount;
		vehiclesTable["getAll"] = Lua::vehicles::getAll;
		vehiclesTable["create"] =
		    sol::overload(Lua::vehicles::create, Lua::vehicles::createVel);

		sol::table _meta = lua->create_table();
		vehiclesTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::vehicles::getCount;
		_meta["__index"] = Lua::vehicles::getByIndex;
	}

	{
		auto bulletsTable = lua->create_table();
		(*lua)["bullets"] = bulletsTable;
		bulletsTable["getCount"] = Lua::bullets::getCount;
		bulletsTable["getAll"] = Lua::bullets::getAll;
		bulletsTable["create"] = Lua::bullets::create;
	}

	{
		auto rigidBodiesTable = lua->create_table();
		(*lua)["rigidBodies"] = rigidBodiesTable;
		rigidBodiesTable["getCount"] = Lua::rigidBodies::getCount;
		rigidBodiesTable["getAll"] = Lua::rigidBodies::getAll;

		sol::table _meta = lua->create_table();
		rigidBodiesTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::rigidBodies::getCount;
		_meta["__index"] = Lua::rigidBodies::getByIndex;
	}

	{
		auto bondsTable = lua->create_table();
		(*lua)["bonds"] = bondsTable;
		bondsTable["getCount"] = Lua::bonds::getCount;
		bondsTable["getAll"] = Lua::bonds::getAll;

		sol::table _meta = lua->create_table();
		bondsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::bonds::getCount;
		_meta["__index"] = Lua::bonds::getByIndex;
	}

	{
		auto streetsTable = lua->create_table();
		(*lua)["streets"] = streetsTable;
		streetsTable["getCount"] = Lua::streets::getCount;
		streetsTable["getAll"] = Lua::streets::getAll;

		sol::table _meta = lua->create_table();
		streetsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::streets::getCount;
		_meta["__index"] = Lua::streets::getByIndex;
	}

	{
		auto intersectionsTable = lua->create_table();
		(*lua)["intersections"] = intersectionsTable;
		intersectionsTable["getCount"] = Lua::intersections::getCount;
		intersectionsTable["getAll"] = Lua::intersections::getAll;

		sol::table _meta = lua->create_table();
		intersectionsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::intersections::getCount;
		_meta["__index"] = Lua::intersections::getByIndex;
	}

	{
		auto trafficCarsTable = lua->create_table();
		(*lua)["trafficCars"] = trafficCarsTable;
		trafficCarsTable["getCount"] = Lua::trafficCars::getCount;
		trafficCarsTable["getAll"] = Lua::trafficCars::getAll;
		trafficCarsTable["createMany"] = Lua::trafficCars::createMany;

		sol::table _meta = lua->create_table();
		trafficCarsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::trafficCars::getCount;
		_meta["__index"] = Lua::trafficCars::getByIndex;
	}

	{
		auto buildingsTable = lua->create_table();
		(*lua)["buildings"] = buildingsTable;
		buildingsTable["getCount"] = Lua::buildings::getCount;
		buildingsTable["getAll"] = Lua::buildings::getAll;

		sol::table _meta = lua->create_table();
		buildingsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::buildings::getCount;
		_meta["__index"] = Lua::buildings::getByIndex;
	}

	{
		auto eventsTable = lua->create_table();
		(*lua)["events"] = eventsTable;
		eventsTable["getCount"] = Lua::events::getCount;
		eventsTable["getAll"] = Lua::events::getAll;
		eventsTable["createBullet"] = Lua::events::createBullet;
		eventsTable["createBulletHit"] = Lua::events::createBulletHit;
		eventsTable["createMessage"] = Lua::events::createMessage;
		eventsTable["createSound"] =
		    sol::overload(Lua::events::createSound, Lua::events::createSoundSimple);
		eventsTable["createExplosion"] = Lua::events::createExplosion;

		sol::table _meta = lua->create_table();
		eventsTable[sol::metatable_key] = _meta;
		_meta["__len"] = Lua::events::getCount;
		_meta["__index"] = Lua::events::getByIndex;
	}

	{
		auto memoryTable = lua->create_table();
		(*lua)["memory"] = memoryTable;
		memoryTable["getBaseAddress"] = Lua::memory::getBaseAddress;
		memoryTable["getAddress"] = sol::overload(
		    &Lua::memory::getAddressOfConnection, &Lua::memory::getAddressOfAccount,
		    &Lua::memory::getAddressOfPlayer, &Lua::memory::getAddressOfHuman,
		    &Lua::memory::getAddressOfItemType, &Lua::memory::getAddressOfItem,
		    &Lua::memory::getAddressOfVehicleType,
		    &Lua::memory::getAddressOfVehicle, &Lua::memory::getAddressOfBullet,
		    &Lua::memory::getAddressOfBone, &Lua::memory::getAddressOfRigidBody,
		    &Lua::memory::getAddressOfBond, &Lua::memory::getAddressOfAction,
		    &Lua::memory::getAddressOfMenuButton,
		    &Lua::memory::getAddressOfStreetLane, &Lua::memory::getAddressOfStreet,
		    &Lua::memory::getAddressOfStreetIntersection,
		    &Lua::memory::getAddressOfInventorySlot);
		memoryTable["readByte"] = Lua::memory::readByte;
		memoryTable["readUByte"] = Lua::memory::readUByte;
		memoryTable["readShort"] = Lua::memory::readShort;
		memoryTable["readUShort"] = Lua::memory::readUShort;
		memoryTable["readInt"] = Lua::memory::readInt;
		memoryTable["readUInt"] = Lua::memory::readUInt;
		memoryTable["readLong"] = Lua::memory::readLong;
		memoryTable["readULong"] = Lua::memory::readULong;
		memoryTable["readFloat"] = Lua::memory::readFloat;
		memoryTable["readDouble"] = Lua::memory::readDouble;
		memoryTable["readBytes"] = Lua::memory::readBytes;
		memoryTable["writeByte"] = Lua::memory::writeByte;
		memoryTable["writeUByte"] = Lua::memory::writeUByte;
		memoryTable["writeShort"] = Lua::memory::writeShort;
		memoryTable["writeUShort"] = Lua::memory::writeUShort;
		memoryTable["writeInt"] = Lua::memory::writeInt;
		memoryTable["writeUInt"] = Lua::memory::writeUInt;
		memoryTable["writeLong"] = Lua::memory::writeLong;
		memoryTable["writeULong"] = Lua::memory::writeULong;
		memoryTable["writeFloat"] = Lua::memory::writeFloat;
		memoryTable["writeDouble"] = Lua::memory::writeDouble;
		memoryTable["writeBytes"] = Lua::memory::writeBytes;
	}

	(*lua)["RESET_REASON_BOOT"] = RESET_REASON_BOOT;
	(*lua)["RESET_REASON_ENGINECALL"] = RESET_REASON_ENGINECALL;
	(*lua)["RESET_REASON_LUARESET"] = RESET_REASON_LUARESET;
	(*lua)["RESET_REASON_LUACALL"] = RESET_REASON_LUACALL;

	(*lua)["STATE_PREGAME"] = 1;
	(*lua)["STATE_GAME"] = 2;
	(*lua)["STATE_RESTARTING"] = 3;

	(*lua)["TYPE_DRIVING"] = 1;
	(*lua)["TYPE_RACE"] = 2;
	(*lua)["TYPE_ROUND"] = 3;
	(*lua)["TYPE_WORLD"] = 4;
	(*lua)["TYPE_TERMINATOR"] = 5;
	(*lua)["TYPE_COOP"] = 6;
	(*lua)["TYPE_VERSUS"] = 7;

	Console::log(LUA_PREFIX "Running " LUA_ENTRY_FILE "...\n");

	sol::load_result load = lua->load_file(LUA_ENTRY_FILE);
	if (noLuaCallError(&load)) {
		sol::protected_function_result res = load();
		if (noLuaCallError(&res)) {
			Hooks::run = (*lua)["hook"]["run"];
			Console::log(LUA_PREFIX "No problems!\n");
			if (Hooks::run == sol::nil) {
				Console::log(LUA_PREFIX "To use hooks, define hook.run!\n");
			}
		}
	}
}

static inline uintptr_t getBaseAddress() {
	std::ifstream file("/proc/self/maps");
	std::string line;
	// First line
	std::getline(file, line);
	auto pos = line.find("-");
	auto truncated = line.substr(0, pos);

	return std::stoul(truncated, nullptr, 16);
}

static inline void printBaseAddress(uintptr_t base) {
	std::ostringstream stream;

	stream << RS_PREFIX "Base address is ";
	stream << std::showbase << std::hex;
	stream << base;
	stream << "\n";

	Console::log(stream.str());
}

static inline void locateMemory(uintptr_t base) {
	Engine::version = (unsigned int*)(base + 0x2e9f08);
	Engine::subVersion = (unsigned int*)(base + 0x2e9f04);
	Engine::serverName = (char*)(base + 0x250ec1d4);
	Engine::serverPort = (unsigned int*)(base + 0x18db02a0);
	Engine::packetSize = (int*)(base + 0x39075c7c);
	Engine::packet = (unsigned char*)(base + 0x39075c84);
	Engine::serverMaxBytesPerSecond = (int*)(base + 0x18db02a4);
	Engine::adminPassword = (char*)(base + 0x18db06ac);
	Engine::isPassworded = (int*)(base + 0x250ec5f0);
	Engine::password = (char*)(base + 0x18db08ac);
	Engine::maxPlayers = (int*)(base + 0x250ec5f4);

	Engine::World::traffic = (int*)(base + 0x44f855e0);
	Engine::World::startCash = (int*)(base + 0x44f85608);
	Engine::World::minCash = (int*)(base + 0x44f8560c);
	Engine::World::showJoinExit = (bool*)(base + 0x44f85610);
	Engine::World::respawnTeam = (bool*)(base + 0x44f85614);
	Engine::World::Crime::civCiv = (int*)(base + 0x44f855e4);
	Engine::World::Crime::civTeam = (int*)(base + 0x44f855e8);
	Engine::World::Crime::teamCiv = (int*)(base + 0x44f855ec);
	Engine::World::Crime::teamTeam = (int*)(base + 0x44f855f0);
	Engine::World::Crime::teamTeamInBase = (int*)(base + 0x44f855f4);
	Engine::World::Crime::noSpawn = (int*)(base + 0x44f85600);

	Engine::Round::roundTime = (int*)(base + 0x44f855cc);
	Engine::Round::startCash = (int*)(base + 0x44f855d0);
	Engine::Round::weekly = (bool*)(base + 0x44f855d4);
	Engine::Round::weekDay = (int*)(base + 0x44ecace4);
	Engine::Round::bonusRatio = (bool*)(base + 0x44f855d8);
	Engine::Round::teamDamage = (int*)(base + 0x44f855dc);

	Engine::gameType = (int*)(base + 0x44ecaacc);
	Engine::mapName = (char*)(base + 0x44ecaad0);
	Engine::loadedMapName = (char*)(base + 0x39496124);
	Engine::gameState = (int*)(base + 0x44ecacec);
	Engine::gameTimer = (int*)(base + 0x44ecacf4);
	Engine::ticksSinceReset = (int*)(base + 0x44ecad54);
	Engine::sunTime = (unsigned int*)(base + 0xdfb21e0);
	Engine::isLevelLoaded = (int*)(base + 0x39496120);
	Engine::gravity = (float*)(base + 0xd874c);
	pryMemory(Engine::gravity, 1);
	Engine::originalGravity = *Engine::gravity;

	Engine::lineIntersectResult = (LineIntersectResult*)(base + 0x569ef720);

	Engine::connections = (Connection*)(base + 0x669b20);
	Engine::accounts = (Account*)(base + 0x357fd10);
	Engine::voices = (Voice*)(base + 0xaebed80);
	Engine::players = (Player*)(base + 0x15b35420);
	Engine::humans = (Human*)(base + 0xcefe1c8);
	Engine::itemTypes = (ItemType*)(base + 0x5a60d7c0);
	Engine::items = (Item*)(base + 0x8117920);
	Engine::vehicleTypes = (VehicleType*)(base + 0x4d03560);
	Engine::vehicles = (Vehicle*)(base + 0x1cf42740);
	Engine::bullets = (Bullet*)(base + 0x44835bc0);
	Engine::bodies = (RigidBody*)(base + 0x4f1ae0);
	Engine::bonds = (Bond*)(base + 0x24adc1c0);
	Engine::streets = (Street*)(base + 0x394b8188);
	Engine::streetIntersections = (StreetIntersection*)(base + 0x39496184);
	Engine::trafficCars = (TrafficCar*)(base + 0x5a80bea0);
	Engine::buildings = (Building*)(base + 0x3958a358);
	Engine::events = (Event*)(base + 0x5edcd80);

	Engine::numConnections = (unsigned int*)(base + 0x45ed7da8);
	Engine::numBullets = (unsigned int*)(base + 0x45e07380);
	Engine::numStreets = (unsigned int*)(base + 0x394b8184);
	Engine::numStreetIntersections = (unsigned int*)(base + 0x3949617c);
	Engine::numTrafficCars = (unsigned int*)(base + 0x15b253f8);
	Engine::numBuildings = (unsigned int*)(base + 0x3958a314);
	Engine::numEvents = (unsigned short*)(base + 0x45e07384);

	Engine::subRosaPuts = (Engine::subRosaPutsFunc)(base + 0x1fd0);
	Engine::subRosa__printf_chk =
	    (Engine::subRosa__printf_chkFunc)(base + 0x2300);

	Engine::resetGame = (Engine::voidFunc)(base + 0xbf380);
	Engine::createTraffic = (Engine::createTrafficFunc)(base + 0x9f690);
	Engine::trafficSimulation = (Engine::voidFunc)(base + 0xa28d0);
	Engine::aiTrafficCar = (Engine::aiTrafficCarFunc)(base + 0xfe00);
	Engine::aiTrafficCarDestination =
	    (Engine::aiTrafficCarDestinationFunc)(base + 0xf7a0);

	Engine::areaCreateBlock = (Engine::areaCreateBlockFunc)(base + 0x19920);
	Engine::areaDeleteBlock = (Engine::areaDeleteBlockFunc)(base + 0x13800);

	Engine::logicSimulation = (Engine::voidFunc)(base + 0xc5f30);
	Engine::logicSimulationRace = (Engine::voidFunc)(base + 0xc19d0);
	Engine::logicSimulationRound = (Engine::voidFunc)(base + 0xc2150);
	Engine::logicSimulationWorld = (Engine::voidFunc)(base + 0xc54e0);
	Engine::logicSimulationTerminator = (Engine::voidFunc)(base + 0xc30d0);
	Engine::logicSimulationCoop = (Engine::voidFunc)(base + 0xc1790);
	Engine::logicSimulationVersus = (Engine::voidFunc)(base + 0xc4930);
	Engine::logicPlayerActions = (Engine::voidIndexFunc)(base + 0xba950);

	Engine::physicsSimulation = (Engine::voidFunc)(base + 0xa95e0);
	Engine::rigidBodySimulation = (Engine::voidFunc)(base + 0x7f320);
	Engine::serverReceive = (Engine::serverReceiveFunc)(base + 0xd0200);
	Engine::serverSend = (Engine::voidFunc)(base + 0xcd200);
	Engine::packetWrite = (Engine::packetWriteFunc)(base + 0xc8230);
	Engine::calculatePlayerVoice =
	    (Engine::calculatePlayerVoiceFunc)(base + 0xb4c80);
	Engine::sendPacket = (Engine::sendPacketFunc)(base + 0xc7ff0);
	Engine::bulletSimulation = (Engine::voidFunc)(base + 0x8b4c0);
	Engine::bulletTimeToLive = (Engine::voidFunc)(base + 0x1ed50);

	Engine::economyCarMarket = (Engine::voidFunc)(base + 0x26e70);
	Engine::saveAccountsServer = (Engine::voidFunc)(base + 0xe260);

	Engine::createAccountByJoinTicket =
	    (Engine::createAccountByJoinTicketFunc)(base + 0xdb70);
	Engine::serverSendConnectResponse =
	    (Engine::serverSendConnectResponseFunc)(base + 0xc8610);

	Engine::scenarioArmHuman = (Engine::scenarioArmHumanFunc)(base + 0x7ae30);
	Engine::linkItem = (Engine::linkItemFunc)(base + 0x460e0);
	Engine::itemSetMemo = (Engine::itemSetMemoFunc)(base + 0x3dbf0);
	Engine::itemComputerTransmitLine =
	    (Engine::itemComputerTransmitLineFunc)(base + 0x3dd70);
	Engine::itemComputerIncrementLine = (Engine::voidIndexFunc)(base + 0x3e0e0);
	Engine::itemComputerInput = (Engine::itemComputerInputFunc)(base + 0x78000);
	Engine::itemCashAddBill = (Engine::itemCashAddBillFunc)(base + 0x3c8a0);
	Engine::itemCashRemoveBill = (Engine::itemCashRemoveBillFunc)(base + 0x3c990);
	Engine::itemCashGetBillValue =
	    (Engine::itemCashGetBillValueFunc)(base + 0x3c840);

	Engine::humanApplyDamage = (Engine::humanApplyDamageFunc)(base + 0x2b120);
	Engine::humanCollisionVehicle =
	    (Engine::humanCollisionVehicleFunc)(base + 0x68450);
	Engine::humanLimbInverseKinematics =
	    (Engine::humanLimbInverseKinematicsFunc)(base + 0x718d0);
	Engine::grenadeExplosion = (Engine::voidIndexFunc)(base + 0x43410);
	Engine::serverPlayerMessage =
	    (Engine::serverPlayerMessageFunc)(base + 0xb9050);
	Engine::playerAI = (Engine::voidIndexFunc)(base + 0x89a60);
	Engine::playerDeathTax = (Engine::voidIndexFunc)(base + 0x5d30);
	Engine::accountDeathTax = (Engine::voidIndexFunc)(base + 0x5710);
	Engine::playerGiveWantedLevel =
	    (Engine::playerGiveWantedLevelFunc)(base + 0x6bc0);
	Engine::createBondRigidBodyToRigidBody =
	    (Engine::createBondRigidBodyToRigidBodyFunc)(base + 0x1ae60);
	Engine::createBondRigidBodyRotRigidBody =
	    (Engine::createBondRigidBodyRotRigidBodyFunc)(base + 0x1b0e0);
	Engine::createBondRigidBodyToLevel =
	    (Engine::createBondRigidBodyToLevelFunc)(base + 0x1ad40);
	Engine::addCollisionRigidBodyOnRigidBody =
	    (Engine::addCollisionRigidBodyOnRigidBodyFunc)(base + 0x1b1c0);
	Engine::addCollisionRigidBodyOnLevel =
	    (Engine::addCollisionRigidBodyOnLevelFunc)(base + 0x1b350);

	Engine::createBullet = (Engine::createBulletFunc)(base + 0x1e880);
	Engine::createPlayer = (Engine::createPlayerFunc)(base + 0x6d040);
	Engine::deletePlayer = (Engine::voidIndexFunc)(base + 0x6d330);
	Engine::createHuman = (Engine::createHumanFunc)(base + 0x76b80);
	Engine::deleteHuman = (Engine::voidIndexFunc)(base + 0x6ef0);
	Engine::createItem = (Engine::createItemFunc)(base + 0x777a0);
	Engine::deleteItem = (Engine::voidIndexFunc)(base + 0x48ce0);
	Engine::createRope = (Engine::createRopeFunc)(base + 0x78b70);
	Engine::createVehicle = (Engine::createVehicleFunc)(base + 0x7cf20);
	Engine::deleteVehicle = (Engine::voidIndexFunc)(base + 0x7150);
	Engine::createRigidBody = (Engine::createRigidBodyFunc)(base + 0x76940);

	Engine::createEventMessage = (Engine::createEventMessageFunc)(base + 0x58a0);
	Engine::createEventUpdatePlayer = (Engine::voidIndexFunc)(base + 0x5ba0);
	Engine::createEventUpdatePlayerFinance =
	    (Engine::voidIndexFunc)(base + 0x5cc0);
	Engine::createEventCreateVehicle = (Engine::voidIndexFunc)(base + 0x59c0);
	Engine::createEventUpdateVehicle =
	    (Engine::createEventUpdateVehicleFunc)(base + 0x5a20);
	Engine::createEventSound = (Engine::createEventSoundFunc)(base + 0x5e00);
	Engine::createEventExplosion =
	    (Engine::createEventExplosionFunc)(base + 0x64d0);
	Engine::createEventBullet = (Engine::createEventBulletFunc)(base + 0x5760);
	Engine::createEventBulletHit =
	    (Engine::createEventBulletHitFunc)(base + 0x57f0);

	Engine::lineIntersectHuman = (Engine::lineIntersectHumanFunc)(base + 0x3a200);
	Engine::lineIntersectLevel = (Engine::lineIntersectLevelFunc)(base + 0x88cf0);
	Engine::lineIntersectVehicle =
	    (Engine::lineIntersectVehicleFunc)(base + 0x6b2a0);
	Engine::lineIntersectTriangle =
	    (Engine::lineIntersectTriangleFunc)(base + 0x8ef0);
}

static inline void installHook(
    const char* name, subhook::Hook& hook, void* source, void* destination,
    subhook::HookFlags flags = subhook::HookFlags::HookFlag64BitOffset) {
	if (!hook.Install(source, destination, flags)) {
		std::ostringstream stream;
		stream << RS_PREFIX "Hook " << name << " failed to install";

		throw std::runtime_error(stream.str());
	}
}

#define INSTALL(name)                                               \
	installHook(#name "Hook", Hooks::name##Hook, (void*)Engine::name, \
	            (void*)Hooks::name);

static inline void installHooks() {
	INSTALL(subRosaPuts);
	INSTALL(subRosa__printf_chk);
	INSTALL(resetGame);
	INSTALL(createTraffic);
	INSTALL(trafficSimulation);
	INSTALL(aiTrafficCar);
	INSTALL(aiTrafficCarDestination);
	INSTALL(areaCreateBlock);
	INSTALL(areaDeleteBlock);
	INSTALL(logicSimulation);
	INSTALL(logicSimulationRace);
	INSTALL(logicSimulationRound);
	INSTALL(logicSimulationWorld);
	INSTALL(logicSimulationTerminator);
	INSTALL(logicSimulationCoop);
	INSTALL(logicSimulationVersus);
	INSTALL(logicPlayerActions);
	INSTALL(physicsSimulation);
	INSTALL(rigidBodySimulation);
	INSTALL(serverReceive);
	INSTALL(serverSend);
	INSTALL(packetWrite);
	INSTALL(calculatePlayerVoice);
	INSTALL(sendPacket);
	INSTALL(bulletSimulation);
	INSTALL(economyCarMarket);
	INSTALL(saveAccountsServer);
	INSTALL(createAccountByJoinTicket);
	INSTALL(serverSendConnectResponse);
	INSTALL(linkItem);
	INSTALL(itemComputerInput);
	INSTALL(humanApplyDamage);
	INSTALL(humanCollisionVehicle);
	INSTALL(humanLimbInverseKinematics);
	INSTALL(grenadeExplosion);
	INSTALL(serverPlayerMessage);
	INSTALL(playerAI);
	INSTALL(playerDeathTax);
	INSTALL(accountDeathTax);
	INSTALL(playerGiveWantedLevel);
	INSTALL(addCollisionRigidBodyOnRigidBody);
	INSTALL(createBullet);
	INSTALL(createPlayer);
	INSTALL(deletePlayer);
	INSTALL(createHuman);
	INSTALL(deleteHuman);
	INSTALL(createItem);
	INSTALL(deleteItem);
	INSTALL(createVehicle);
	INSTALL(deleteVehicle);
	INSTALL(createRigidBody);
	INSTALL(createEventMessage);
	INSTALL(createEventUpdatePlayer);
	INSTALL(createEventUpdateVehicle);
	INSTALL(createEventSound);
	INSTALL(createEventBullet);
	INSTALL(createEventBulletHit);
	INSTALL(lineIntersectHuman);
	INSTALL(lineIntersectLevel);
}

static inline void attachInterruptSignalHandler() {
	struct sigaction action;
	action.sa_handler = Console::handleInterruptSignal;

	if (sigaction(SIGINT, &action, nullptr) == -1) {
		throw std::runtime_error(strerror(errno));
	}
}

static subhook::Hook getPathsHook;
typedef void (*getPathsFunc)();
static getPathsFunc getPaths;

// 'getpaths' is a tiny function that's called inside of main. It has to be
// recreated since installing a hook before main can't be reversed for some
// reason.
static inline void getPathsNormally() {
	char* pathA = (char*)(Lua::memory::baseAddress + 0x5a35cbc0);
	char* pathB = (char*)(Lua::memory::baseAddress + 0x5a35cdc0);

	getcwd(pathA, 0x200);
	getcwd(pathB, 0x200);
}

static void crashSignalHandler(int signal) {
	Console::shouldExit = true;

	std::cerr << std::flush << "\033[41;1m " << strsignal(signal)
	          << " \033[0m\n\033[31m";

	std::cerr << "Stack traceback:\n";

	void* backtraceEntries[10];

	size_t backtraceSize = backtrace(backtraceEntries, 10);
	auto backtraceSymbols = backtrace_symbols(backtraceEntries, backtraceSize);

	for (int i = 0; i < backtraceSize; i++) {
		std::cerr << "\t#" << i << ' ' << backtraceSymbols[i] << '\n';
	}

	std::cerr << std::flush;

	luaL_traceback(*lua, *lua, nullptr, 0);
	std::cerr << "Lua " << lua_tostring(*lua, -1);

	std::cerr << "\033[0m" << std::endl;

	raise(signal);
	_exit(EXIT_FAILURE);
}

static const std::array handledSignals = {
    SIGABRT, SIGBUS, SIGFPE,  SIGILL,  SIGQUIT,
    SIGSEGV, SIGSYS, SIGTRAP, SIGXCPU, SIGXFSZ,
};

static inline void attachCrashSignalHandler() {
	struct sigaction signalAction;

	signalAction.sa_handler = crashSignalHandler;
	signalAction.sa_flags = SA_RESTART | SA_SIGINFO;

	for (const auto signal : handledSignals) {
		if (sigaction(signal, &signalAction, nullptr) == -1) {
			std::ostringstream stream;
			stream << RS_PREFIX "Signal handler failed to attach for signal "
			       << signal << " (" << strsignal(signal) << "): " << strerror(errno);

			throw std::runtime_error(stream.str());
		}
	}
}

static void hookedGetPaths() {
	getPathsNormally();

	attachCrashSignalHandler();
	attachInterruptSignalHandler();
	signal(SIGPIPE, SIG_IGN);

	Console::log(RS_PREFIX "Assuming 38e\n");

	Console::log(RS_PREFIX "Locating memory...\n");
	printBaseAddress(Lua::memory::baseAddress);
	locateMemory(Lua::memory::baseAddress);

	Console::log(RS_PREFIX "Installing hooks...\n");
	installHooks();

	Console::log(RS_PREFIX "Waiting for engine init...\n");

	// Don't load self into future child processes
	unsetenv("LD_PRELOAD");
}

void __attribute__((constructor)) entry() {
	Lua::memory::baseAddress = getBaseAddress();
	getPaths = (getPathsFunc)(Lua::memory::baseAddress + 0xd5200);

	installHook("getPathsHook", getPathsHook, (void*)getPaths,
	            (void*)hookedGetPaths);
}
