#include "hooks.h"
#include "api.h"

void h_resetgame()
{
	if (!initialized)
	{
		initialized = true;
		luaInit();

		{
			std::thread thread(consoleThread);
			thread.detach();
		}
		{
			std::thread thread(HTTPThread);
			thread.detach();
		}

		hookAndReset(RESET_REASON_BOOT);
	}
	else
	{
		hookAndReset(RESET_REASON_ENGINECALL);
	}
}

void h_logicsimulation()
{
	if (shouldReset)
	{
		shouldReset = false;
		luaInit(true);

		hookAndReset(RESET_REASON_LUARESET);
	}

	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("Logic");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&logicsimulation_hook);
			logicsimulation();
		}
		if (func != sol::nil)
		{
			auto res = func("PostLogic");
			noLuaCallError(&res);
		}
	}

	{
		std::lock_guard<std::mutex> guard(consoleQueueMutex);
		while (!consoleQueue.empty())
		{
			if (func != sol::nil)
			{
				auto res = func("ConsoleInput", consoleQueue.front());
				noLuaCallError(&res);
			}
			consoleQueue.pop();
		}
	}

	while (true)
	{
		responseQueueMutex.lock();
		if (responseQueue.empty())
		{
			responseQueueMutex.unlock();
			break;
		}
		auto res = responseQueue.front();
		responseQueue.pop();
		responseQueueMutex.unlock();

		if (func != sol::nil)
		{
			if (res.responded)
			{
				sol::table table = lua->create_table();
				table["status"] = res.status;
				table["body"] = res.body;

				sol::table headers = lua->create_table();
				for (const auto& h : res.headers)
					headers[h.first] = h.second;
				table["headers"] = headers;

				auto resf = func("HTTPResponse", res.identifier, table);
				noLuaCallError(&resf);
			}
			else
			{
				auto resf = func("HTTPResponse", res.identifier);
				noLuaCallError(&resf);
			}
		}
	}
}

void h_logicsimulation_race()
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("LogicRace");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&logicsimulation_race_hook);
			logicsimulation_race();
		}
		if (func != sol::nil)
		{
			auto res = func("PostLogicRace");
			noLuaCallError(&res);
		}
	}
}

void h_logicsimulation_round()
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("LogicRound");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&logicsimulation_round_hook);
			logicsimulation_round();
		}
		if (func != sol::nil)
		{
			auto res = func("PostLogicRound");
			noLuaCallError(&res);
		}
	}
}

void h_logicsimulation_world()
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("LogicWorld");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&logicsimulation_world_hook);
			logicsimulation_world();
		}
		if (func != sol::nil)
		{
			auto res = func("PostLogicWorld");
			noLuaCallError(&res);
		}
	}
}

void h_logicsimulation_terminator()
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("LogicTerminator");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&logicsimulation_terminator_hook);
			logicsimulation_terminator();
		}
		if (func != sol::nil)
		{
			auto res = func("PostLogicTerminator");
			noLuaCallError(&res);
		}
	}
}

void h_logicsimulation_coop()
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("LogicCoop");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&logicsimulation_coop_hook);
			logicsimulation_coop();
		}
		if (func != sol::nil)
		{
			auto res = func("PostLogicCoop");
			noLuaCallError(&res);
		}
	}
}

void h_logicsimulation_versus()
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("LogicVersus");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&logicsimulation_versus_hook);
			logicsimulation_versus();
		}
		if (func != sol::nil)
		{
			auto res = func("PostLogicVersus");
			noLuaCallError(&res);
		}
	}
}

void h_physicssimulation()
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("Physics");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&physicssimulation_hook);
			physicssimulation();
		}
		if (func != sol::nil)
		{
			auto res = func("PostPhysics");
			noLuaCallError(&res);
		}
	}
}

int h_serverrecv()
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("InPacket");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		int ret;
		{
			subhook::ScopedHookRemove remove(&serverrecv_hook);
			ret = serverrecv();
		}
		if (func != sol::nil)
		{
			auto res = func("PostInPacket");
			noLuaCallError(&res);
		}
		return ret;
	}
	return -1;
}

void h_serversend()
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("SendPacket");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&serversend_hook);
			serversend();
		}
		if (func != sol::nil)
		{
			auto res = func("PostSendPacket");
			noLuaCallError(&res);
		}
	}
}

void h_bulletsimulation()
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("PhysicsBullets");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&bulletsimulation_hook);
			bulletsimulation();
		}
		if (func != sol::nil)
		{
			auto res = func("PostPhysicsBullets");
			noLuaCallError(&res);
		}
	}
}

void h_saveaccountsserver()
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("AccountsSave");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&saveaccountsserver_hook);
			saveaccountsserver();
		}
		if (func != sol::nil)
		{
			auto res = func("AccountsSave");
			noLuaCallError(&res);
		}
	}
}

int h_createaccount_jointicket(int identifier, unsigned int ticket)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("AccountTicketBegin", identifier, ticket);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		int id;
		{
			subhook::ScopedHookRemove remove(&createaccount_jointicket_hook);
			id = createaccount_jointicket(identifier, ticket);
		}
		if (func != sol::nil)
		{
			auto res = func("AccountTicketFound", id == -1 ? nullptr : &accounts[id]);
			noParent = false;
			if (noLuaCallError(&res))
				noParent = (bool)res;

			if (!noParent)
			{
				auto res = func("PostAccountTicket", id == -1 ? nullptr : &accounts[id]);
				noLuaCallError(&res);
				return id;
			}
			return -1;
		}
		return id;
	}
	return -1;
}

void h_server_sendconnectreponse(unsigned int address, unsigned int port, const char* message)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];

	auto addressString = addressFromInteger(address);

	auto data = lua->create_table();
	data["message"] = message;
	std::string newMessage;

	if (func != sol::nil)
	{
		auto res = func("SendConnectResponse", addressString, port, data);
		if (noLuaCallError(&res))
		{
			noParent = (bool)res;
			newMessage = data["message"];
			message = newMessage.c_str();
		}
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&server_sendconnectreponse_hook);
			server_sendconnectreponse(address, port, message);
		}
		if (func != sol::nil)
		{
			auto res = func("PostSendConnectResponse", addressString, port, data);
			noLuaCallError(&res);
		}
	}
}

int h_createplayer()
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("PlayerCreate");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		int id;
		{
			subhook::ScopedHookRemove remove(&createplayer_hook);
			id = createplayer();

			if (id != -1 && playerDataTables[id])
			{
				delete playerDataTables[id];
				playerDataTables[id] = nullptr;
			}
		}
		if (func != sol::nil && id != -1)
		{
			auto res = func("PostPlayerCreate", &players[id]);
			noLuaCallError(&res);
		}
		return id;
	}
	return -1;
}

void h_deleteplayer(int playerID)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("PlayerDelete", &players[playerID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&deleteplayer_hook);
			deleteplayer(playerID);

			if (playerDataTables[playerID])
			{
				delete playerDataTables[playerID];
				playerDataTables[playerID] = nullptr;
			}
		}
		if (func != sol::nil)
		{
			auto res = func("PostPlayerDelete", &players[playerID]);
			noLuaCallError(&res);
		}
	}
}

int h_createhuman(Vector* pos, RotMatrix* rot, int playerID)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("HumanCreate", pos, rot, &players[playerID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		int id;
		{
			subhook::ScopedHookRemove remove(&createhuman_hook);
			id = createhuman(pos, rot, playerID);

			if (id != -1 && humanDataTables[id])
			{
				delete humanDataTables[id];
				humanDataTables[id] = nullptr;
			}
		}
		if (func != sol::nil && id != -1)
		{
			auto res = func("PostHumanCreate", &humans[id]);
			noLuaCallError(&res);
		}
		return id;
	}
	return -1;
}

void h_deletehuman(int humanID)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("HumanDelete", &humans[humanID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&deletehuman_hook);
			deletehuman(humanID);

			if (humanDataTables[humanID])
			{
				delete humanDataTables[humanID];
				humanDataTables[humanID] = nullptr;
			}
		}
		if (func != sol::nil)
		{
			auto res = func("PostHumanDelete", &humans[humanID]);
			noLuaCallError(&res);
		}
	}
}

int h_createitem(int type, Vector* pos, Vector* vel, RotMatrix* rot)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("ItemCreate", type, pos, rot);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		int id;
		{
			subhook::ScopedHookRemove remove(&createitem_hook);
			id = createitem(type, pos, vel, rot);

			if (id != -1 && itemDataTables[id])
			{
				delete itemDataTables[id];
				itemDataTables[id] = nullptr;
			}
		}
		if (id != -1 && func != sol::nil)
		{
			auto res = func("PostItemCreate", &items[id]);
			noLuaCallError(&res);
		}
		return id;
	}
	return -1;
}

void h_deleteitem(int itemID)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("ItemDelete", &items[itemID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&deleteitem_hook);
			deleteitem(itemID);

			if (itemDataTables[itemID])
			{
				delete itemDataTables[itemID];
				itemDataTables[itemID] = nullptr;
			}
		}
		if (func != sol::nil)
		{
			auto res = func("PostItemDelete", &items[itemID]);
			noLuaCallError(&res);
		}
	}
}

int h_createobject(int type, Vector* pos, Vector* vel, RotMatrix* rot, int color)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("VehicleCreate", type, pos, rot, color);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		int id;
		{
			subhook::ScopedHookRemove remove(&createobject_hook);
			id = createobject(type, pos, vel, rot, color);

			if (id != -1 && vehicleDataTables[id])
			{
				delete vehicleDataTables[id];
				vehicleDataTables[id] = nullptr;
			}
		}
		if (id != -1 && func != sol::nil)
		{
			auto res = func("PostVehicleCreate", &vehicles[id]);
			noLuaCallError(&res);
		}
		return id;
	}
	return -1;
}

void h_deleteobject(int vehicleID)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("VehicleDelete", &vehicles[vehicleID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&deleteobject_hook);
			deleteobject(vehicleID);

			if (vehicleDataTables[vehicleID])
			{
				delete vehicleDataTables[vehicleID];
				vehicleDataTables[vehicleID] = nullptr;
			}
		}
		if (func != sol::nil)
		{
			auto res = func("PostVehicleDelete", &vehicles[vehicleID]);
			noLuaCallError(&res);
		}
	}
}

int h_linkitem(int itemID, int childItemID, int parentHumanID, int slot)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("ItemLink", &items[itemID], childItemID == -1 ? nullptr : &items[childItemID], parentHumanID == -1 ? nullptr : &humans[parentHumanID], slot);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		int worked;
		{
			subhook::ScopedHookRemove remove(&linkitem_hook);
			worked = linkitem(itemID, childItemID, parentHumanID, slot);
		}
		if (func != sol::nil)
		{
			auto res = func("PostItemLink", &items[itemID], childItemID == -1 ? nullptr : &items[childItemID], parentHumanID == -1 ? nullptr : &humans[parentHumanID], slot, (bool)worked);
			noLuaCallError(&res);
		}
		return worked;
	}
	return 0;
}

void h_item_computerinput(int itemID, unsigned int character)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("ItemComputerInput", &items[itemID], character);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&item_computerinput_hook);
			item_computerinput(itemID, character);
		}
		if (func != sol::nil)
		{
			auto res = func("PostItemComputerInput", &items[itemID], character);
			noLuaCallError(&res);
		}
	}
}

void h_human_applydamage(int humanID, int bone, int unk, int damage)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("HumanDamage", &humans[humanID], bone, damage);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&human_applydamage_hook);
			human_applydamage(humanID, bone, unk, damage);
		}
		if (func != sol::nil)
		{
			auto res = func("PostHumanDamage", &humans[humanID], bone, damage);
			noLuaCallError(&res);
		}
	}
}

void h_human_collisionvehicle(int humanID, int vehicleID)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("HumanCollisionVehicle", &humans[humanID], &vehicles[vehicleID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&human_collisionvehicle_hook);
			human_collisionvehicle(humanID, vehicleID);
		}
		if (func != sol::nil)
		{
			auto res = func("PostHumanCollisionVehicle", &humans[humanID], &vehicles[vehicleID]);
			noLuaCallError(&res);
		}
	}
}

void h_human_grabbing(int humanID)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("HumanGrabbing", &humans[humanID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&human_grabbing_hook);
			human_grabbing(humanID);
		}
		if (func != sol::nil)
		{
			auto res = func("PostHumanGrabbing", &humans[humanID]);
			noLuaCallError(&res);
		}
	}
}

void h_grenadeexplosion(int itemID)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("GrenadeExplode", &items[itemID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&grenadeexplosion_hook);
			grenadeexplosion(itemID);
		}
		if (func != sol::nil)
		{
			auto res = func("PostGrenadeExplode", &items[itemID]);
			noLuaCallError(&res);
		}
	}
}

int h_server_playermessage(int playerID, char* message)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("PlayerChat", &players[playerID], message);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		subhook::ScopedHookRemove remove(&server_playermessage_hook);
		return server_playermessage(playerID, message);
	}
	return 1;
}

void h_playerai(int playerID)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("PlayerAI", &players[playerID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&playerai_hook);
			playerai(playerID);
		}
		if (func != sol::nil)
		{
			auto res = func("PostPlayerAI", &players[playerID]);
			noLuaCallError(&res);
		}
	}
}

void h_playerdeathtax(int playerID)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("PlayerDeathTax", &players[playerID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&playerdeathtax_hook);
			playerdeathtax(playerID);
		}
		if (func != sol::nil)
		{
			auto res = func("PostPlayerDeathTax", &players[playerID]);
			noLuaCallError(&res);
		}
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
void h_createevent_message(int speakerType, char* message, int speakerID, int distance)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("EventMessage", speakerType, message, speakerID, distance);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&createevent_message_hook);
			createevent_message(speakerType, message, speakerID, distance);
		}
		if (func != sol::nil)
		{
			auto res = func("PostEventMessage", speakerType, message, speakerID, distance);
			noLuaCallError(&res);
		}
	}
}

void h_createevent_updateplayer(int id)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("EventUpdatePlayer", &players[id]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&createevent_updateplayer_hook);
			createevent_updateplayer(id);
		}
		if (func != sol::nil)
		{
			auto res = func("PostEventUpdatePlayer", &players[id]);
			noLuaCallError(&res);
		}
	}
}

void h_createevent_updateplayer_finance(int id)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("EventUpdatePlayerFinance", &players[id]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&createevent_updateplayer_finance_hook);
			createevent_updateplayer_finance(id);
		}
		if (func != sol::nil)
		{
			auto res = func("PostEventUpdatePlayerFinance", &players[id]);
			noLuaCallError(&res);
		}
	}
}

// Doesn't seem to be necessary anymore
/*void h_createevent_updateitem(int id) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventUpdateItem", &items[id]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&createevent_updateitem_hook);
			createevent_updateitem(id);
		}
		if (func != sol::nil) {
			auto res = func("PostEventUpdateItem", &items[id]);
			noLuaCallError(&res);
		}
	}
}*/

void h_createevent_updateobject(int vehicleID, int updateType, int partID, Vector* pos, Vector* normal)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("EventUpdateVehicle", &vehicles[vehicleID], updateType, partID, pos, normal);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&createevent_updateobject_hook);
			createevent_updateobject(vehicleID, updateType, partID, pos, normal);
		}
		if (func != sol::nil)
		{
			auto res = func("PostEventUpdateVehicle", &vehicles[vehicleID], updateType, partID, pos, normal);
			noLuaCallError(&res);
		}
	}
}

// Crashes sometimes, don't know why. Don't care!
/*void h_createevent_sound(int soundType, Vector* pos, float volume, float pitch) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventSound", soundType, pos, volume, pitch);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		{
			subhook::ScopedHookRemove remove(&createevent_sound_hook);
			createevent_sound(soundType, pos, volume, pitch);
		}
		if (func != sol::nil) {
			auto res = func("PostEventSound", soundType, pos, volume, pitch);
			noLuaCallError(&res);
		}
	}
}*/

void h_createevent_bullethit(int unk, int hitType, Vector* pos, Vector* normal)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("EventBulletHit", hitType, pos, normal);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		{
			subhook::ScopedHookRemove remove(&createevent_bullethit_hook);
			createevent_bullethit(unk, hitType, pos, normal);
		}
		if (func != sol::nil)
		{
			auto res = func("PostEventBulletHit", hitType, pos, normal);
			noLuaCallError(&res);
		}
	}
}

int h_lineintersecthuman(int humanID, Vector* posA, Vector* posB)
{
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil)
	{
		auto res = func("LineIntersectHuman", &humans[humanID], posA, posB);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
	{
		subhook::ScopedHookRemove remove(&lineintersecthuman_hook);
		return lineintersecthuman(humanID, posA, posB);
	}
	return 0;
}