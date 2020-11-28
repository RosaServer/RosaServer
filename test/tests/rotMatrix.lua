local rotMatrix = RotMatrix(
	1, 0, 0,
	0, 1, 0,
	0, 0, 1
)

assert(rotMatrix.x1 == 1)
assert(rotMatrix.y1 == 0)
assert(rotMatrix.z1 == 0)

assert(rotMatrix.x2 == 0)
assert(rotMatrix.y2 == 1)
assert(rotMatrix.z2 == 0)

assert(rotMatrix.x3 == 0)
assert(rotMatrix.y3 == 0)
assert(rotMatrix.z3 == 1)

rotMatrix.x1 = -1
rotMatrix.y2 = -1
rotMatrix.z3 = -1

rotMatrix:set(RotMatrix(
	1, 0, 0,
	0, 1, 0,
	0, 0, 1
))

assert(rotMatrix.x1 == 1)
assert(rotMatrix.y2 == 1)
assert(rotMatrix.z3 == 1)

local clone = rotMatrix:clone()
clone.x1 = 0
assert(rotMatrix.x1 == 1)