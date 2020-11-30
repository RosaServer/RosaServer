assert(server.class == 'Server')
assert(server.name == 'Test')
assert(server.port == 1)
assert(server.type == TYPE_WORLD)
assert(server.loadedLevel == 'test2')
assert(server.gravity == server.defaultGravity)
assert(server.sunTime >= 8 * 60 * 60 * server.TPS)
assert(server.versionMajor >= 37)

server:reset()

server:setConsoleTitle('Testing!')