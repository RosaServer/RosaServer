local vector = Vector(1, 2, 3.5)
assert(vector.x == 1)
assert(vector.y == 2)
assert(vector.z == 3.5)

vector:add(Vector(0, 0, -0.5))
assert(vector.z == 3)

vector:mult(2)
assert(vector.x == 2)
assert(vector.y == 4)
assert(vector.z == 6)

vector:set(Vector(1, 2, 3))
assert(vector.x == 1)
assert(vector.y == 2)
assert(vector.z == 3)

local clone = vector:clone()
clone.x = 3
assert(vector.x == 1)

assert(vector:dist(clone) == 2)
assert(vector:distSquare(clone) == 2*2)

local sum = vector + Vector(3, 2, 1)
assert(sum:dist(Vector(4, 4, 4)) == 0)

local difference = vector - Vector(1, 1, 1)
assert(difference:dist(Vector(0, 1, 2)) == 0)

local product = vector * 2
assert(product:dist(Vector(2, 4, 6)) == 0)

local quotient = vector / 2
assert(quotient:dist(Vector(0.5, 1, 1.5)) == 0)

local negated = -vector
assert(negated:dist(Vector(-1, -2, -3)) == 0)

local ninetyDegreesClockwise = RotMatrix(
	0, 0, 1,
	0, 1, 0,
	-1, 0, 0
)
local rotated = vector * ninetyDegreesClockwise
assert(rotated:dist(Vector(3, 2, -1)) == 0)