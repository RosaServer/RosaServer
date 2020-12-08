#include "engine.h"

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