#pragma once
#include "structs.h"

namespace Engine {
extern unsigned int* version;
extern unsigned int* subVersion;
extern char* serverName;
extern unsigned int* serverPort;

extern int* packetSize;
extern unsigned char* packet;

extern int* gameType;
extern char* mapName;
extern char* loadedMapName;
extern int* gameState;
extern int* gameTimer;
extern int* ticksSinceReset;
extern unsigned int* sunTime;

extern int* serverMaxBytesPerSecond;
extern char* adminPassword;
extern int* isPassworded;
extern char* password;
extern int* maxPlayers;

namespace World {
extern int* traffic;
extern int* startCash;
extern int* minCash;
extern bool* showJoinExit;
extern bool* respawnTeam;
namespace Crime {
extern int* civCiv;
extern int* civTeam;
extern int* teamCiv;
extern int* teamTeam;
extern int* teamTeamInBase;
extern int* noSpawn;
};  // namespace Crime
};  // namespace World

namespace Round {
extern int* roundTime;
extern int* startCash;
extern bool* weekly;
extern bool* bonusRatio;
extern int* teamDamage;
extern int* weekDay;
};  // namespace Round

extern int* isLevelLoaded;
extern float* gravity;
extern float originalGravity;

extern LineIntersectResult* lineIntersectResult;

extern Connection* connections;
extern Account* accounts;
extern Voice* voices;
extern Player* players;
extern Human* humans;
extern ItemType* itemTypes;
extern Item* items;
extern VehicleType* vehicleTypes;
extern Vehicle* vehicles;
extern Bullet* bullets;
extern RigidBody* bodies;
extern Bond* bonds;
extern Street* streets;
extern StreetIntersection* streetIntersections;
extern TrafficCar* trafficCars;
extern Building* buildings;
extern Event* events;

extern unsigned int* numConnections;
extern unsigned int* numBullets;
extern unsigned int* numStreets;
extern unsigned int* numStreetIntersections;
extern unsigned int* numTrafficCars;
extern unsigned int* numBuildings;
extern unsigned short* numEvents;

/*
  Misc
*/

typedef int (*subRosaPutsFunc)(const char* str);
extern subRosaPutsFunc subRosaPuts;
typedef int (*subRosa__printf_chkFunc)(int flag, const char* format, ...);
extern subRosa__printf_chkFunc subRosa__printf_chk;

typedef void (*voidFunc)();
typedef void (*voidIndexFunc)(int id);

typedef void (*createTrafficFunc)(int amount);
typedef void (*aiTrafficCarFunc)(int id);
typedef void (*aiTrafficCarDestinationFunc)(int id, int a, int b, int c, int d);
typedef long (*aiFunc)(float a, int b, int c, int d, int e, int f);

extern voidFunc resetGame;
extern createTrafficFunc createTraffic;
extern voidFunc trafficSimulation;
extern aiTrafficCarFunc aiTrafficCar;
extern aiTrafficCarDestinationFunc aiTrafficCarDestination;

typedef int (*areaCreateBlockFunc)(int zero, int blockX, int blockY, int blockZ,
                                   unsigned int flags, short unk[8]);
extern areaCreateBlockFunc areaCreateBlock;
typedef int (*areaDeleteBlockFunc)(int zero, int blockX, int blockY,
                                   int blockZ);
extern areaDeleteBlockFunc areaDeleteBlock;

extern voidFunc logicSimulation;
extern voidFunc logicSimulationRace;
extern voidFunc logicSimulationRound;
extern voidFunc logicSimulationWorld;
extern voidFunc logicSimulationTerminator;
extern voidFunc logicSimulationCoop;
extern voidFunc logicSimulationVersus;
extern voidIndexFunc logicPlayerActions;

extern voidFunc physicsSimulation;
extern voidFunc rigidBodySimulation;
typedef int (*serverReceiveFunc)();
extern serverReceiveFunc serverReceive;
extern voidFunc serverSend;
typedef int (*packetWriteFunc)(void* source, int elementSize, int elementCount);
extern packetWriteFunc packetWrite;
typedef void (*calculatePlayerVoiceFunc)(int connectionID, int playerID);
extern calculatePlayerVoiceFunc calculatePlayerVoice;
typedef int (*sendPacketFunc)(unsigned int address, unsigned short port);
extern sendPacketFunc sendPacket;
extern voidFunc bulletSimulation;
extern voidFunc bulletTimeToLive;

extern voidFunc economyCarMarket;
extern voidFunc saveAccountsServer;

typedef int (*createAccountByJoinTicketFunc)(int identifier,
                                             unsigned int ticket);
extern createAccountByJoinTicketFunc createAccountByJoinTicket;
typedef void (*serverSendConnectResponseFunc)(unsigned int address,
                                              unsigned int port, int unk,
                                              const char* message);
extern serverSendConnectResponseFunc serverSendConnectResponse;

typedef void (*scenarioArmHumanFunc)(int human, int weapon, int magCount);
extern scenarioArmHumanFunc scenarioArmHuman;

typedef int (*linkItemFunc)(int itemID, int childItemID, int parentHumanID,
                            int slot);
extern linkItemFunc linkItem;

typedef int (*itemSetMemoFunc)(int itemID, const char* memo);
extern itemSetMemoFunc itemSetMemo;
typedef int (*itemComputerTransmitLineFunc)(int itemID, unsigned int line);
extern itemComputerTransmitLineFunc itemComputerTransmitLine;
typedef int (*itemCashAddBillFunc)(int itemID, int zero, int amount);
extern itemCashAddBillFunc itemCashAddBill;
typedef void (*itemCashRemoveBillFunc)(int itemID, int amount);
extern itemCashRemoveBillFunc itemCashRemoveBill;
typedef int (*itemCashGetBillValueFunc)(int itemID);
extern itemCashGetBillValueFunc itemCashGetBillValue;
extern voidIndexFunc itemComputerIncrementLine;
typedef int (*itemComputerInputFunc)(int itemID, unsigned int character);
extern itemComputerInputFunc itemComputerInput;

typedef void (*humanApplyDamageFunc)(int humanID, int bone, int unk,
                                     int damage);
extern humanApplyDamageFunc humanApplyDamage;

typedef void (*humanCollisionVehicleFunc)(int humanID, int vehicleID);
extern humanCollisionVehicleFunc humanCollisionVehicle;

typedef void (*humanLimbInverseKinematicsFunc)(int, int, int, Vector*,
                                               RotMatrix*, Vector*, float,
                                               float, float,
                                               float* /* Quaternion? */,
                                               Vector*, Vector*, Vector*, char);
extern humanLimbInverseKinematicsFunc humanLimbInverseKinematics;

extern voidIndexFunc grenadeExplosion;

typedef int (*serverPlayerMessageFunc)(int playerID, char* message);
extern serverPlayerMessageFunc serverPlayerMessage;
extern voidIndexFunc playerAI;
extern voidIndexFunc playerDeathTax;
extern voidIndexFunc accountDeathTax;

typedef void (*playerGiveWantedLevelFunc)(int playerID, int victimPlayerID,
                                          int basePoints);
extern playerGiveWantedLevelFunc playerGiveWantedLevel;

typedef void (*addCollisionRigidBodyOnRigidBodyFunc)(int aBodyID, int bBodyID,
                                                     Vector* aLocalPos,
                                                     Vector* bLocalPos,
                                                     Vector* normal, float,
                                                     float, float, float);
extern addCollisionRigidBodyOnRigidBodyFunc addCollisionRigidBodyOnRigidBody;

typedef void (*addCollisionRigidBodyOnLevelFunc)(int bodyID, Vector* localPos,
                                                 Vector* normal, float, float,
                                                 float, float);
extern addCollisionRigidBodyOnLevelFunc addCollisionRigidBodyOnLevel;

/*
  Object Handling
*/

typedef int (*createBondRigidBodyToRigidBodyFunc)(int aBodyID, int bBodyID,
                                                  Vector* aLocalPos,
                                                  Vector* bLocalPos);
extern createBondRigidBodyToRigidBodyFunc createBondRigidBodyToRigidBody;
typedef int (*createBondRigidBodyRotRigidBodyFunc)(int aBodyID, int bBodyID);
extern createBondRigidBodyRotRigidBodyFunc createBondRigidBodyRotRigidBody;
typedef int (*createBondRigidBodyToLevelFunc)(int bodyID, Vector* localPos,
                                              Vector* globalPos);
extern createBondRigidBodyToLevelFunc createBondRigidBodyToLevel;
extern voidIndexFunc deleteBond;

typedef int (*createBulletFunc)(int type, Vector* pos, Vector* vel,
                                int playerID);
extern createBulletFunc createBullet;

typedef int (*createPlayerFunc)();
extern createPlayerFunc createPlayer;
extern voidIndexFunc deletePlayer;

typedef int (*createHumanFunc)(Vector* pos, RotMatrix* rot, int playerID);
extern createHumanFunc createHuman;
extern voidIndexFunc deleteHuman;

typedef int (*createItemFunc)(int type, Vector* pos, Vector* vel,
                              RotMatrix* rot);
extern createItemFunc createItem;
extern voidIndexFunc deleteItem;

typedef int (*createRopeFunc)(Vector* pos, RotMatrix* rot);
extern createRopeFunc createRope;

typedef int (*createVehicleFunc)(int type, Vector* pos, Vector* vel,
                                 RotMatrix* rot, int color);
extern createVehicleFunc createVehicle;
extern voidIndexFunc deleteVehicle;

typedef int (*createRigidBodyFunc)(int type, Vector* pos, RotMatrix* rot,
                                   Vector* vel, Vector* scale, float mass);
extern createRigidBodyFunc createRigidBody;

/*
  Events
*/

typedef void (*createEventMessageFunc)(int type, char* message, int speakerID,
                                       int distance);
extern createEventMessageFunc createEventMessage;
// Sends phoneTexture, displayedPhoneNumber
extern voidIndexFunc createEventUpdateItemInfo;
// Sends team, active, isBot, humanID, skinColor, hair, gender, head, necklace,
// eyeColor, tieColor, suitColor, shirtColor, hairColor, name
extern voidIndexFunc createEventUpdatePlayer;
// Sends money, stocks, phoneNumber
extern voidIndexFunc createEventUpdatePlayerFinance;
// Sends type, color
extern voidIndexFunc createEventCreateVehicle;
/*
updateType:
0 = window
1 = tire
2 = body
*/
typedef void (*createEventUpdateVehicleFunc)(int vehicleID, int updateType,
                                             int partID, Vector* pos,
                                             Vector* normal);
extern createEventUpdateVehicleFunc createEventUpdateVehicle;
typedef void (*createEventSoundFunc)(int soundType, Vector* pos, float volume,
                                     float pitch);
extern createEventSoundFunc createEventSound;
typedef void (*createEventExplosionFunc)(int type, Vector* pos);
extern createEventExplosionFunc createEventExplosion;
typedef void (*createEventBulletFunc)(int bulletType, Vector* pos, Vector* vel,
                                      int itemID);
extern createEventBulletFunc createEventBullet;
/*
hitType:
0 = bullet hole
1 = hit body
2 = hit car
3 = blood drip
*/
typedef void (*createEventBulletHitFunc)(int unk, int hitType, Vector* pos,
                                         Vector* normal);
extern createEventBulletHitFunc createEventBulletHit;

/*
  Math
*/

typedef int (*lineIntersectLevelFunc)(Vector* posA, Vector* posB,
                                      int includeCityObjects);
extern lineIntersectLevelFunc lineIntersectLevel;

typedef int (*lineIntersectHumanFunc)(int humanID, Vector* posA, Vector* posB,
                                      float padding);
extern lineIntersectHumanFunc lineIntersectHuman;

typedef int (*lineIntersectVehicleFunc)(int vehicleID, Vector* posA,
                                        Vector* posB, int includeWheels);
extern lineIntersectVehicleFunc lineIntersectVehicle;

typedef int (*lineIntersectTriangleFunc)(Vector* outPos, Vector* normal,
                                         float* outFraction, Vector* posA,
                                         Vector* posB, Vector* triA,
                                         Vector* triB, Vector* triC);
extern lineIntersectTriangleFunc lineIntersectTriangle;
};  // namespace Engine
