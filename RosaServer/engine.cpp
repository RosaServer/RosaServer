#include "engine.h"

namespace Engine {
unsigned int* version;
unsigned int* subVersion;
char* serverName;
unsigned int* serverPort;
unsigned int* numEvents;

int* packetSize;
unsigned char* packet;

int* gameType;
char* mapName;
char* loadedMapName;
int* gameState;
int* gameTimer;
int* ticksSinceReset;
unsigned int* sunTime;

int* serverMaxBytesPerSecond;
char* adminPassword;
int* isPassworded;
char* password;
int* maxPlayers;

namespace World {
int* traffic;
int* startCash;
int* minCash;
bool* showJoinExit;
bool* respawnTeam;
namespace Crime {
int* civCiv;
int* civTeam;
int* teamCiv;
int* teamTeam;
int* teamTeamInBase;
int* noSpawn;
};  // namespace Crime
};  // namespace World

namespace Round {
int* roundTime;
int* startCash;
bool* weekly;
bool* bonusRatio;
int* teamDamage;
};  // namespace Round

int* isLevelLoaded;
float* gravity;
float originalGravity;

LineIntersectResult* lineIntersectResult;

Connection* connections;
Account* accounts;
Voice* voices;
Player* players;
Human* humans;
ItemType* itemTypes;
Item* items;
VehicleType* vehicleTypes;
Vehicle* vehicles;
Bullet* bullets;
RigidBody* bodies;
Bond* bonds;
Street* streets;
StreetIntersection* streetIntersections;
Building* buildings;

unsigned int* numConnections;
unsigned int* numBullets;
unsigned int* numStreets;
unsigned int* numStreetIntersections;
unsigned int* numBuildings;

subRosaPutsFunc subRosaPuts;
subRosa__printf_chkFunc subRosa__printf_chk;

voidFunc resetGame;
createTrafficFunc createTraffic;

areaCreateBlockFunc areaCreateBlock;
areaDeleteBlockFunc areaDeleteBlock;

voidFunc logicSimulation;
voidFunc logicSimulationRace;
voidFunc logicSimulationRound;
voidFunc logicSimulationWorld;
voidFunc logicSimulationTerminator;
voidFunc logicSimulationCoop;
voidFunc logicSimulationVersus;
voidIndexFunc logicPlayerActions;

voidFunc physicsSimulation;
voidFunc rigidBodySimulation;
serverReceiveFunc serverReceive;
voidFunc serverSend;
packetWriteFunc packetWrite;
calculatePlayerVoiceFunc calculatePlayerVoice;
sendPacketFunc sendPacket;
voidFunc bulletSimulation;
voidFunc bulletTimeToLive;

voidFunc economyCarMarket;
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
humanLimbInverseKinematicsFunc humanLimbInverseKinematics;
voidIndexFunc grenadeExplosion;
serverPlayerMessageFunc serverPlayerMessage;
voidIndexFunc playerAI;
voidIndexFunc playerDeathTax;
voidIndexFunc accountDeathTax;
playerGiveWantedLevelFunc playerGiveWantedLevel;
createBondRigidBodyToRigidBodyFunc createBondRigidBodyToRigidBody;
createBondRigidBodyRotRigidBodyFunc createBondRigidBodyRotRigidBody;
createBondRigidBodyToLevelFunc createBondRigidBodyToLevel;
addCollisionRigidBodyOnRigidBodyFunc addCollisionRigidBodyOnRigidBody;
addCollisionRigidBodyOnLevelFunc addCollisionRigidBodyOnLevel;

createBulletFunc createBullet;
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
createEventBulletFunc createEventBullet;
createEventBulletHitFunc createEventBulletHit;

lineIntersectLevelFunc lineIntersectLevel;
lineIntersectHumanFunc lineIntersectHuman;
lineIntersectVehicleFunc lineIntersectVehicle;
lineIntersectTriangleFunc lineIntersectTriangle;
};  // namespace Engine