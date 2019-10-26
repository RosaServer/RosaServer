#include "pch.h"

#include "hooks.h"
#include "api.h"

void __cdecl h_createlevel() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LevelCreate");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		createlevel();
		if (func != sol::nil) {
			auto res = func("PostLevelCreate");
			noLuaCallError(&res);
		}
	}
}

int __cdecl h_createplayer() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PlayerCreate");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		int id = createplayer();
		if (func != sol::nil && id != -1) {
			auto res = func("PostPlayerCreate", &players[id]);
			noLuaCallError(&res);
		}
		return id;
	}
	return -1;
}

void __cdecl h_deleteplayer(int playerID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PlayerDelete", &players[playerID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		deleteplayer(playerID);
		if (func != sol::nil) {
			auto res = func("PostPlayerDelete", &players[playerID]);
			noLuaCallError(&res);
		}
	}
}

int __cdecl h_createhuman(Vector* pos, RotMatrix* rot, int playerID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("HumanCreate", pos, rot, &players[playerID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		int id = createhuman(pos, rot, playerID);
		if (func != sol::nil && id != -1) {
			auto res = func("PostHumanCreate", &humans[id]);
			noLuaCallError(&res);
		}
		return id;
	}
	return -1;
}

void __cdecl h_deletehuman(int humanID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("HumanDelete", &humans[humanID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		deletehuman(humanID);
		if (func != sol::nil) {
			auto res = func("PostHumanDelete", &humans[humanID]);
			noLuaCallError(&res);
		}
	}
}

BOOL __cdecl h_linkitem(int itemID, int childItemID, int parentHumanID, int slot) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("ItemLink", &items[itemID], childItemID == -1 ? nullptr : &items[childItemID], parentHumanID == -1 ? nullptr : &humans[parentHumanID], slot);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		BOOL worked = linkitem(itemID, childItemID, parentHumanID, slot);
		if (func != sol::nil) {
			auto res = func("PostItemLink", &items[itemID], childItemID == -1 ? nullptr : &items[childItemID], parentHumanID == -1 ? nullptr : &humans[parentHumanID], slot, (bool)worked);
			noLuaCallError(&res);
		}
		return worked;
	}
	return FALSE;
}

int __cdecl h_createitem(int type, Vector* pos, Vector* vel, RotMatrix* rot) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("ItemCreate", type, pos, rot);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		int id = createitem(type, pos, vel, rot);
		if (id != -1 && func != sol::nil) {
			auto res = func("PostItemCreate", &items[id]);
			noLuaCallError(&res);
		}
		return id;
	}
	return -1;
}

int __cdecl h_createvehicle(int type, Vector* pos, Vector* vel, RotMatrix* rot, int color) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("VehicleCreate", type, pos, rot, color);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		int id = createvehicle(type, pos, vel, rot, color);
		if (id != -1 && func != sol::nil) {
			auto res = func("PostVehicleCreate", &vehicles[id]);
			noLuaCallError(&res);
		}
		return id;
	}
	return -1;
}

void __cdecl h_grenadeexplosion(int itemID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("GrenadeExplode", &items[itemID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		grenadeexplosion(itemID);
		if (func != sol::nil) {
			auto res = func("PostGrenadeExplode", &items[itemID]);
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_playerdeathtax(int playerID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PlayerDeathTax", &players[playerID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
		playerdeathtax(playerID);
}

void __cdecl h_human_applydamage(int humanID, int bone, int unk, int damage) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("HumanDamage", &humans[humanID], bone, damage);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		human_applydamage(humanID, bone, unk, damage);
		if (func != sol::nil) {
			auto res = func("PostHumanDamage", &humans[humanID], bone, damage);
			noLuaCallError(&res);
		}
	}
}

int __cdecl h_chat(int playerID, char* message) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PlayerChat", &players[playerID], message);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
		return chat(playerID, message);
	return 1;
}

/*
Type:
0 = Chat
1 = Speaking
2 = Item (Phone)
3 = MOTD
4 = To Admins
5 = Billboard
*/
void __cdecl h_createevent_message(int type, char* message, int speakerID, int distance) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventMessage", type, message, speakerID, distance);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		createevent_message(type, message, speakerID, distance);
		if (func != sol::nil) {
			auto res = func("PostEventMessage", type, message, speakerID, distance);
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_createevent_updateitem(int id) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventUpdateItem", &items[id]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		createevent_updateitem(id);
		if (func != sol::nil) {
			auto res = func("PostEventUpdateItem", &items[id]);
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_createevent_updateplayer(int id) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventUpdatePlayer", &players[id]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		createevent_updateplayer(id);
		if (func != sol::nil) {
			auto res = func("PostEventUpdatePlayer", &players[id]);
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_createevent_updateplayer_finance(int id) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventUpdatePlayerFinance", &players[id]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		createevent_updateplayer_finance(id);
		if (func != sol::nil) {
			auto res = func("PostEventUpdatePlayerFinance", &players[id]);
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_createevent_updateobject(int vehicleID, int updateType, int partID, Vector* pos, Vector* normal) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventUpdateVehicle", &vehicles[vehicleID], updateType, partID, pos, normal);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		createevent_updateobject(vehicleID, updateType, partID, pos, normal);
		if (func != sol::nil) {
			auto res = func("PostEventUpdateVehicle", &vehicles[vehicleID], updateType, partID, pos, normal);
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_createevent_sound(int soundType, Vector* pos, float volume, float pitch) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventSound", soundType, pos, volume, pitch);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		createevent_sound(soundType, pos, volume, pitch);
		if (func != sol::nil) {
			auto res = func("PostEventSound", soundType, pos, volume, pitch);
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_createevent_updatedoor(int team, BOOL isOpen) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventUpdateDoor", team, (bool)isOpen);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		createevent_updatedoor(team, isOpen);
		if (func != sol::nil) {
			auto res = func("PostEventUpdateDoor", team, (bool)isOpen);
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_createevent_bullethit(int unk, int hitType, Vector* pos, Vector* normal) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("EventBulletHit", hitType, pos, normal);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		createevent_bullethit(unk, hitType, pos, normal);
		if (func != sol::nil) {
			auto res = func("EventBulletHit", hitType, pos, normal);
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_playerai(int playerID) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PlayerAI", &players[playerID]);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		playerai(playerID);
		if (func != sol::nil) {
			auto res = func("PostPlayerAI", &players[playerID]);
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_rigidbodysimulation() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PhysicsRigidBodies");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		rigidbodysimulation();
		if (func != sol::nil) {
			auto res = func("PostPhysicsRigidBodies");
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_objectsimulation() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PhysicsVehicles");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		objectsimulation();
		if (func != sol::nil) {
			auto res = func("PostPhysicsVehicles");
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_itemsimulation() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PhysicsItems");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		itemsimulation();
		if (func != sol::nil) {
			auto res = func("PostPhysicsItems");
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_humansimulation() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PhysicsHumans");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		humansimulation();
		if (func != sol::nil) {
			auto res = func("PostPhysicsHumans");
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_logicsimulation() {
	if (shouldReset) {
		shouldReset = false;
		luaInit(true);

		hookAndReset(RESET_REASON_LUARESET);
	}

	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("Logic");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		logicsimulation();
		if (func != sol::nil) {
			auto res = func("PostLogic");
			noLuaCallError(&res);
		}
	}

	while (!consoleQueue.empty()) {
		if (func != sol::nil) {
			auto res = func("ConsoleInput", consoleQueue.front());
			noLuaCallError(&res);
		}
		consoleQueue.pop();
	}

	while (!responseQueue.empty()) {
		if (func != sol::nil) {
			auto res = responseQueue.front();
			if (res.responded) {
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
			else {
				auto resf = func("HTTPResponse", res.identifier);
				noLuaCallError(&resf);
			}
		}
		responseQueue.pop();
	}
}

void __cdecl h_logicsimulation_race() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LogicRace");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		logicsimulation_race();
		if (func != sol::nil) {
			auto res = func("PostLogicRace");
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_logicsimulation_round() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LogicRound");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		logicsimulation_round();
		if (func != sol::nil) {
			auto res = func("PostLogicRound");
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_logicsimulation_world() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LogicWorld");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		logicsimulation_world();
		if (func != sol::nil) {
			auto res = func("PostLogicWorld");
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_logicsimulation_terminator() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LogicTerminator");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		logicsimulation_terminator();
		if (func != sol::nil) {
			auto res = func("PostLogicTerminator");
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_logicsimulation_coop() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LogicCoop");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		logicsimulation_coop();
		if (func != sol::nil) {
			auto res = func("PostLogicCoop");
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_logicsimulation_versus() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("LogicVersus");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		logicsimulation_versus();
		if (func != sol::nil) {
			auto res = func("PostLogicVersus");
			noLuaCallError(&res);
		}
	}
}

int __cdecl h_recvpacket() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("InPacket");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		int ret = recvpacket();
		if (func != sol::nil) {
			auto res = func("PostInPacket");
			noLuaCallError(&res);
		}
		return ret;
	}
	return -1;
}

void __cdecl h_sendpacket() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("SendPacket");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		sendpacket();
		if (func != sol::nil) {
			auto res = func("PostSendPacket");
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_bulletsimulation() {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("PhysicsBullets");
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent) {
		bulletsimulation();
		if (func != sol::nil) {
			auto res = func("PostPhysicsBullets");
			noLuaCallError(&res);
		}
	}
}

void __cdecl h_resetgame() {
	if (!initialized) {
		initialized = true;
		luaInit();
		CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)ConsoleThread, DllHandle, NULL, nullptr);
		CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)HTTPThread, DllHandle, NULL, nullptr);
		hookAndReset(RESET_REASON_BOOT);
	}
	else {
		hookAndReset(RESET_REASON_ENGINECALL);
	}
}

void __cdecl h_scenario_createtraffic3(int density) {
	bool noParent = false;
	sol::protected_function func = (*lua)["hook"]["run"];
	if (func != sol::nil) {
		auto res = func("CreateTraffic", density);
		if (noLuaCallError(&res))
			noParent = (bool)res;
	}
	if (!noParent)
		scenario_createtraffic3(density);
}