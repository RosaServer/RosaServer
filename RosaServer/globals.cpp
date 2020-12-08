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

subhook::Hook subRosaPutsHook;
subRosaPutsFunc subRosaPuts;
subhook::Hook subRosa__printf_chkHook;
subRosa__printf_chkFunc subRosa__printf_chk;

subhook::Hook resetGameHook;
voidFunc resetGame;

subhook::Hook logicSimulationHook;
voidFunc logicSimulation;
subhook::Hook logicSimulationRaceHook;
voidFunc logicSimulationRace;
subhook::Hook logicSimulationRoundHook;
voidFunc logicSimulationRound;
subhook::Hook logicSimulationWorldHook;
voidFunc logicSimulationWorld;
subhook::Hook logicSimulationTerminatorHook;
voidFunc logicSimulationTerminator;
subhook::Hook logicSimulationCoopHook;
voidFunc logicSimulationCoop;
subhook::Hook logicSimulationVersusHook;
voidFunc logicSimulationVersus;
subhook::Hook logicPlayerActionsHook;
voidIndexFunc logicPlayerActions;

subhook::Hook physicsSimulationHook;
voidFunc physicsSimulation;
subhook::Hook serverReceiveHook;
serverrecv_func serverReceive;
subhook::Hook serverSendHook;
voidFunc serverSend;
subhook::Hook bulletSimulationHook;
voidFunc bulletSimulation;
voidFunc bulletTimeToLive;

subhook::Hook saveAccountsServerHook;
voidFunc saveAccountsServer;

subhook::Hook createAccountByJoinTicketHook;
createAccountByJoinTicketFunc createAccountByJoinTicket;
subhook::Hook serverSendConnectResponseHook;
serverSendConnectResponseFunc serverSendConnectResponse;

scenarioArmHumanFunc scenarioArmHuman;
subhook::Hook linkItemHook;
linkItemFunc linkItem;
itemSetMemoFunc itemSetMemo;
itemComputerTransmitLineFunc itemComputerTransmitLine;
voidIndexFunc itemComputerIncrementLine;
subhook::Hook itemComputerInputHook;
itemComputerInputFunc itemComputerInput;
subhook::Hook humanApplyDamageHook;
humanApplyDamageFunc humanApplyDamage;
subhook::Hook humanCollisionVehicleHook;
humanCollisionVehicleFunc humanCollisionVehicle;
subhook::Hook humanGrabbingHook;
voidIndexFunc humanGrabbing;
subhook::Hook grenadeExplosionHook;
voidIndexFunc grenadeExplosion;
subhook::Hook serverPlayerMessageHook;
server_playermessage_func serverPlayerMessage;
subhook::Hook playerAIHook;
voidIndexFunc playerAI;
subhook::Hook playerDeathTaxHook;
voidIndexFunc playerDeathTax;
createBondRigidBodyToRigidBodyFunc createBondRigidBodyToRigidBody;
createBondRigidBodyRotRigidBodyFunc createBondRigidBodyRotRigidBody;
createBondRigidBodyToLevelFunc createBondRigidBodyToLevel;
subhook::Hook addCollisionRigidBodyOnRigidBodyHook;
addCollisionRigidBodyOnRigidBodyFunc addCollisionRigidBodyOnRigidBody;
addCollisionRigidBodyOnLevelFunc addCollisionRigidBodyOnLevel;

subhook::Hook createPlayerHook;
createPlayerFunc createPlayer;
subhook::Hook deletePlayerHook;
voidIndexFunc deletePlayer;
subhook::Hook createHumanHook;
createHumanFunc createHuman;
subhook::Hook deleteHumanHook;
voidIndexFunc deleteHuman;
subhook::Hook createItemHook;
createItemFunc createItem;
subhook::Hook deleteItemHook;
voidIndexFunc deleteItem;
createRopeFunc createRope;
subhook::Hook createVehicleHook;
createVehicleFunc createVehicle;
subhook::Hook deleteVehicleHook;
voidIndexFunc deleteVehicle;
subhook::Hook createRigidBodyHook;
createRigidBodyFunc createRigidBody;

subhook::Hook createEventMessageHook;
createEventMessageFunc createEventMessage;
subhook::Hook createEventUpdatePlayerHook;
voidIndexFunc createEventUpdatePlayer;
subhook::Hook createEventUpdatePlayerFinanceHook;
voidIndexFunc createEventUpdatePlayerFinance;
voidIndexFunc createEventCreateVehicle;
subhook::Hook createEventUpdateVehicleHook;
createEventUpdateVehicleFunc createEventUpdateVehicle;
createEventSoundFunc createEventSound;
createEventExplosionFunc createEventExplosion;
subhook::Hook createEventBulletHitHook;
createEventBulletHitFunc createEventBulletHit;

lineIntersectLevelFunc lineIntersectLevel;
subhook::Hook lineIntersectHumanHook;
lineIntersectHumanFunc lineIntersectHuman;
lineIntersectVehicleFunc lineIntersectVehicle;
lineIntersectTriangleFunc lineIntersectTriangle;