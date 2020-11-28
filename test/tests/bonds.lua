assert(#bonds.getAll() == 0)
assert(bonds.getCount() == 0)
assert(#bonds == 0)
assert(bonds[0])

local bond = bonds[0]

bond.despawnTime = 420
assert(bond.despawnTime == 420)

assert(not bond.isActive)

local bot = assert(players.createBot())
local man = assert(humans.create(
	Vector(30, 30, 30),
	RotMatrix(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	),
	bot
))

assert(bond.isActive)

man:remove()
bot:remove()

assert(not bond.isActive)