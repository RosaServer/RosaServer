local child = ChildProcess.new('tests/worker.worker.lua')

assert(child:isRunning())
assert(not child:receiveMessage())
child:sendMessage('hi')

nextTick(function ()
	local message = assert(child:receiveMessage())
	assert(message == 'hello')

	assert(not child:isRunning())
	assert(child:getExitCode() == 0)
end, 10)

child:setCPULimit(2, 4)
child:setMemoryLimit(10000000, 20000000)
child:setFileSizeLimit(10000000, 20000000)

assert(child:getPriority() == 0)
child:setPriority(5)
assert(child:getPriority() == 5)