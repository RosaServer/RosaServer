local message = 'Test message'

assertAddsEvent(function ()
	assert(chat.announce(message))
end)

assertAddsEvent(function ()
	assert(chat.tellAdmins(message))
end)