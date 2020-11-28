assert(#players.getAll() == 0)
assert(players.getCount() == 0)
assert(#players == 0)
assert(players[0])

local testPhone = 2561234

local bot = assert(players.createBot())
assert(bot.isActive)

bot.phoneNumber = testPhone
assert(bot.phoneNumber == testPhone)

assert(players[0] == bot)
assert(players.getByPhone(testPhone) == bot)
assert(#players.getNonBots() == 0)
assert(players.getCount() == 1)
assert(#players == 1)

do
	local action = assert(bot:getAction(0))
	action.type = 3
	assert(action.type == 3)
end

do
	local button = assert(bot:getMenuButton(0))
	button.text = 'Hi'
	assert(button.text == 'Hi')
end

assertAddsEvent(function ()
	bot:update()
end)

assertAddsEvent(function ()
	bot:updateFinance()
end)

assertAddsEvent(function ()
	bot:sendMessage('hello')
end)

assertAddsEvent(function ()
	bot:remove()
end)

assert(#players == 0)