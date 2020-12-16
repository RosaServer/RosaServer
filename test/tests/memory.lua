local base = memory.getBaseAddress()
assert(base > 0)

-- ELF magic number
assert(memory.readByte(base) == 0x7f)
assert(memory.readUByte(base) == 0x7f)
assert(memory.readShort(base) == 0x457f)
assert(memory.readUShort(base) == 0x457f)
assert(memory.readInt(base) == 0x464c457f)
assert(memory.readUInt(base) == 0x464c457f)
assert(memory.readBytes(base, 4) == '\x7fELF')

-- 64-bit
assert(memory.readByte(base + 4) == 2)
-- Little-endian
assert(memory.readByte(base + 5) == 1)
-- Version 1
assert(memory.readByte(base + 6) == 1)
-- Platform 0
assert(memory.readByte(base + 7) == 0)

assert(memory.readLong(base) == 0x00010102464c457f)
assert(memory.readULong(base) == 0x00010102464c457f)

assert(memory.getAddress(players[0]) < memory.getAddress(players[1]))

local sizeOfPlayer = memory.getAddress(players[1]) - memory.getAddress(players[0])
assert(memory.getAddress(players[2]) == memory.getAddress(players[1]) + sizeOfPlayer)

local item = items[0]
assert(not item.isActive)
local address = memory.getAddress(items[0])

for _, func in ipairs({
	'writeByte',
	'writeUByte',
	'writeShort',
	'writeUShort',
	'writeInt',
	'writeUInt',
	'writeLong',
	'writeULong',
	'writeFloat',
}) do
	memory[func](address, 1)
	assert(item.isActive)
	item.isActive = false
end

assert(not item.hasPhysics)
memory.writeDouble(address, 1)
assert(not item.isActive)
assert(item.hasPhysics)

memory.writeBytes(address, '\1\0\0\0\0\0\0\0')
assert(item.isActive)
assert(not item.hasPhysics)
memory.writeBytes(address, '\1\0\0\0\1\0\0\0')
assert(item.hasPhysics)

item.isActive = false
item.hasPhysics = false