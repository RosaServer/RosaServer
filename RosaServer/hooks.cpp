#include "hooks.h"
#include "api.h"
#include "console.h"

namespace Hooks {
const std::unordered_map<std::string, EnableKeys> enableNames(
    {{"InterruptSignal", EnableKeys::InterruptSignal},
     {"ResetGame", EnableKeys::ResetGame},
     {"AreaCreateBlock", EnableKeys::AreaCreateBlock},
     {"AreaDeleteBlock", EnableKeys::AreaDeleteBlock},
     {"Logic", EnableKeys::Logic},
     {"ConsoleInput", EnableKeys::ConsoleInput},
     {"ConsoleAutoComplete", EnableKeys::ConsoleAutoComplete},
     {"LogicRace", EnableKeys::LogicRace},
     {"LogicRound", EnableKeys::LogicRound},
     {"LogicWorld", EnableKeys::LogicWorld},
     {"LogicTerminator", EnableKeys::LogicTerminator},
     {"LogicCoop", EnableKeys::LogicCoop},
     {"LogicVersus", EnableKeys::LogicVersus},
     {"PlayerActions", EnableKeys::PlayerActions},
     {"Physics", EnableKeys::Physics},
     {"PhysicsRigidBodies", EnableKeys::PhysicsRigidBodies},
     {"InPacket", EnableKeys::InPacket},
     {"SendPacket", EnableKeys::SendPacket},
     {"PhysicsBullets", EnableKeys::PhysicsBullets},
     {"EconomyCarMarket", EnableKeys::EconomyCarMarket},
     {"AccountsSave", EnableKeys::AccountsSave},
     {"AccountTicketBegin", EnableKeys::AccountTicketBegin},
     {"AccountTicketFound", EnableKeys::AccountTicketFound},
     {"AccountTicket", EnableKeys::AccountTicket},
     {"SendConnectResponse", EnableKeys::SendConnectResponse},
     {"ItemLink", EnableKeys::ItemLink},
     {"ItemComputerInput", EnableKeys::ItemComputerInput},
     {"HumanDamage", EnableKeys::HumanDamage},
     {"HumanCollisionVehicle", EnableKeys::HumanCollisionVehicle},
     {"HumanLimbInverseKinematics", EnableKeys::HumanLimbInverseKinematics},
     {"GrenadeExplode", EnableKeys::GrenadeExplode},
     {"PlayerChat", EnableKeys::PlayerChat},
     {"PlayerAI", EnableKeys::PlayerAI},
     {"PlayerDeathTax", EnableKeys::PlayerDeathTax},
     {"AccountDeathTax", EnableKeys::AccountDeathTax},
     {"PlayerGiveWantedLevel", EnableKeys::PlayerGiveWantedLevel},
     {"CollideBodies", EnableKeys::CollideBodies},
     {"BulletCreate", EnableKeys::BulletCreate},
     {"PlayerCreate", EnableKeys::PlayerCreate},
     {"PlayerDelete", EnableKeys::PlayerDelete},
     {"HumanCreate", EnableKeys::HumanCreate},
     {"HumanDelete", EnableKeys::HumanDelete},
     {"ItemCreate", EnableKeys::ItemCreate},
     {"ItemDelete", EnableKeys::ItemDelete},
     {"VehicleCreate", EnableKeys::VehicleCreate},
     {"VehicleDelete", EnableKeys::VehicleDelete},
     {"EventMessage", EnableKeys::EventMessage},
     {"EventUpdatePlayer", EnableKeys::EventUpdatePlayer},
     {"EventUpdateVehicle", EnableKeys::EventUpdateVehicle},
     {"EventBullet", EnableKeys::EventBullet},
     {"EventBulletHit", EnableKeys::EventBulletHit},
     {"LineIntersectHuman", EnableKeys::LineIntersectHuman}});
bool enabledKeys[EnableKeys::SIZE] = {0};

subhook::Hook subRosaPutsHook;
subhook::Hook subRosa__printf_chkHook;
subhook::Hook resetGameHook;
subhook::Hook areaCreateBlockHook;
subhook::Hook areaDeleteBlockHook;
subhook::Hook logicSimulationHook;
subhook::Hook logicSimulationRaceHook;
subhook::Hook logicSimulationRoundHook;
subhook::Hook logicSimulationWorldHook;
subhook::Hook logicSimulationTerminatorHook;
subhook::Hook logicSimulationCoopHook;
subhook::Hook logicSimulationVersusHook;
subhook::Hook logicPlayerActionsHook;
subhook::Hook physicsSimulationHook;
subhook::Hook rigidBodySimulationHook;
subhook::Hook serverReceiveHook;
subhook::Hook serverSendHook;
subhook::Hook bulletSimulationHook;
subhook::Hook economyCarMarketHook;
subhook::Hook saveAccountsServerHook;
subhook::Hook createAccountByJoinTicketHook;
subhook::Hook serverSendConnectResponseHook;
subhook::Hook linkItemHook;
subhook::Hook itemComputerInputHook;
subhook::Hook humanApplyDamageHook;
subhook::Hook humanCollisionVehicleHook;
subhook::Hook humanLimbInverseKinematicsHook;
subhook::Hook grenadeExplosionHook;
subhook::Hook serverPlayerMessageHook;
subhook::Hook playerAIHook;
subhook::Hook playerDeathTaxHook;
subhook::Hook accountDeathTaxHook;
subhook::Hook playerGiveWantedLevelHook;
subhook::Hook addCollisionRigidBodyOnRigidBodyHook;
subhook::Hook createBulletHook;
subhook::Hook createPlayerHook;
subhook::Hook deletePlayerHook;
subhook::Hook createHumanHook;
subhook::Hook deleteHumanHook;
subhook::Hook createItemHook;
subhook::Hook deleteItemHook;
subhook::Hook createVehicleHook;
subhook::Hook deleteVehicleHook;
subhook::Hook createRigidBodyHook;
subhook::Hook createEventMessageHook;
subhook::Hook createEventUpdatePlayerHook;
subhook::Hook createEventUpdateVehicleHook;
subhook::Hook createEventBulletHook;
subhook::Hook createEventBulletHitHook;
subhook::Hook lineIntersectHumanHook;

int subRosaPuts(const char* str) {
	std::ostringstream stream;

	stream << SUBROSA_PREFIX;
	stream << str;
	stream << "\n";

	Console::log(stream.str());

	return 1;
}

int subRosa__printf_chk(int flag, const char* format, ...) {
	va_list arguments;
	va_start(arguments, format);

	char buffer[256];
	vsnprintf(buffer, 256, format, arguments);

	std::ostringstream stream;

	stream << SUBROSA_PREFIX;
	stream << buffer;

	Console::log(stream.str());

	va_end(arguments);
	return 0;
}

void resetGame() {
	if (!initialized) {
		initialized = true;

		Console::log(RS_PREFIX "Engine ready...\n");

		luaInit();

		Console::log(RS_PREFIX "Initializing input...\n");
		Console::init();

		Console::log(RS_PREFIX "Ready!\n");
		hookAndReset(RESET_REASON_BOOT);
	} else {
		hookAndReset(RESET_REASON_ENGINECALL);
	}
}

void areaCreateBlock(int zero, int blockX, int blockY, int blockZ,
                     unsigned int flags, short unk[8]) {
	if (enabledKeys[EnableKeys::AreaCreateBlock]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			UnsignedInteger wrappedFlags = {flags};

			auto res = func("AreaCreateBlock", blockX, blockY, blockZ, &wrappedFlags);
			if (noLuaCallError(&res)) noParent = (bool)res;

			flags = wrappedFlags.value;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&areaCreateBlockHook);
				Engine::areaCreateBlock(zero, blockX, blockY, blockZ, flags, unk);
			}
			if (func != sol::nil) {
				auto res = func("PostAreaCreateBlock", blockX, blockY, blockZ, flags);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&areaCreateBlockHook);
		Engine::areaCreateBlock(zero, blockX, blockY, blockZ, flags, unk);
	}
}

void areaDeleteBlock(int zero, int blockX, int blockY, int blockZ) {
	if (enabledKeys[EnableKeys::AreaDeleteBlock]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("AreaDeleteBlock", blockX, blockY, blockZ);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&areaDeleteBlockHook);
				Engine::areaDeleteBlock(zero, blockX, blockY, blockZ);
			}
			if (func != sol::nil) {
				auto res = func("PostAreaDeleteBlock", blockX, blockY, blockZ);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&areaDeleteBlockHook);
		Engine::areaDeleteBlock(zero, blockX, blockY, blockZ);
	}
}

void logicSimulation() {
	if (shouldReset) {
		shouldReset = false;
		luaInit(true);

		hookAndReset(RESET_REASON_LUARESET);
	}

	bool noParent = false;
	sol::protected_function hookFunc = (*lua)["hook"]["run"];

	if (Console::shouldExit) {
		if (enabledKeys[EnableKeys::InterruptSignal] && hookFunc != sol::nil) {
			auto res = hookFunc("InterruptSignal");
			noLuaCallError(&res);
		}
		Lua::os::exit();
		return;
	}

	if (enabledKeys[EnableKeys::Logic]) {
		if (hookFunc != sol::nil) {
			auto res = hookFunc("Logic");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&logicSimulationHook);
				Engine::logicSimulation();
			}
			if (hookFunc != sol::nil) {
				auto res = hookFunc("PostLogic");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&logicSimulationHook);
		Engine::logicSimulation();
	}

	{
		std::lock_guard<std::mutex> guard(Console::commandQueueMutex);
		while (!Console::commandQueue.empty()) {
			if (enabledKeys[EnableKeys::ConsoleInput] && hookFunc != sol::nil) {
				auto res = hookFunc("ConsoleInput", Console::commandQueue.front());
				noLuaCallError(&res);
			}
			Console::commandQueue.pop();
		}
	}

	if (Console::isAwaitingAutoComplete()) {
		if (enabledKeys[EnableKeys::ConsoleAutoComplete] && hookFunc != sol::nil) {
			auto data = lua->create_table();
			data["response"] = Console::getAutoCompleteInput();

			auto res = hookFunc("ConsoleAutoComplete", data);
			noLuaCallError(&res);

			std::string response = data["response"];
			Console::respondToAutoComplete(response);
		} else {
			Console::respondToAutoComplete(Console::getAutoCompleteInput());
		}
	}
}

void logicSimulationRace() {
	if (enabledKeys[EnableKeys::LogicRace]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("LogicRace");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&logicSimulationRaceHook);
				Engine::logicSimulationRace();
			}
			if (func != sol::nil) {
				auto res = func("PostLogicRace");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&logicSimulationRaceHook);
		Engine::logicSimulationRace();
	}
}

void logicSimulationRound() {
	if (enabledKeys[EnableKeys::LogicRound]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("LogicRound");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&logicSimulationRoundHook);
				Engine::logicSimulationRound();
			}
			if (func != sol::nil) {
				auto res = func("PostLogicRound");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&logicSimulationRoundHook);
		Engine::logicSimulationRound();
	}
}

void logicSimulationWorld() {
	if (enabledKeys[EnableKeys::LogicWorld]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("LogicWorld");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&logicSimulationWorldHook);
				Engine::logicSimulationWorld();
			}
			if (func != sol::nil) {
				auto res = func("PostLogicWorld");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&logicSimulationWorldHook);
		Engine::logicSimulationWorld();
	}
}

void logicSimulationTerminator() {
	if (enabledKeys[EnableKeys::LogicTerminator]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("LogicTerminator");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&logicSimulationTerminatorHook);
				Engine::logicSimulationTerminator();
			}
			if (func != sol::nil) {
				auto res = func("PostLogicTerminator");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&logicSimulationTerminatorHook);
		Engine::logicSimulationTerminator();
	}
}

void logicSimulationCoop() {
	if (enabledKeys[EnableKeys::LogicCoop]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("LogicCoop");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&logicSimulationCoopHook);
				Engine::logicSimulationCoop();
			}
			if (func != sol::nil) {
				auto res = func("PostLogicCoop");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&logicSimulationCoopHook);
		Engine::logicSimulationCoop();
	}
}

void logicSimulationVersus() {
	if (enabledKeys[EnableKeys::LogicVersus]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("LogicVersus");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&logicSimulationVersusHook);
				Engine::logicSimulationVersus();
			}
			if (func != sol::nil) {
				auto res = func("PostLogicVersus");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&logicSimulationVersusHook);
		Engine::logicSimulationVersus();
	}
}

void logicPlayerActions(int playerID) {
	if (enabledKeys[EnableKeys::PlayerActions]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("PlayerActions", &Engine::players[playerID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&logicPlayerActionsHook);
				Engine::logicPlayerActions(playerID);
			}
			if (func != sol::nil) {
				auto res = func("PostPlayerActions", &Engine::players[playerID]);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&logicPlayerActionsHook);
		Engine::logicPlayerActions(playerID);
	}
}

void physicsSimulation() {
	if (enabledKeys[EnableKeys::Physics]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("Physics");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&physicsSimulationHook);
				Engine::physicsSimulation();
			}
			if (func != sol::nil) {
				auto res = func("PostPhysics");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&physicsSimulationHook);
		Engine::physicsSimulation();
	}
}

void rigidBodySimulation() {
	if (enabledKeys[EnableKeys::PhysicsRigidBodies]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("PhysicsRigidBodies");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&rigidBodySimulationHook);
				Engine::rigidBodySimulation();
			}
			if (func != sol::nil) {
				auto res = func("PostPhysicsRigidBodies");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&rigidBodySimulationHook);
		Engine::rigidBodySimulation();
	}
}

int serverReceive() {
	if (enabledKeys[EnableKeys::InPacket]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("InPacket");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			int ret;
			{
				subhook::ScopedHookRemove remove(&serverReceiveHook);
				ret = Engine::serverReceive();
			}
			if (func != sol::nil) {
				auto res = func("PostInPacket");
				noLuaCallError(&res);
			}
			return ret;
		}
		return -1;
	} else {
		subhook::ScopedHookRemove remove(&serverReceiveHook);
		return Engine::serverReceive();
	}
}

void serverSend() {
	if (enabledKeys[EnableKeys::SendPacket]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("SendPacket");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&serverSendHook);
				Engine::serverSend();
			}
			if (func != sol::nil) {
				auto res = func("PostSendPacket");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&serverSendHook);
		Engine::serverSend();
	}
}

void bulletSimulation() {
	if (enabledKeys[EnableKeys::PhysicsBullets]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("PhysicsBullets");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&bulletSimulationHook);
				Engine::bulletSimulation();
			}
			if (func != sol::nil) {
				auto res = func("PostPhysicsBullets");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&bulletSimulationHook);
		Engine::bulletSimulation();
	}
}

void economyCarMarket() {
	if (enabledKeys[EnableKeys::EconomyCarMarket]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("EconomyCarMarket");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&economyCarMarketHook);
				Engine::economyCarMarket();
			}
			if (func != sol::nil) {
				auto res = func("PostEconomyCarMarket");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&economyCarMarketHook);
		Engine::economyCarMarket();
	}
}

void saveAccountsServer() {
	if (enabledKeys[EnableKeys::AccountsSave]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("AccountsSave");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&saveAccountsServerHook);
				Engine::saveAccountsServer();
			}
			if (func != sol::nil) {
				auto res = func("PostAccountsSave");
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&saveAccountsServerHook);
		Engine::saveAccountsServer();
	}
}

int createAccountByJoinTicket(int identifier, unsigned int ticket) {
	if (enabledKeys[EnableKeys::AccountTicketBegin] ||
	    enabledKeys[EnableKeys::AccountTicketFound] ||
	    enabledKeys[EnableKeys::AccountTicket]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("AccountTicketBegin", identifier, ticket);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			int id;
			{
				subhook::ScopedHookRemove remove(&createAccountByJoinTicketHook);
				id = Engine::createAccountByJoinTicket(identifier, ticket);
			}
			if (func != sol::nil) {
				auto res = func("AccountTicketFound",
				                id == -1 ? nullptr : &Engine::accounts[id]);
				noParent = false;
				if (noLuaCallError(&res)) noParent = (bool)res;

				if (!noParent) {
					auto res = func("PostAccountTicket",
					                id == -1 ? nullptr : &Engine::accounts[id]);
					noLuaCallError(&res);
					return id;
				}
				return -1;
			}
			return id;
		}
		return -1;
	} else {
		subhook::ScopedHookRemove remove(&createAccountByJoinTicketHook);
		return Engine::createAccountByJoinTicket(identifier, ticket);
	}
}

void serverSendConnectResponse(unsigned int address, unsigned int port, int unk,
                               const char* message) {
	if (enabledKeys[EnableKeys::SendConnectResponse]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];

		auto addressString = addressFromInteger(address);

		auto data = lua->create_table();
		data["message"] = message;
		std::string newMessage;

		if (func != sol::nil) {
			auto res = func("SendConnectResponse", addressString, port, data);
			if (noLuaCallError(&res)) {
				noParent = (bool)res;
				newMessage = data["message"];
				message = newMessage.c_str();
			}
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&serverSendConnectResponseHook);
				Engine::serverSendConnectResponse(address, port, unk, message);
			}
			if (func != sol::nil) {
				auto res = func("PostSendConnectResponse", addressString, port, data);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&serverSendConnectResponseHook);
		Engine::serverSendConnectResponse(address, port, unk, message);
	}
}

int createBullet(int type, Vector* pos, Vector* vel, int playerID) {
	if (enabledKeys[EnableKeys::BulletCreate]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res =
			    func("BulletCreate", type, pos, vel, &Engine::players[playerID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			int id;
			{
				subhook::ScopedHookRemove remove(&createBulletHook);
				id = Engine::createBullet(type, pos, vel, playerID);
			}
			if (func != sol::nil && id != -1) {
				auto res = func("PostBulletCreate", &Engine::bullets[id]);
				noLuaCallError(&res);
			}
			return id;
		}
		return -1;
	} else {
		subhook::ScopedHookRemove remove(&createBulletHook);
		return Engine::createBullet(type, pos, vel, playerID);
	}
}

int createPlayer() {
	if (enabledKeys[EnableKeys::PlayerCreate]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("PlayerCreate");
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			int id;
			{
				subhook::ScopedHookRemove remove(&createPlayerHook);
				id = Engine::createPlayer();

				if (id != -1 && playerDataTables[id]) {
					delete playerDataTables[id];
					playerDataTables[id] = nullptr;
				}
			}
			if (func != sol::nil && id != -1) {
				auto res = func("PostPlayerCreate", &Engine::players[id]);
				noLuaCallError(&res);
			}
			return id;
		}
		return -1;
	} else {
		subhook::ScopedHookRemove remove(&createPlayerHook);
		int id = Engine::createPlayer();

		if (id != -1 && playerDataTables[id]) {
			delete playerDataTables[id];
			playerDataTables[id] = nullptr;
		}

		return id;
	}
}

void deletePlayer(int playerID) {
	if (enabledKeys[EnableKeys::PlayerDelete]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("PlayerDelete", &Engine::players[playerID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&deletePlayerHook);
				Engine::deletePlayer(playerID);
			}
			if (func != sol::nil) {
				auto res = func("PostPlayerDelete", &Engine::players[playerID]);
				noLuaCallError(&res);
			}
			if (playerDataTables[playerID]) {
				delete playerDataTables[playerID];
				playerDataTables[playerID] = nullptr;
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&deletePlayerHook);
		Engine::deletePlayer(playerID);

		if (playerDataTables[playerID]) {
			delete playerDataTables[playerID];
			playerDataTables[playerID] = nullptr;
		}
	}
}

int createHuman(Vector* pos, RotMatrix* rot, int playerID) {
	if (enabledKeys[EnableKeys::HumanCreate]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("HumanCreate", pos, rot, &Engine::players[playerID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			int id;
			{
				subhook::ScopedHookRemove remove(&createHumanHook);
				id = Engine::createHuman(pos, rot, playerID);

				if (id != -1 && humanDataTables[id]) {
					delete humanDataTables[id];
					humanDataTables[id] = nullptr;
				}
			}
			if (func != sol::nil && id != -1) {
				auto res = func("PostHumanCreate", &Engine::humans[id]);
				noLuaCallError(&res);
			}
			return id;
		}
		return -1;
	} else {
		subhook::ScopedHookRemove remove(&createHumanHook);
		int id = Engine::createHuman(pos, rot, playerID);

		if (id != -1 && humanDataTables[id]) {
			delete humanDataTables[id];
			humanDataTables[id] = nullptr;
		}

		return id;
	}
}

void deleteHuman(int humanID) {
	if (enabledKeys[EnableKeys::HumanDelete]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("HumanDelete", &Engine::humans[humanID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&deleteHumanHook);
				Engine::deleteHuman(humanID);
			}
			if (func != sol::nil) {
				auto res = func("PostHumanDelete", &Engine::humans[humanID]);
				noLuaCallError(&res);
			}
			if (humanDataTables[humanID]) {
				delete humanDataTables[humanID];
				humanDataTables[humanID] = nullptr;
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&deleteHumanHook);
		Engine::deleteHuman(humanID);

		if (humanDataTables[humanID]) {
			delete humanDataTables[humanID];
			humanDataTables[humanID] = nullptr;
		}
	}
}

int createItem(int type, Vector* pos, Vector* vel, RotMatrix* rot) {
	if (enabledKeys[EnableKeys::ItemCreate]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("ItemCreate", &Engine::itemTypes[type], pos, rot);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			int id;
			{
				subhook::ScopedHookRemove remove(&createItemHook);
				id = Engine::createItem(type, pos, vel, rot);
			}
			if (id != -1 && func != sol::nil) {
				auto res = func("PostItemCreate", &Engine::items[id]);
				noLuaCallError(&res);
			}
			if (id != -1 && itemDataTables[id]) {
				delete itemDataTables[id];
				itemDataTables[id] = nullptr;
			}
			return id;
		}
		return -1;
	} else {
		subhook::ScopedHookRemove remove(&createItemHook);
		int id = Engine::createItem(type, pos, vel, rot);

		if (id != -1 && itemDataTables[id]) {
			delete itemDataTables[id];
			itemDataTables[id] = nullptr;
		}

		return id;
	}
}

void deleteItem(int itemID) {
	if (enabledKeys[EnableKeys::ItemDelete]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("ItemDelete", &Engine::items[itemID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&deleteItemHook);
				Engine::deleteItem(itemID);
			}
			if (func != sol::nil) {
				auto res = func("PostItemDelete", &Engine::items[itemID]);
				noLuaCallError(&res);
			}
			if (itemDataTables[itemID]) {
				delete itemDataTables[itemID];
				itemDataTables[itemID] = nullptr;
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&deleteItemHook);
		Engine::deleteItem(itemID);

		if (itemDataTables[itemID]) {
			delete itemDataTables[itemID];
			itemDataTables[itemID] = nullptr;
		}
	}
}

int createVehicle(int type, Vector* pos, Vector* vel, RotMatrix* rot,
                  int color) {
	if (enabledKeys[EnableKeys::VehicleCreate]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res =
			    func("VehicleCreate", &Engine::vehicleTypes[type], pos, rot, color);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			int id;
			{
				subhook::ScopedHookRemove remove(&createVehicleHook);
				id = Engine::createVehicle(type, pos, vel, rot, color);

				if (id != -1 && vehicleDataTables[id]) {
					delete vehicleDataTables[id];
					vehicleDataTables[id] = nullptr;
				}
			}
			if (id != -1 && func != sol::nil) {
				auto res = func("PostVehicleCreate", &Engine::vehicles[id]);
				noLuaCallError(&res);
			}
			return id;
		}
		return -1;
	} else {
		subhook::ScopedHookRemove remove(&createVehicleHook);
		int id = Engine::createVehicle(type, pos, vel, rot, color);

		if (id != -1 && vehicleDataTables[id]) {
			delete vehicleDataTables[id];
			vehicleDataTables[id] = nullptr;
		}

		return id;
	}
}

void deleteVehicle(int vehicleID) {
	if (enabledKeys[EnableKeys::VehicleDelete]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("VehicleDelete", &Engine::vehicles[vehicleID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&deleteVehicleHook);
				Engine::deleteVehicle(vehicleID);
			}
			if (func != sol::nil) {
				auto res = func("PostVehicleDelete", &Engine::vehicles[vehicleID]);
				noLuaCallError(&res);
			}
			if (vehicleDataTables[vehicleID]) {
				delete vehicleDataTables[vehicleID];
				vehicleDataTables[vehicleID] = nullptr;
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&deleteVehicleHook);
		Engine::deleteVehicle(vehicleID);

		if (vehicleDataTables[vehicleID]) {
			delete vehicleDataTables[vehicleID];
			vehicleDataTables[vehicleID] = nullptr;
		}
	}
}

int createRigidBody(int type, Vector* pos, RotMatrix* rot, Vector* vel,
                    Vector* scale, float mass) {
	int id;
	{
		subhook::ScopedHookRemove remove(&createRigidBodyHook);
		id = Engine::createRigidBody(type, pos, rot, vel, scale, mass);
	}
	if (id != -1 && bodyDataTables[id]) {
		delete bodyDataTables[id];
		bodyDataTables[id] = nullptr;
	}
	return id;
}

int linkItem(int itemID, int childItemID, int parentHumanID, int slot) {
	if (enabledKeys[EnableKeys::ItemLink]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func(
			    "ItemLink", &Engine::items[itemID],
			    childItemID == -1 ? nullptr : &Engine::items[childItemID],
			    parentHumanID == -1 ? nullptr : &Engine::humans[parentHumanID], slot);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			int worked;
			{
				subhook::ScopedHookRemove remove(&linkItemHook);
				worked = Engine::linkItem(itemID, childItemID, parentHumanID, slot);
			}
			if (func != sol::nil) {
				auto res =
				    func("PostItemLink", &Engine::items[itemID],
				         childItemID == -1 ? nullptr : &Engine::items[childItemID],
				         parentHumanID == -1 ? nullptr : &Engine::humans[parentHumanID],
				         slot, (bool)worked);
				noLuaCallError(&res);
			}
			return worked;
		}
		return 0;
	} else {
		subhook::ScopedHookRemove remove(&linkItemHook);
		return Engine::linkItem(itemID, childItemID, parentHumanID, slot);
	}
}

void itemComputerInput(int itemID, unsigned int character) {
	if (enabledKeys[EnableKeys::ItemComputerInput]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("ItemComputerInput", &Engine::items[itemID], character);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&itemComputerInputHook);
				Engine::itemComputerInput(itemID, character);
			}
			if (func != sol::nil) {
				auto res =
				    func("PostItemComputerInput", &Engine::items[itemID], character);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&itemComputerInputHook);
		Engine::itemComputerInput(itemID, character);
	}
}

void humanApplyDamage(int humanID, int bone, int unk, int damage) {
	if (enabledKeys[EnableKeys::HumanDamage]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("HumanDamage", &Engine::humans[humanID], bone, damage);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&humanApplyDamageHook);
				Engine::humanApplyDamage(humanID, bone, unk, damage);
			}
			if (func != sol::nil) {
				auto res =
				    func("PostHumanDamage", &Engine::humans[humanID], bone, damage);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&humanApplyDamageHook);
		Engine::humanApplyDamage(humanID, bone, unk, damage);
	}
}

void humanCollisionVehicle(int humanID, int vehicleID) {
	if (enabledKeys[EnableKeys::HumanCollisionVehicle]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("HumanCollisionVehicle", &Engine::humans[humanID],
			                &Engine::vehicles[vehicleID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&humanCollisionVehicleHook);
				Engine::humanCollisionVehicle(humanID, vehicleID);
			}
			if (func != sol::nil) {
				auto res = func("PostHumanCollisionVehicle", &Engine::humans[humanID],
				                &Engine::vehicles[vehicleID]);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&humanCollisionVehicleHook);
		Engine::humanCollisionVehicle(humanID, vehicleID);
	}
}

void humanLimbInverseKinematics(int humanID, int trunkBoneID, int branchBoneID,
                                Vector* destination, RotMatrix* destinationAxis,
                                Vector* vecA, float a, float rot,
                                float strength, float* d /* Quaternion? */,
                                Vector* vecB, Vector* vecC, Vector* vecD,
                                char flags) {
	if (enabledKeys[EnableKeys::HumanLimbInverseKinematics]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];

		if (func != sol::nil) {
			Float wrappedA = {a};
			Float wrappedRot = {rot};
			Float wrappedStrength = {strength};
			Integer wrappedFlags = {+flags};

			auto res = func("HumanLimbInverseKinematics", &Engine::humans[humanID],
			                trunkBoneID, branchBoneID, destination, destinationAxis,
			                vecA, &wrappedA, &wrappedRot, &wrappedStrength, vecB,
			                vecC, vecD, &wrappedFlags);
			if (noLuaCallError(&res)) noParent = (bool)res;

			a = wrappedA.value;
			rot = wrappedRot.value;
			strength = wrappedStrength.value;
			flags = wrappedFlags.value;
		}
		if (!noParent) {
			subhook::ScopedHookRemove remove(&humanLimbInverseKinematicsHook);
			Engine::humanLimbInverseKinematics(
			    humanID, trunkBoneID, branchBoneID, destination, destinationAxis,
			    vecA, a, rot, strength, d, vecB, vecC, vecD, flags);
		}
	} else {
		subhook::ScopedHookRemove remove(&humanLimbInverseKinematicsHook);
		Engine::humanLimbInverseKinematics(
		    humanID, trunkBoneID, branchBoneID, destination, destinationAxis, vecA,
		    a, rot, strength, d, vecB, vecC, vecD, flags);
	}
}

void grenadeExplosion(int itemID) {
	if (enabledKeys[EnableKeys::GrenadeExplode]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("GrenadeExplode", &Engine::items[itemID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&grenadeExplosionHook);
				Engine::grenadeExplosion(itemID);
			}
			if (func != sol::nil) {
				auto res = func("PostGrenadeExplode", &Engine::items[itemID]);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&grenadeExplosionHook);
		Engine::grenadeExplosion(itemID);
	}
}

int serverPlayerMessage(int playerID, char* message) {
	if (enabledKeys[EnableKeys::PlayerChat]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("PlayerChat", &Engine::players[playerID], message);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			subhook::ScopedHookRemove remove(&serverPlayerMessageHook);
			return Engine::serverPlayerMessage(playerID, message);
		}
		return 1;
	} else {
		subhook::ScopedHookRemove remove(&serverPlayerMessageHook);
		return Engine::serverPlayerMessage(playerID, message);
	}
}

void playerAI(int playerID) {
	if (enabledKeys[EnableKeys::PlayerAI]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("PlayerAI", &Engine::players[playerID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&playerAIHook);
				Engine::playerAI(playerID);
			}
			if (func != sol::nil) {
				auto res = func("PostPlayerAI", &Engine::players[playerID]);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&playerAIHook);
		Engine::playerAI(playerID);
	}
}

void playerDeathTax(int playerID) {
	if (enabledKeys[EnableKeys::PlayerDeathTax]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("PlayerDeathTax", &Engine::players[playerID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&playerDeathTaxHook);
				Engine::playerDeathTax(playerID);
			}
			if (func != sol::nil) {
				auto res = func("PostPlayerDeathTax", &Engine::players[playerID]);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&playerDeathTaxHook);
		Engine::playerDeathTax(playerID);
	}
}

void accountDeathTax(int accountID) {
	if (enabledKeys[EnableKeys::AccountDeathTax]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("AccountDeathTax", &Engine::accounts[accountID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&accountDeathTaxHook);
				Engine::accountDeathTax(accountID);
			}
			if (func != sol::nil) {
				auto res = func("PostAccountDeathTax", &Engine::accounts[accountID]);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&accountDeathTaxHook);
		Engine::accountDeathTax(accountID);
	}
}

void playerGiveWantedLevel(int playerID, int victimPlayerID, int basePoints) {
	if (enabledKeys[EnableKeys::PlayerGiveWantedLevel]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			Integer wrappedBasePoints = {basePoints};

			auto res = func("PlayerGiveWantedLevel", &Engine::players[playerID],
			                &Engine::players[victimPlayerID], &wrappedBasePoints);
			if (noLuaCallError(&res)) noParent = (bool)res;

			basePoints = wrappedBasePoints.value;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&playerGiveWantedLevelHook);
				Engine::playerGiveWantedLevel(playerID, victimPlayerID, basePoints);
			}
			if (func != sol::nil) {
				auto res = func("PostPlayerGiveWantedLevel", &Engine::players[playerID],
				                &Engine::players[victimPlayerID], basePoints);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&playerGiveWantedLevelHook);
		Engine::playerGiveWantedLevel(playerID, victimPlayerID, basePoints);
	}
}

void addCollisionRigidBodyOnRigidBody(int aBodyID, int bBodyID,
                                      Vector* aLocalPos, Vector* bLocalPos,
                                      Vector* normal, float a, float b, float c,
                                      float d) {
	if (enabledKeys[EnableKeys::CollideBodies]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("CollideBodies", &Engine::bodies[aBodyID],
			                &Engine::bodies[bBodyID], aLocalPos, bLocalPos, normal, a,
			                b, c, d);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			subhook::ScopedHookRemove remove(&addCollisionRigidBodyOnRigidBodyHook);
			Engine::addCollisionRigidBodyOnRigidBody(aBodyID, bBodyID, aLocalPos,
			                                         bLocalPos, normal, a, b, c, d);
		}
	} else {
		subhook::ScopedHookRemove remove(&addCollisionRigidBodyOnRigidBodyHook);
		Engine::addCollisionRigidBodyOnRigidBody(aBodyID, bBodyID, aLocalPos,
		                                         bLocalPos, normal, a, b, c, d);
	}
}

/*
Type:
0 = Chat
1 = Speaking
2 = Item (Phone)
3 = MOTD
4 = To Admins
5 = Billboard
6 = To Player (Crim)
*/
void createEventMessage(int speakerType, char* message, int speakerID,
                        int distance) {
	if (enabledKeys[EnableKeys::EventMessage]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res =
			    func("EventMessage", speakerType, message, speakerID, distance);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&createEventMessageHook);
				Engine::createEventMessage(speakerType, message, speakerID, distance);
			}
			if (func != sol::nil) {
				auto res =
				    func("PostEventMessage", speakerType, message, speakerID, distance);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&createEventMessageHook);
		Engine::createEventMessage(speakerType, message, speakerID, distance);
	}
}

void createEventUpdatePlayer(int id) {
	if (enabledKeys[EnableKeys::EventUpdatePlayer]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("EventUpdatePlayer", &Engine::players[id]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&createEventUpdatePlayerHook);
				Engine::createEventUpdatePlayer(id);
			}
			if (func != sol::nil) {
				auto res = func("PostEventUpdatePlayer", &Engine::players[id]);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&createEventUpdatePlayerHook);
		Engine::createEventUpdatePlayer(id);
	}
}

void createEventUpdateVehicle(int vehicleID, int updateType, int partID,
                              Vector* pos, Vector* normal) {
	if (enabledKeys[EnableKeys::EventUpdateVehicle]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("EventUpdateVehicle", &Engine::vehicles[vehicleID],
			                updateType, partID, pos, normal);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&createEventUpdateVehicleHook);
				Engine::createEventUpdateVehicle(vehicleID, updateType, partID, pos,
				                                 normal);
			}
			if (func != sol::nil) {
				auto res = func("PostEventUpdateVehicle", &Engine::vehicles[vehicleID],
				                updateType, partID, pos, normal);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&createEventUpdateVehicleHook);
		Engine::createEventUpdateVehicle(vehicleID, updateType, partID, pos,
		                                 normal);
	}
}

void createEventBullet(int bulletType, Vector* pos, Vector* vel, int itemID) {
	if (enabledKeys[EnableKeys::EventBullet]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res =
			    func("EventBullet", bulletType, pos, vel, &Engine::items[itemID]);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&createEventBulletHook);
				Engine::createEventBullet(bulletType, pos, vel, itemID);
			}
			if (func != sol::nil) {
				auto res = func("PostEventBullet", bulletType, pos, vel,
				                &Engine::items[itemID]);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&createEventBulletHook);
		Engine::createEventBullet(bulletType, pos, vel, itemID);
	}
}

void createEventBulletHit(int unk, int hitType, Vector* pos, Vector* normal) {
	if (enabledKeys[EnableKeys::EventBulletHit]) {
		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res = func("EventBulletHit", hitType, pos, normal);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}
		if (!noParent) {
			{
				subhook::ScopedHookRemove remove(&createEventBulletHitHook);
				Engine::createEventBulletHit(unk, hitType, pos, normal);
			}
			if (func != sol::nil) {
				auto res = func("PostEventBulletHit", hitType, pos, normal);
				noLuaCallError(&res);
			}
		}
	} else {
		subhook::ScopedHookRemove remove(&createEventBulletHitHook);
		Engine::createEventBulletHit(unk, hitType, pos, normal);
	}
}

int lineIntersectHuman(int humanID, Vector* posA, Vector* posB, float padding) {
	if (enabledKeys[EnableKeys::LineIntersectHuman]) {
		int didHit;
		{
			subhook::ScopedHookRemove remove(&lineIntersectHumanHook);
			didHit = Engine::lineIntersectHuman(humanID, posA, posB, padding);
		}

		if (!didHit) {
			return didHit;
		}

		bool noParent = false;
		sol::protected_function func = (*lua)["hook"]["run"];
		if (func != sol::nil) {
			auto res =
			    func("LineIntersectHuman", &Engine::humans[humanID], posA, posB);
			if (noLuaCallError(&res)) noParent = (bool)res;
		}

		return !noParent;
	} else {
		subhook::ScopedHookRemove remove(&lineIntersectHumanHook);
		return Engine::lineIntersectHuman(humanID, posA, posB, padding);
	}
}
};  // namespace Hooks