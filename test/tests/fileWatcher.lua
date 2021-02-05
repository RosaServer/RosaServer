local watcher = FileWatcher.new()
local fileName = 'textFile.txt'

os.remove(fileName)

watcher:addWatch('.', bit32.bor(FILE_WATCH_CREATE, FILE_WATCH_MODIFY, FILE_WATCH_DELETE))

assert(not watcher:receiveEvent())

do
	local file = assert(io.open(fileName, 'w'))
	file:write('hello')
	file:close()
end

do
	local event = assert(watcher:receiveEvent())
	assert(event.descriptor == '.')
	assert(event.name == fileName)
	assert(event.mask == FILE_WATCH_CREATE)
end

do
	local file = assert(io.open(fileName, 'w'))
	file:write('hello again')
	file:close()
end

do
	local event = assert(watcher:receiveEvent())
	assert(event.descriptor == '.')
	assert(event.name == fileName)
	assert(event.mask == FILE_WATCH_MODIFY)
end

assert(os.remove(fileName))

do
	local event = assert(watcher:receiveEvent())
	assert(event.descriptor == '.')
	assert(event.name == fileName)
	assert(event.mask == FILE_WATCH_DELETE)
end

assert(watcher:removeWatch('.'))