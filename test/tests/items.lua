assert(#items.getAll() == 0)
assert(items.getCount() == 0)
assert(#items == 0)
assert(items[0])

local item = assert(items.create(
	1,
	Vector(),
	RotMatrix(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	)
))
assert(item.isActive)
item:remove()

item = assert(items.create(
	1,
	Vector(),
	Vector(1, 0, 0),
	RotMatrix(
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	)
))
assert(item.isActive)
assert(item.rigidBody.vel:dist(Vector(1, 0, 0)) == 0)

item.despawnTime = 420
assert(item.despawnTime == 420)

do
	local magazine = assert(items.create(
		2,
		Vector(),
		RotMatrix(
			1, 0, 0,
			0, 1, 0,
			0, 0, 1
		)
	))

	assert(item:mountItem(magazine, 0))
	assert(magazine:unmount())

	magazine:remove()
end

assertAddsEvent(function ()
	item:speak('hello', 0)
end)

item:explode()

item:setMemo('memo')

item:computerSetLine(0, 'hello')
item:computerSetColor(0, 0, 0xFF)
item:computerTransmitLine(0)

item:remove()

assert(#items == 0)