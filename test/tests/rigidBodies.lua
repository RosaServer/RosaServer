assert(#rigidBodies.getAll() == 0)
assert(rigidBodies.getCount() == 0)
assert(#rigidBodies == 0)
assert(rigidBodies[0])

local body = rigidBodies[0]

assert(not body.isActive)

local vehicle = assert(vehicles.create(
	0,
	Vector(100, 50, 100),
	RotMatrix(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	),
	4
))

assert(body.isActive)

body.mass = 420
assert(body.mass == 420)

assert(body:bondTo(rigidBodies[1], Vector(), Vector())).isActive = false

assert(body:bondRotTo(rigidBodies[1])).isActive = false

assert(body:bondToLevel(Vector(), Vector())).isActive = false

vehicle:remove()

assert(not body.isActive)