while true do
	if receiveMessage() == 'hi' then
		sendMessage('hello')
		break
	end
	sleep(8)
end