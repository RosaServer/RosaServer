assert(#streets.getAll() == 1)
assert(streets.getCount() == 1)
assert(#streets == 1)
assert(streets[0])

local street = streets[0]
street.numTraffic = 420
assert(street.numTraffic == 420)

local lane = assert(street:getLane(0))
assert(lane.direction == 0)
lane.direction = 1
assert(lane.direction == 1)

assert(#intersections.getAll() == 2)
assert(intersections.getCount() == 2)
assert(#intersections == 2)
assert(intersections[0])

assert(intersections[0].streetWest == street)
assert(intersections[1].streetEast == street)