local message = 'Test message'

assertAddsEvent(function ()
	chat.announce(message)
end)

assertAddsEvent(function ()
	chat.tellAdmins(message)
end)

assertAddsEvent(function ()
	chat.addRaw(0, message, -1, 0)
end)