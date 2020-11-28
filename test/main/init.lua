local function runTests ()
	assert(server.name == 'Test')
end

function hook.run (event, ...)
	if event == 'Logic' then
		local success, result = pcall(runTests)
		if not success then
			print(result)
			os.exit(1)
		else
			os.exit(0)
		end
	end
end