local function log (...)
	local prefix = '\27[34;1m[Test]\27[0m '
	print(prefix .. string.format(...))
end

local function runTests ()
	require('tests.vector')
	require('tests.rotMatrix')
	require('tests.event')
	require('tests.physics')
	require('tests.chat')
	require('tests.accounts')
	require('tests.players')
	require('tests.humans')
	require('tests.itemTypes')
	require('tests.items')
	require('tests.vehicles')
	require('tests.bullets')
	require('tests.rigidBodies')
	require('tests.bonds')
	require('tests.streets')
	require('tests.os')
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
			local handlersThisTick = handlers
			handlers = {}

			for _, func in ipairs(handlersThisTick) do
				protectedFailCall(func)
			end

			if #handlers == 0 then
				testsPassed()
			end
		end
	end
end

function nextTick (func)
	table.insert(handlers, func)
end

function assertAddsEvent (func, message)
	local numEvents = server.numEvents
	func()
	if server.numEvents ~= numEvents + 1 then
		error(message or 'event assertion failed!', 2)
	end
end