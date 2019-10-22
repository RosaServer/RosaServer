#pragma once

#define MAXNUMOFPLAYERS 256
#define MAXNUMOFHUMANS 256
#define MAXNUMOFITEMTYPES 36
#define MAXNUMOFITEMS 1024
#define MAXNUMOFVEHICLES 512
#define MAXNUMOFRIGIDBODIES 2048

/*
	Event types:
	0x2		2	message
	0x3		3	updateobject (vehicle)
	0x4		4	something else object
	0x5		5	updateitem
	0x7		7	updateplayer
	0x8		8	updateplayer_finance
	0x9		9	sound
	0xA		10	updatedoor
	0x11	17	updatehuman
	0x14	20	explosion
*/

//135564 bytes (2118C)
struct Connection {
	uint8_t address[4];
	unsigned int port; //04
		int unk0; //08
	BOOL adminVisible; //0c
	int playerID; //10
		int unk1; //14
	int bandwidth; //18
	int timeoutTime; //1c
		char unk2[135564 - 32];

	std::string getAddress();
	bool getAdminVisible() const {
		return adminVisible;
	}
	void setAdminVisible(bool b) {
		adminVisible = b;
	}
};

//104 bytes (68)
struct Account {
	int subRosaID;
	int phoneNumber; //04
	long long steamID; //08
	char name[32]; //10
	int unk0; //30
	int money; //34
	char unk1[0x54 - 0x34 - 4];
	int banTime; //54
	char unk2[104 - 88];

	char* getName() {
		return name;
	}
	std::string getSteamID() {
		return std::to_string(steamID);
	}
};

struct Vector {
	float x, y, z;
	void add(Vector* other);
	void mult(float scalar);
	void set(Vector* other);
	Vector clone() const;
	float dist(Vector* other) const;
	float distSquare(Vector* other) const;
};

struct RotMatrix {
	float x1, y1, z1;
	float x2, y2, z2;
	float x3, y3, z3;
	void set(RotMatrix* other);
	RotMatrix clone() const;
};

struct SpecCam {
	Vector position;
	float yaw, pitch;
	float smoothZoom, actualZoom;
};

struct RayCastResult {
	Vector pos;
	Vector normal; //0c
	float fraction; //18
	float unk0; //1c
	int unk1; //20
	int unk2; //24
	int unk3; //28
	int unk4; //2c
	int unk5; //30
	int humanBone; //34
	int unk6; //38
	int unk7; //3c
	int unk8; //40
	int unk9; //44
	int unk10; //48
	int unk11; //4c
	int unk12; //50
	int unk13; //54
	int blockX; //58
	int blockY; //5c
	int blockZ; //60
	int unk17; //64
	int unk18; //68
	int matMaybe; //6c
	int unk20; //70
	int unk21; //74
	int unk22; //78
	int unk23; //7c
	int unk24; //80
};

// Forward decl
struct Human;
struct Vehicle;
struct Item;
struct RigidBody;

//14364 bytes (381C)
struct Player {
	BOOL active;
	char name[32]; //04
	int unk0; //24
	int unk1; //28
	unsigned int subRosaID; //2c
	unsigned int phoneNumber; //30
	BOOL isAdmin; //34
	unsigned int adminAttempts; //38
	unsigned int accountID; //3C
	char unk2[0x48 - 0x3C - 4];
	BOOL isReady; //48
	int money; //4C
	char unk3[0x74 - 0x4C - 4];
	unsigned int team; //74
	unsigned int teamSwitchTimer; //78
	int stocks; //7c
	int unk4[2];
	int humanID; //88
	char unk5[0x150 - 0x88 - 4];
	int menuTab; //150
	char unk6[0x2d04 - 0x150 - 4];
	BOOL isBot; //2d04
	char unk7[0x3794 - 0x2d04 - 4];
	int gender; //3794
	int skinColor; //3798
	int hairColor; //379c
	int hair; //37a0
	int eyeColor; //37a4
	//currently doesn't work client-side
	int shirtColor; //37a8
	int suitColor; //37ac
	//0 = no tie
	int tieColor; //37b0
	int unk8; //37b4
	int head; //37b8
	int necklace; //37bc
	char unk9[14364 - 14272];

	int getIndex() const;
	bool getIsActive() const {
		return active;
	}
	void setIsActive(bool b) {
		active = b;
	}
	char* getName() {
		return name;
	}
	void setName(const char* newName) {
		strncpy(name, newName, 31);
	}
	bool getIsAdmin() const {
		return isAdmin;
	}
	void setIsAdmin(bool b) {
		isAdmin = b;
	}
	bool getIsReady() const {
		return isReady;
	}
	void setIsReady(bool b) {
		isReady = b;
	}
	bool getIsBot() const {
		return isBot;
	}
	void setIsBot(bool b) {
		isBot = b;
	}
	Human* getHuman();
	Connection* getConnection();

	void update() const;
	void updateFinance() const;
	void remove() const;
};

//288 bytes (120)
struct Bone {
	int bodyID;
	Vector pos;
	Vector pos2;
	Vector vel;
	Vector vel2;
	RotMatrix rot;
	char unk[288 - 88];
};

//13632 bytes (3540)
struct Human {
	BOOL active;
	unsigned int unk00; //04
	int playerID; //08
	int unk0; //0c
	int unk1; //10
	int unk2; //14
	int unk3; //18
	int unk4; //1c
	int vehicleID; //20
	int vehicleSeat; //24
	int lastVehicleID; //28
	int lastVehicleCooldown; //2c
	//counts down after death
	unsigned int despawnTime; //30
	unsigned int oldHealth; //34
	BOOL isImmortal; //38
	int unk10; //3c
	int unk11; //40
	int unk12; //44
	unsigned int spawnProtection; //38
	BOOL isOnGround; //4c
	/*
	0=normal
	1=jumping/falling
	2=sliding
	*/
	int movementState; //50
	int zoomLevel; //54
	int unk14; //58
	int unk15; //5c
	int unk16; //60
	int unk17; //64
	int unk18; //68
	//max 60
	int damage; //6c
	BOOL isStanding; //70
	Vector pos; //74
	Vector pos2; //80
	float viewYaw; //8c
	float viewPitch; //90
	char unk19[0x120 - 0x90 - 4];
	float strafeInput; //120
	float unk20;
	float walkInput; //128
	char unk21[0x1b4 - 0x128 - 4];
	/*
	mouse1 = 1		1 << 0
	mouse2 = 2		1 << 1
	space = 4		1 << 2
	ctrl = 8		1 << 3
	shift = 16		1 << 4

	1 = 32			1 << 5
	1 = 64			1 << 6
	2 = 128			1 << 7
	3 = 256			1 << 8
	4 = 512			1 << 9
	5 = 1024		1 << 10
	e = 2048		1 << 11
	r = 4096		1 << 12
	f = 8192		1 << 13
	
	del = 262144	1 << 18
	z = 524288		1 << 19
	*/
	unsigned int inputFlags; //1b4
	unsigned int lastInputFlags; //1b8
	char unk22[0x1c0 - 0x1b8 - 4];
	Bone bones[16]; //1c0
	char unk23[0x3038 - (0x1c0 + 4608)];
	int rightHandOccupied; //3038
	int rightHandItemID; //303c
	char unk24[0x3060 - 0x303c - 4];
	int leftHandOccupied; //3060
	int leftHandItemID; //3064
	char unk25[0x32a0 - 0x3064 - 4];
	int health; //32a0
	int bloodLevel; //32a4
	BOOL isBleeding; //32a8
	int chestHP; //32ac
	int unk26; //32b0
	int headHP; //32b4
	int unk27; //32b8
	int leftArmHP; //32bc
	int unk28; //32c0
	int rightArmHP; //32c4
	int unk29; //32c8
	int leftLegHP; //32cc
	int unk30; //32d0
	int rightLegHP; //32d4

	char unk31[0x34d0 - 0x32d4 - 4];
	//000000000400000002000000070000000400000004000000
	unsigned int gender; //34d0
	unsigned int head; //34d4
	unsigned int skinColor; //34d8
	unsigned int hairColor; //34dc
	unsigned int hair; //34e0
	unsigned int eyeColor; //34e4
	char unk32[13632 - 0x34e4 - 4];

	int getIndex() const;
	bool getIsActive() const {
		return active;
	}
	void setIsActive(bool b) {
		active = b;
	}
	bool getIsAlive() const {
		return oldHealth != 0;
	}
	void setIsAlive(bool b) {
		oldHealth = b ? 100 : 0;
	}
	bool getIsImmortal() const {
		return isImmortal;
	}
	void setIsImmortal(bool b) {
		isImmortal = b;
	}
	bool getIsOnGround() const {
		return isOnGround;
	}
	bool getIsStanding() const {
		return isStanding;
	}
	bool getIsBleeding() const {
		return isBleeding;
	}
	void setIsBleeding(bool b) {
		isBleeding = b;
	}
	Player* getPlayer() const;
	Vehicle* getVehicle() const;
	void setVehicle(Vehicle* vcl);
	Bone* getBone(unsigned int idx);
	RigidBody* getRigidBody(unsigned int idx) const;

	void update() const;
	void remove() const;
	Vector getPos() const;
	void setPos(Vector* vec);
	void speak(const char* message, int distance) const;
	void arm(int weapon, int magCount) const;
	void setVelocity(Vector* vel) const;
	void addVelocity(Vector* vel) const;
	bool mountItem(Item* childItem, unsigned int slot) const;
	void applyDamage(int bone, int damage) const;
};

//3464 bytes (d88)
struct ItemType {
	int price;
	float mass; //04
	BOOL isGun; //08
	BOOL fuckedUpAiming; //0c
	//in ticks per bullet
	int fireRate; //10
	//?
	int bulletType; //14
	int unk0; //18
	int unk1; //1c
	float bulletVelocity; //20
	float bulletSpread; //24
	char name[64]; //28
	char unk2[3464 - 104];

	int getIndex() const;
	char* getName() {
		return name;
	}
	void setName(const char* newName) {
		strncpy(name, newName, 63);
	}
	bool getIsGun() const {
		return isGun;
	}
	void setIsGun(bool b) {
		isGun = b;
	}
};

//532 bytes (214)
struct Item {
	BOOL active;
	BOOL physicsSim; //04
	BOOL physicsSettled; //08
	int unk1; //0c
	int type; //10
	int unk2; //14
	int unk3; //18
	int unk4; //1c
	int parentHumanID; //20
	int parentItemID; //24
	int parentSlot; //28
	char unk5[0x50 - 0x28 - 4];
	int bodyID; //50
	Vector pos; //54
	Vector pos2; //60
	Vector vel; //6c
	Vector vel2; //78
	Vector vel3; //84
	Vector vel4; //90
	RotMatrix rot; //9c
	char unk6[0xec - 0x9c - 36];
	int bullets; //ec
	char unk7[532 - 240];

	int getIndex() const;
	bool getIsActive() const {
		return active;
	}
	void setIsActive(bool b) {
		active = b;
	}
	bool getHasPhysics() const {
		return physicsSim;
	}
	void setHasPhysics(bool b) {
		physicsSim = b;
	}
	bool getPhysicsSettled() const {
		return physicsSettled;
	}
	void setPhysicsSettled(bool b) {
		physicsSettled = b;
	}

	void update() const;
	void remove() const;
	Human* getParentHuman() const;
	Item* getParentItem() const;
	RigidBody* getRigidBody() const;
	bool mountItem(Item* childItem, unsigned int slot) const;
	void speak(const char* message, int distance) const;
	void explode() const;
};

//20700 bytes (50DC)
struct Vehicle {
	BOOL active;
	unsigned int type; //04
	int controllableState; //08
	//default 100
	int health; //0c
		int unk1; //10
	int lastDriverPlayerID; //14
	unsigned int color; //18
	//-1 = won't despawn
	short despawnTime; //1c
	short spawnedState; //1e
	BOOL isLocked; //20
	int bodyID; //24
		int unk3[34]; //28
	Vector pos; //b0
	Vector pos2; //bc
	RotMatrix rot; //c8
	Vector vel; //ec
		char unk4[0x110 - 0xec - 12];
	Vector pos3; //110
	Vector pos4; //11c
		char unk5[0x2844 - 0x11c - 12];
	BOOL windowState0; //2844
	BOOL windowState1; //2848
	BOOL windowState2; //284c
	BOOL windowState3; //2850
	BOOL windowState4; //2854
	BOOL windowState5; //2858
	BOOL windowState6; //285c
	BOOL windowState7; //2860
		char unk5_1[0x3648 - 0x2860 - 4];
	float gearX; //3648
	float steerControl; //364c
	float gearY; //3650
	float gasControl; //3654
		char unk6[0x3988 - 0x3654 - 4];
	int numWheels; //3988
	int wheelBodyID0; //398c
		char unk7[0x3a38 - 0x398c - 4];
	int wheelBodyID1; //3a38
		char unk8[0x3ae4 - 0x3a38 - 4];
	int wheelBodyID2; //3ae4
		char unk9[0x3b90 - 0x3ae4 - 4];
	int wheelBodyID3; //3b90
		char unk10[0x4f0c - 0x3b90 - 4];
	int bladeBodyID; //4f0c
		char unk11[20700 - 20240];

	int getIndex() const;
	bool getIsActive() const {
		return active;
	}
	void setIsActive(bool b) {
		active = b;
	}
	RigidBody* getRigidBody() const;

	void updateType() const;
	void updateDestruction(int updateType, int partID, Vector* pos, Vector* normal) const;
	void remove() const;
};

//92 bytes (5C)
struct Bullet {
	unsigned int type;
	int time; //04
	int playerID; //08
	float unk0; //0c
	float unk1; //10
	Vector lastPos; //14
	Vector pos; //20
	Vector vel; //2c
	char unk2[92 - 56];

	Player* getPlayer() const;
};

//172 bytes (AC)
struct RigidBody {
	BOOL active;
	/*
	0 = human bone
	1 = car body
	2 = wheel
	3 = item
	*/
	int type; //04
	BOOL settled; //08
	int unk0; //0c
	float mass; //10
	Vector pos; //14
	Vector vel; //20
	Vector startVel; //? 2C
	RotMatrix rot; //38
	RotMatrix rot2; //5c
	char unk3[172 - 128];

	int getIndex() const;
	bool getIsActive() const {
		return active;
	}
	void setIsActive(bool b) {
		active = b;
	}
	bool getIsSettled() const {
		return settled;
	}
	void setIsSettled(bool b) {
		settled = b;
	}
};