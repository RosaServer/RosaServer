assertAddsEvent(function ()
	event.sound(0, Vector(), 1.0, 1.0)
end)

assertAddsEvent(function ()
	event.sound(0, Vector())
end)

assertAddsEvent(function ()
	event.explosion(Vector())
end)

assertAddsEvent(function ()
	event.bulletHit(0, Vector(), Vector(1, 0, 0))
end)