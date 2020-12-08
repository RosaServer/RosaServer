#include "api.h"
#include "engine.h"
#include "hooks.h"

bool initialized = false;
bool shouldReset = false;

sol::state* lua;
std::string hookMode;

sol::table* playerDataTables[maxNumberOfPlayers];
sol::table* humanDataTables[maxNumberOfHumans];
sol::table* itemDataTables[maxNumberOfItems];
sol::table* vehicleDataTables[maxNumberOfVehicles];
sol::table* bodyDataTables[maxNumberOfRigidBodies];

std::mutex stateResetMutex;
std::queue<LuaHTTPRequest> requestQueue;
std::mutex requestQueueMutex;
std::queue<LuaHTTPResponse> responseQueue;
std::mutex responseQueueMutex;

subhook::Hook subRosaPutsHook;
subhook::Hook subRosa__printf_chkHook;
subhook::Hook resetGameHook;
subhook::Hook logicSimulationHook;
subhook::Hook logicSimulationRaceHook;
subhook::Hook logicSimulationRoundHook;
subhook::Hook logicSimulationWorldHook;
subhook::Hook logicSimulationTerminatorHook;
subhook::Hook logicSimulationCoopHook;
subhook::Hook logicSimulationVersusHook;
subhook::Hook logicPlayerActionsHook;
subhook::Hook physicsSimulationHook;
subhook::Hook serverReceiveHook;
subhook::Hook serverSendHook;
subhook::Hook bulletSimulationHook;
subhook::Hook saveAccountsServerHook;
subhook::Hook createAccountByJoinTicketHook;
subhook::Hook serverSendConnectResponseHook;
subhook::Hook linkItemHook;
subhook::Hook itemComputerInputHook;
subhook::Hook humanApplyDamageHook;
subhook::Hook humanCollisionVehicleHook;
subhook::Hook humanGrabbingHook;
subhook::Hook grenadeExplosionHook;
subhook::Hook serverPlayerMessageHook;
subhook::Hook playerAIHook;
subhook::Hook playerDeathTaxHook;
subhook::Hook addCollisionRigidBodyOnRigidBodyHook;
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
subhook::Hook createEventUpdatePlayerFinanceHook;
subhook::Hook createEventUpdateVehicleHook;
subhook::Hook createEventBulletHitHook;
subhook::Hook lineIntersectHumanHook;

namespace Engine {
int* gameType;
char* mapName;
char* loadedMapName;
int* gameState;
int* gameTimer;
unsigned int* sunTime;

RayCastResult* lineIntersectResult;

Connection* connections;
Account* accounts;
Player* players;
Human* humans;
Vehicle* vehicles;
ItemType* itemTypes;
Item* items;
Bullet* bullets;
RigidBody* bodies;
Bond* bonds;
Street* streets;
StreetIntersection* streetIntersections;

unsigned int* numConnections;
unsigned int* numBullets;
unsigned int* numStreets;
unsigned int* numStreetIntersections;

subRosaPutsFunc subRosaPuts;
subRosa__printf_chkFunc subRosa__printf_chk;

voidFunc resetGame;

voidFunc logicSimulation;
voidFunc logicSimulationRace;
voidFunc logicSimulationRound;
voidFunc logicSimulationWorld;
voidFunc logicSimulationTerminator;
voidFunc logicSimulationCoop;
voidFunc logicSimulationVersus;
voidIndexFunc logicPlayerActions;

voidFunc physicsSimulation;
serverReceiveFunc serverReceive;
voidFunc serverSend;
voidFunc bulletSimulation;
voidFunc bulletTimeToLive;

voidFunc saveAccountsServer;

createAccountByJoinTicketFunc createAccountByJoinTicket;
serverSendConnectResponseFunc serverSendConnectResponse;

scenarioArmHumanFunc scenarioArmHuman;
linkItemFunc linkItem;
itemSetMemoFunc itemSetMemo;
itemComputerTransmitLineFunc itemComputerTransmitLine;
voidIndexFunc itemComputerIncrementLine;
itemComputerInputFunc itemComputerInput;
humanApplyDamageFunc humanApplyDamage;
humanCollisionVehicleFunc humanCollisionVehicle;
voidIndexFunc humanGrabbing;
voidIndexFunc grenadeExplosion;
serverPlayerMessageFunc serverPlayerMessage;
voidIndexFunc playerAI;
voidIndexFunc playerDeathTax;
createBondRigidBodyToRigidBodyFunc createBondRigidBodyToRigidBody;
createBondRigidBodyRotRigidBodyFunc createBondRigidBodyRotRigidBody;
createBondRigidBodyToLevelFunc createBondRigidBodyToLevel;
addCollisionRigidBodyOnRigidBodyFunc addCollisionRigidBodyOnRigidBody;
addCollisionRigidBodyOnLevelFunc addCollisionRigidBodyOnLevel;

createPlayerFunc createPlayer;
voidIndexFunc deletePlayer;
createHumanFunc createHuman;
voidIndexFunc deleteHuman;
createItemFunc createItem;
voidIndexFunc deleteItem;
createRopeFunc createRope;
createVehicleFunc createVehicle;
voidIndexFunc deleteVehicle;
createRigidBodyFunc createRigidBody;

createEventMessageFunc createEventMessage;
voidIndexFunc createEventUpdatePlayer;
voidIndexFunc createEventUpdatePlayerFinance;
voidIndexFunc createEventCreateVehicle;
createEventUpdateVehicleFunc createEventUpdateVehicle;
createEventSoundFunc createEventSound;
createEventExplosionFunc createEventExplosion;
createEventBulletHitFunc createEventBulletHit;

lineIntersectLevelFunc lineIntersectLevel;
lineIntersectHumanFunc lineIntersectHuman;
lineIntersectVehicleFunc lineIntersectVehicle;
lineIntersectTriangleFunc lineIntersectTriangle;
};  // namespace Engine