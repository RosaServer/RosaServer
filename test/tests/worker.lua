local worker = assert(Worker.new('tests/worker.worker.lua'))

assert(not worker:receiveMessage())
worker:sendMessage('hi')

nextTick(function ()
	local message = assert(worker:receiveMessage())
	assert(message == 'hello')
end, 10)