local testString = ('Good morning Dr. Chandra. This is Hal. I am ready for my first lesson.'):rep(8)

local compressed = zlib.compress(testString)
assert(#compressed < #testString)

local uncompressed = zlib.uncompress(compressed, #testString)
assert(uncompressed == testString)