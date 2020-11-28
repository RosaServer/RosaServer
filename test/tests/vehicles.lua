assert(#vehicles.getAll() == 0)
assert(vehicles.getCount() == 0)
assert(#vehicles == 0)
assert(vehicles[0])

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
assert(vehicle.isActive)
assert(vehicle.color == 4)

vehicle.color = 2
assert(vehicle.color == 2)

assertAddsEvent(function ()
	vehicle:updateType()
end)

assertAddsEvent(function ()
	vehicle:updateDestruction(
		0,
		0,
		vehicle.pos,
		Vector(0, 1, 0)
	)
end)

assertAddsEvent(function ()
	vehicle:remove()
end)

assert(#vehicles == 0)