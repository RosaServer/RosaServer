local function log (...)
	local prefix = '\27[34;1m[Test]\27[0m '
	print(prefix .. string.format(...))
end

local function runTests ()
	require('tests.accounts')
	require('tests.bonds')
	require('tests.bullets')
	require('tests.chat')
	require('tests.event')
	require('tests.http')
	require('tests.humans')
	require('tests.image')
	require('tests.items')
	require('tests.itemTypes')
	require('tests.memory')
	require('tests.os')
	require('tests.physics')
	require('tests.players')
	require('tests.rigidBodies')
	require('tests.rotMatrix')
	require('tests.server')
	require('tests.streets')
	require('tests.vector')
	require('tests.vehicles')
	require('tests.worker')
end

local function testsPassed ()
	log('\27[32;1m✔\27[0m All tests passed')
	os.exit(0)
end

local function protectedFailCall (func, ...)
	local success, result = pcall(func, ...)
	if not success then
		log('\27[31;1m✘\27[0m Test failed at %s', tostring(result))
		os.exit(1)
	end
end

local handlers = {}

local tick = 0

function hook.run (event, ...)
	if event == 'Logic' then
		tick = tick + 1

		log('Tick %i...', tick, maxTicks)

		if tick == 1 then
			protectedFailCall(runTests)
		else
			for i = #handlers, 1, -1 do
				local handler = handlers[i]
				handler.ticksToWait = handler.ticksToWait - 1
				if handler.ticksToWait <= 0 then
					table.remove(handlers, i)
					protectedFailCall(handler.func)
				end
			end

			if #handlers == 0 then
				testsPassed()
			end
		end
	end
end

function nextTick (func, ticksToWait)
	ticksToWait = ticksToWait or 1
	table.insert(handlers, {
		func = func,
		ticksToWait = ticksToWait
	})
end

function assertAddsEvent (func, message)
	local numEvents = server.numEvents
	func()
	if server.numEvents ~= numEvents + 1 then
		error(message or 'event assertion failed!', 2)
	end
end