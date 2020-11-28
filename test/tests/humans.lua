assert(#humans.getAll() == 0)
assert(humans.getCount() == 0)
assert(#humans == 0)
assert(humans[0])

local man = humans.create(
	Vector(),
	RotMatrix(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	),
	players[0]
)
assert(man.isActive)

man.leftArmHP = 24
assert(man.leftArmHP == 24)

assert(humans[0] == man)
assert(humans.getCount() == 1)
assert(#humans == 1)

man:teleport(Vector(0, 30, 0))

assertAddsEvent(function ()
	man:speak('hello', 0)
end)

nextTick(function ()
	local bot = assert(players.createBot())

	local man = humans.create(
		Vector(100, 50, 100),
		RotMatrix(
			1, 0, 0,
			0, 1, 0,
			0, 0, 1
		),
		bot
	)

	man:arm(1, 5)

	man:remove()
	bot:remove()
end)

do
	local bone = assert(man:getBone(0))
	assert(bone.pos)
end

do
	local body = assert(man:getRigidBody(0))
	assert(body.isActive)
end

man:setVelocity(Vector(1, 2, 3))
assert(man:getRigidBody(0).vel:dist(Vector(1, 2, 3)) == 0)

man:addVelocity(Vector(2, 1, 0))
assert(man:getRigidBody(0).vel:dist(Vector(3, 3, 3)) == 0)

do
	local item = items.create(
		0,
		Vector(),
		RotMatrix(
			1, 0, 0,
			0, 1, 0,
			0, 0, 1
		)
	)

	assert(man:mountItem(item, 0))

	item:remove()
end

man:applyDamage(0, 10)

man:remove()

assert(#humans == 0)