local groundLevel = 23.875
local airLevel = groundLevel * 2

do
	local ray = physics.lineIntersectLevel(
		Vector(0, airLevel, 0),
		Vector(0, 0, 0)
	)

	assert(ray.hit)
	assert(ray.pos:dist(Vector(0, groundLevel, 0)) == 0)
	assert(ray.normal:dist(Vector(0, 1, 0)) == 0)
	assert(ray.fraction == 0.5)
end

nextTick(function ()
	do
		local bot = players.createBot()
		local man = assert(humans.create(
			Vector(0, airLevel, 0),
			RotMatrix(
				1, 0, 0,
				0, 1, 0,
				0, 0, 1
			),
			bot
		))

		nextTick(function ()
			local ray = physics.lineIntersectHuman(
				man,
				Vector(-10, airLevel, 0),
				Vector(10, airLevel, 0)
			)

			assert(ray.hit)
			assert(ray.pos:dist(Vector(0, airLevel, 0)) < 1)
			assert(ray.fraction <= 0.5)
			assert(ray.bone == 5)

			man:remove()
			bot:remove()
		end)
	end

	do
		local vehicle = assert(vehicles.create(
			0,
			Vector(0, airLevel, 0),
			RotMatrix(
				1, 0, 0,
				0, 1, 0,
				0, 0, 1
			),
			0
		))

		nextTick(function ()
			local ray = physics.lineIntersectVehicle(
				vehicle,
				Vector(0, airLevel + 10, 0),
				Vector(0, airLevel - 10, 0)
			)

			assert(ray.hit)
			assert(ray.pos.x == 0)
			assert(ray.pos.z == 0)
			assert(ray.normal:dist(Vector(0, 1, 0)) < 0.01)

			vehicle:remove()
		end)
	end
end)

local outPosition = Vector()
local fraction = assert(physics.lineIntersectTriangle(
	outPosition,
	Vector(0, 1, 0),

	Vector(0, airLevel + 10, 0),
	Vector(0, airLevel - 10, 0),

	Vector(0, airLevel, -1),
	Vector(1, airLevel, 1),
	Vector(-1, airLevel, 1)
))

assert(fraction == 0.5)
assert(outPosition:dist(Vector(0, airLevel, 0)) == 0)

assert(not physics.lineIntersectTriangle(
	outPosition,
	Vector(0, 1, 0),

	Vector(0, airLevel + 10, 0),
	Vector(0, airLevel - 10, 0),

	Vector(100, airLevel, -1),
	Vector(101, airLevel, 1),
	Vector(99, airLevel, 1)
))

physics.garbageCollectBullets()
assert(bullets.getCount() == 0)