assertAddsEvent(function ()
	assert(events.createBullet(0, Vector(), Vector(1, 2, 3), items[0]))
end)

assertAddsEvent(function ()
	assert(events.createBulletHit(0, Vector(), Vector(1, 0, 0)))
end)

assertAddsEvent(function ()
	assert(events.createMessage(0, 'Test', -1, 0))
end)

assertAddsEvent(function ()
	assert(events.createSound(0, Vector(), 1.0, 1.0))
end)

assertAddsEvent(function ()
	assert(events.createSound(0, Vector()))
end)

assertAddsEvent(function ()
	assert(events.createExplosion(Vector()))
end)