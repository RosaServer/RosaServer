#pragma once

#include <cstring>
#include <string>
#include "sol.hpp"

#define MAXNUMOFACCOUNTS 32768
#define MAXNUMOFPLAYERS 256
#define MAXNUMOFHUMANS 256
#define MAXNUMOFITEMTYPES 45
#define MAXNUMOFITEMS 1024
#define MAXNUMOFVEHICLES 512
#define MAXNUMOFRIGIDBODIES 8192
#define MAXNUMOFBONDS 16384

/*
	Event types:
	0x1		1	bullethit
	0x2		2	message
	0x3		3	createobject (vehicle)
	0x4		4	updateobject
	0x5		5	updateitem
	0x7		7	updateplayer
	0x8		8	updateplayer_finance
	0x9		9	sound
	0xA		10	updatedoor
	0x11	17	updatehuman
	0x14	20	explosion
*/

//188888 bytes (2E1D8)
struct Connection
{
	unsigned int address;
	unsigned int port;	//04
	int unk0;						//08
	int adminVisible;		//0c
	int playerID;				//10
	int unk1;						//14
	int bandwidth;			//18
	int timeoutTime;		//1c
	char unk2[188888 - 32];

	const char* getClass() const
	{
		return "Connection";
	}
	std::string getAddress();
	bool getAdminVisible() const
	{
		return adminVisible;
	}
	void setAdminVisible(bool b)
	{
		adminVisible = b;
	}
};

//112 bytes (70)
struct Account
{
	int subRosaID;
	int phoneNumber;			//04
	long long steamID;		//08
	char name[32];				//10
	int unk0;							//30
	int money;						//34
	int corporateRating;	//38
	int criminalRating;		//3c
	int spawnTimer; //40
	//in-game minutes
	int playTime; //44
	char unk1[0x60 - 0x44 - 4];
	int banTime;	//60
	char unk2[112 - 104];

	const char* getClass() const
	{
		return "Account";
	}
	std::string __tostring() const;
	int getIndex() const;
	char* getName()
	{
		return name;
	}
	std::string getSteamID()
	{
		return std::to_string(steamID);
	}
};

struct RotMatrix;

struct Vector
{
	float x, y, z;

	const char* getClass() const
	{
		return "Vector";
	}
	std::string __tostring() const;
	Vector __add(Vector* other) const;
	Vector __sub(Vector* other) const;
	Vector __mul(float scalar) const;
	Vector __mul_RotMatrix(RotMatrix* rot) const;
	Vector __div(float scalar) const;
	Vector __unm() const;
	void add(Vector* other);
	void mult(float scalar);
	void set(Vector* other);
	Vector clone() const;
	float dist(Vector* other) const;
	float distSquare(Vector* other) const;
};

struct RotMatrix
{
	float x1, y1, z1;
	float x2, y2, z2;
	float x3, y3, z3;

	const char* getClass() const
	{
		return "RotMatrix";
	}
	std::string __tostring() const;
	RotMatrix __mul(RotMatrix* other) const;
	void set(RotMatrix* other);
	RotMatrix clone() const;
};

struct RayCastResult
{
	Vector pos;
	Vector normal;	 //0c
	float fraction;	 //18
	float unk0;			 //1c
	int unk1;				 //20
	int unk2;				 //24
	int unk3;				 //28
	int unk4;				 //2c
	int vehicleFace;		 //30
	int humanBone;	 //34
	int unk6;				 //38
	int unk7;				 //3c
	int unk8;				 //40
	int unk9;				 //44
	int unk10;			 //48
	int unk11;			 //4c
	int unk12;			 //50
	int unk13;			 //54
	int blockX;			 //58
	int blockY;			 //5c
	int blockZ;			 //60
	int unk17;			 //64
	int unk18;			 //68
	int matMaybe;		 //6c
	int unk20;			 //70
	int unk21;			 //74
	int unk22;			 //78
	int unk23;			 //7c
	int unk24;			 //80
};

// Forward decl
struct Human;
struct Vehicle;
struct Item;
struct RigidBody;
struct Bond;

//84 bytes (54)
struct Action
{
	int type;
	int a;
	int b;
	int c;
	int d;
	char text[64];

	const char* getClass() const
	{
		return "Action";
	}
};

//72 bytes (48)
struct MenuButton
{
	int id;
	char text[64];
	int unk;

	const char* getClass() const
	{
		return "MenuButton";
	}
	char* getText()
	{
		return text;
	}
	void setText(const char* newText)
	{
		std::strncpy(text, newText, 63);
	}
};

//14384 bytes (0x3830)
struct Player
{
	int active;
	char name[32];							 //04
		int unk0;										 //24
		int unk1;										 //28
	unsigned int subRosaID;			 //2c
	unsigned int phoneNumber;		 //30
	int isAdmin;								 //34
	unsigned int adminAttempts;	 //38
	unsigned int accountID;			 //3C
		char unk2[0x48 - 0x3C - 4];
	int isReady;					//48
	int money;						//4C
		int unk2a; //50
		int unk2b; //54
	int corporateRating;	//58
	int criminalRating;		//5c
		char unk3[0x84 - 0x5c - 4];
	unsigned int team; //84
	unsigned int teamSwitchTimer; //88
	int stocks; //8c
		int unk4[3];
	int humanID;	//9c
		char unk5[0x164 - 0x9c - 4];
	//0 = none, 1-19 = shop, 2X = base
	int menuTab;	//164
		char unk5_1[0x1b4 - 0x164 - 4];
	int numActions; //1b4
	int lastNumActions; //1b8
		char unk5_2[0x1c8 - 0x1b8 - 4];
	Action actions[64];	 //1c8
		char unk6[0x1b14 - (0x1c8 + (sizeof(Action) * 64))];
	int numMenuButtons; //1b14
	MenuButton menuButtons[32]; //1b18
		char unk6_1[0x2d18 - (0x1b18 + (sizeof(MenuButton) * 32))];
	int isBot;	//2d18
		char unk7a[0x2d34 - 0x2d18 - 4];
	int botHasDestination;	//2d34
	Vector botDestination;	//2d38
		char unk7[0x37a8 - 0x2d38 - 12];
	//int botState;	 //354c
		//int unk8;
	//int botEnemyID;	 //3554
		//char unk9[0x379c - 0x3554 - 4];
		//char unk9[0x37a8 - 0x2d18 - 4];
	int gender;			//37a8
	int skinColor;	//37ac
	int hairColor;	//37b0
	int hair;				//37b4
	int eyeColor;		//37b8
	//0 = casual, 1 = suit
	int model;			//37bc
	int suitColor;	//37c0
	//0 = no tie
	int tieColor;	 //37c4
		int unk10;		 //37c8
	int head;			 //37cc
	int necklace;	 //37d0
		char unk11[14384 - 14292];

	const char* getClass() const
	{
		return "Player";
	}
	std::string __tostring() const;
	int getIndex() const;
	bool getIsActive() const
	{
		return active;
	}
	void setIsActive(bool b)
	{
		active = b;
	}
	sol::table getDataTable() const;
	char* getName()
	{
		return name;
	}
	void setName(const char* newName)
	{
		std::strncpy(name, newName, 31);
	}
	bool getIsAdmin() const
	{
		return isAdmin;
	}
	void setIsAdmin(bool b)
	{
		isAdmin = b;
	}
	bool getIsReady() const
	{
		return isReady;
	}
	void setIsReady(bool b)
	{
		isReady = b;
	}
	bool getIsBot() const
	{
		return isBot;
	}
	void setIsBot(bool b)
	{
		isBot = b;
	}
	Human* getHuman();
	Connection* getConnection();
	Account* getAccount();
	void setAccount(Account* account);
	const Vector* getBotDestination() const;
	void setBotDestination(Vector* vec);
	Action* getAction(unsigned int idx);
	MenuButton* getMenuButton(unsigned int idx);

	void update() const;
	void updateFinance() const;
	void remove() const;
	void sendMessage(const char* message) const;
};

//312 bytes (138)
struct Bone
{
	int bodyID;
	Vector pos;
	Vector pos2;
	Vector vel;
	Vector vel2;
	RotMatrix rot;
	char unk[312 - 88];

	const char* getClass() const
	{
		return "Bone";
	}
};

//14288 bytes (37D0)
struct Human
{
	int active;
	int physicsSim;				//04
	int playerID;							//08
		int unk0;									//0c
		int unk1;									//10
		int unk2;									//14
		int unk3;									//18
	int stamina;									//1c
	int maxStamina; //20
		int unk4; //24
	int vehicleID;						//28
	int vehicleSeat;					//2c
	int lastVehicleID;				//30
	int lastVehicleCooldown;	//34
	//counts down after death
	unsigned int despawnTime;	 //38
	int oldHealth;						 //3c
	//eliminator
	int isImmortal;								 //40
	int unk10;										 //44
	int unk11;										 //48
	int unk12;										 //4c
	unsigned int spawnProtection;	 //50
	int isOnGround;								 //54
	/*
	0=normal
	1=jumping/falling
	2=sliding
	5=getting up?
	*/
	int movementState;	//58
		int unk13;					//5c
	int zoomLevel;			//60
		int unk14;					//64
		int unk15;					//68
		int unk16;					//6c
		int unk17;					//70
		int unk18;					//74
	//max 60
	int damage;				//78
	int isStanding;		//7c
	Vector pos;				//80
	Vector pos2;			//8c
	float viewYaw;		//98
	float viewPitch;	//9c
		char unk19[0x12c - 0x9c - 4];
	float strafeInput;	//12c
		float unk20; //130
	float walkInput;	//134
		char unk21[0x214 - 0x134 - 4];
	/*
	mouse1 = 1		1 << 0
	mouse2 = 2		1 << 1
	space = 4		1 << 2
	ctrl = 8		1 << 3
	shift = 16		1 << 4

	Q = 32			1 << 5
	e = 2048		1 << 11
	r = 4096		1 << 12
	f = 8192		1 << 13

	del = 262144	1 << 18
	z = 524288		1 << 19
	*/
	unsigned int inputFlags;			//214
	unsigned int lastInputFlags;	//218
		char unk22[0x220 - 0x218 - 4];
	Bone bones[16];	 //220
		char unk23[0x32a8 - (0x220 + (sizeof(Bone) * 16))];
	int rightHandOccupied;	//32a8
	int rightHandItemID;		//32ac
		char unk24[0x32d0 - 0x32ac - 4];
	int leftHandOccupied;	 //32d0
	int leftHandItemID;		 //32d4
		char unk25[0x33f8 - 0x32d4 - 4];
	int isGrabbingRight;			 //33f8
	int grabbingRightHumanID;	 //33fc
		int unk26_1; //3400
	int grabbingRightBone;	//3404
		char unk26_2[0x3430 - 0x3404 - 4];
	int isGrabbingLeft;				//3430
	int grabbingLeftHumanID;	//3434
		int unk26_3; //3438
	int grabbingLeftBone;	 //343c
		char unk26_4[0x3528 - 0x343c - 4];
	int health;			 //3528
	int bloodLevel;	 //352c
	int isBleeding;	 //3530
	int chestHP;		 //33534
		int unk26;			 //3538
	int headHP;			 //353c
		int unk27;			 //3540
	int leftArmHP;	 //3544
		int unk28;			 //3548
	int rightArmHP;	 //354c
		int unk29;			 //3550
	int leftLegHP;	 //3554
		int unk30;			 //3558
	int rightLegHP;	 //355c
		char unk31[0x3758 - 0x355c - 4];
	int gender;			//3758
	int head;				//375c
	int skinColor;	//3760
	int hairColor;	//3764
	int hair;				//3768
	int eyeColor;		//376c
	int model;			//3770
	int suitColor;	//3774
	int tieColor;		//3778
	char unk32[14288 - 0x3778 - 4];

	const char* getClass() const
	{
		return "Human";
	}
	std::string __tostring() const;
	int getIndex() const;
	bool getIsActive() const
	{
		return active;
	}
	void setIsActive(bool b)
	{
		active = b;
	}
	sol::table getDataTable() const;
	bool getIsAlive() const
	{
		return oldHealth > 0;
	}
	void setIsAlive(bool b)
	{
		oldHealth = b ? 100 : 0;
	}
	bool getIsImmortal() const
	{
		return isImmortal;
	}
	void setIsImmortal(bool b)
	{
		isImmortal = b;
	}
	bool getIsOnGround() const
	{
		return isOnGround;
	}
	bool getIsStanding() const
	{
		return isStanding;
	}
	bool getIsBleeding() const
	{
		return isBleeding;
	}
	void setIsBleeding(bool b)
	{
		isBleeding = b;
	}
	Player* getPlayer() const;
	Vehicle* getVehicle() const;
	void setVehicle(Vehicle* vcl);
	Bone* getBone(unsigned int idx);
	RigidBody* getRigidBody(unsigned int idx) const;
	Item* getRightHandItem() const;
	Item* getLeftHandItem() const;
	Human* getRightHandGrab() const;
	void setRightHandGrab(Human* man);
	Human* getLeftHandGrab() const;
	void setLeftHandGrab(Human* man);

	void remove() const;
	void teleport(Vector* vec);
	void speak(const char* message, int distance) const;
	void arm(int weapon, int magCount) const;
	void setVelocity(Vector* vel) const;
	void addVelocity(Vector* vel) const;
	bool mountItem(Item* childItem, unsigned int slot) const;
	void applyDamage(int bone, int damage) const;
};

//4912 bytes (1330)
struct ItemType
{
	int price;
	float mass;					 //04
	int unk00; //08
	int isGun;					 //0c
	int messedUpAiming;	 //10
	//in ticks per bullet
	int fireRate;	 //14
	//?
	int bulletType;				 //18
		int unk0;							 //1c
		int unk1;							 //20
	float bulletVelocity;	 //24
	float bulletSpread;		 //28
	char name[64];				 //2c
		char unk2[0x78 - 0x2c - 64];
	int numHands;					//78
	Vector rightHandPos;	//7c
	Vector leftHandPos;		//88
		char unk3[4912 - 0x88 - 12];

	const char* getClass() const
	{
		return "ItemType";
	}
	std::string __tostring() const;
	int getIndex() const;
	char* getName()
	{
		return name;
	}
	void setName(const char* newName)
	{
		std::strncpy(name, newName, 63);
	}
	bool getIsGun() const
	{
		return isGun;
	}
	void setIsGun(bool b)
	{
		isGun = b;
	}
};

//6784 bytes (1A80)
struct Item
{
	int active;
	int physicsSim;			 //04
	int physicsSettled;	 //08
	//counts to 60 ticks before settling
	int physicsSettledTimer; //0c
	int isStatic;						 //10
	int type;						 //14
		int unk2;						 //18
	int despawnTime;		 //1c
	int grenadePrimerID; //20
	int parentHumanID;	 //24
	int parentItemID;		 //28
	int parentSlot;			 //2c
		char unk5[0x58 - 0x2c - 4];
	int bodyID;			//58
	Vector pos;			//5c
	Vector pos2;		//68
	Vector vel;			//74
	Vector vel2;		//80
	Vector vel3;		//8c
	Vector vel4;		//98
	RotMatrix rot;	//a4
		char unk6[0x144 - 0xa4 - 36];
	int bullets;	//144
		char unk7[0x368 - 0x144 - 4];
	unsigned int computerCurrentLine; //368
	unsigned int computerTopLine; //36c
	//-1 for no cursor
	int computerCursor; //370
	char computerLines[32][64]; //374
		char unk8[0xb74 - 0x374 - (64 * 32)];
	unsigned char computerLineColors[32][64]; //b74
		char unk9[0x1658 - 0xb74 - (64 * 32)];
	int computerTeam; //1658
		char unk10[6784 - 5724];

	const char* getClass() const
	{
		return "Item";
	}
	std::string __tostring() const;
	int getIndex() const;
	bool getIsActive() const
	{
		return active;
	}
	void setIsActive(bool b)
	{
		active = b;
	}
	sol::table getDataTable() const;
	bool getHasPhysics() const
	{
		return physicsSim;
	}
	void setHasPhysics(bool b)
	{
		physicsSim = b;
	}
	bool getPhysicsSettled() const
	{
		return physicsSettled;
	}
	void setPhysicsSettled(bool b)
	{
		physicsSettled = b;
	}
	bool getIsStatic() const
	{
		return isStatic;
	}
	void setIsStatic(bool b)
	{
		isStatic = b;
	}

	void remove() const;
	Player* getGrenadePrimer() const;
	void setGrenadePrimer(Player* player);
	Human* getParentHuman() const;
	Item* getParentItem() const;
	RigidBody* getRigidBody() const;
	bool mountItem(Item* childItem, unsigned int slot) const;
	bool unmount() const;
	void speak(const char* message, int distance) const;
	void explode() const;
	void setMemo(const char* memo) const;
	void computerTransmitLine(unsigned int line) const;
	void computerSetLine(unsigned int line, const char* newLine);
	void computerSetColor(unsigned int line, unsigned int column, unsigned char color);
};

//20572 bytes (505C)
struct Vehicle
{
	int active;
	unsigned int type;			//04
	int controllableState;	//08
	//default 100
	int health;							 //0c
		int unk1;								 //10
	int lastDriverPlayerID;	 //14
	unsigned int color;			 //18
	//-1 = won't despawn
	short despawnTime;	 //1c
	short spawnedState;	 //1e
	int isLocked;				 //20
		int unk3;						 //24
	int bodyID;					 //28
	Vector pos;					 //2c
	Vector pos2;				 //38
	RotMatrix rot;			 //44
	Vector vel;					 //68
		char unk5[0x27d8 - 0x68 - 12];
	int windowState0;	 //27d8
	int windowState1;	 //27dc
	int windowState2;	 //27e0
	int windowState3;	 //27e4
	int windowState4;	 //27e8
	int windowState5;	 //27ec
	int windowState6;	 //27f0
	int windowState7;	 //27f4
		char unk6[0x35dc - 0x27f4 - 4];
	float gearX;				 //35dc
	float steerControl;	 //35e0
	float gearY;				 //35e4
	float gasControl;		 //35e8
		char unk7[0x4ea0 - 0x35e8 - 4];
	int bladeBodyID;	//4ea0
		char unk8[20572 - 20132];

	const char* getClass() const
	{
		return "Vehicle";
	}
	std::string __tostring() const;
	int getIndex() const;
	bool getIsActive() const
	{
		return active;
	}
	void setIsActive(bool b)
	{
		active = b;
	}
	sol::table getDataTable() const;
	Player* getLastDriver() const;
	RigidBody* getRigidBody() const;

	void updateType() const;
	void updateDestruction(int updateType, int partID, Vector* pos, Vector* normal) const;
	void remove() const;
};

//92 bytes (5C)
struct Bullet
{
	unsigned int type;
	int time;				 //04
	int playerID;		 //08
	float unk0;			 //0c
	float unk1;			 //10
	Vector lastPos;	 //14
	Vector pos;			 //20
	Vector vel;			 //2c
	char unk2[92 - 56];

	const char* getClass() const
	{
		return "Bullet";
	}
	Player* getPlayer() const;
};

//176 bytes (B0)
struct RigidBody
{
	int active;
	/*
	0 = human bone
	1 = car body
	2 = wheel
	3 = item
	*/
	int type;					//04
	int settled;			//08
		int unk0;					//0c
		int unk01; //10
	float mass;				//14
	Vector pos;				//18
	Vector vel;				//24
	Vector startVel;	//? 30
	RotMatrix rot;		//3c
	RotMatrix rot2;		//60
		char unk3[176 - 132];

	const char* getClass() const
	{
		return "RigidBody";
	}
	std::string __tostring() const;
	int getIndex() const;
	bool getIsActive() const
	{
		return active;
	}
	void setIsActive(bool b)
	{
		active = b;
	}
	bool getIsSettled() const
	{
		return settled;
	}
	void setIsSettled(bool b)
	{
		settled = b;
	}
	Bond* bondTo(RigidBody* other, Vector* thisLocalPos, Vector* otherLocalPos) const;
	Bond* bondRotTo(RigidBody* other) const;
	Bond* bondToLevel(Vector* localPos, Vector* globalPos) const;
};

//244 bytes (F4)
struct Bond
{
	int active;
	/*
	* 4 = rigidbody_level
	* 7 = rigidbody_rigidbody
	* 8 = rigidbody_rot_rigidbody
	*/
	int type; //04
		int unk0; //08
	int despawnTime; //0c
		char unk1[0x2c - 0x0c - 4];
	// for level bonds
	Vector globalPos; //2c
	Vector localPos; //38
	// for non-level bonds
	Vector otherLocalPos; //44
		char unk2[0x98 - 0x44 - 12];
	int bodyID; //98
	int otherBodyID; //9C
		char unk3[244 - 160];
		
	const char* getClass() const
	{
		return "Bond";
	}
	std::string __tostring() const;
	int getIndex() const;
	bool getIsActive() const
	{
		return active;
	}
	void setIsActive(bool b)
	{
		active = b;
	}
	RigidBody* getBody() const;
	RigidBody* getOtherBody() const;
};