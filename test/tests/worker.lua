local worker = assert(Worker.new('tests/worker.worker.lua'))

assert(not worker:receiveMessage())
worker:sendMessage('hi')

local maxTicks = 10
local ticks = 0

local function try ()
	ticks = ticks + 1

	local message = worker:receiveMessage()
	if message then
		assert(message == 'hello')
	else
		assert(ticks < maxTicks)
		nextTick(try)
	end
end

nextTick(try)