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