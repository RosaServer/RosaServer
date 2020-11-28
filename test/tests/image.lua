local image = Image.new()

image:loadFromFile('lena.jpg')

assert(image.width == 256)
assert(image.height == 256)
assert(image.numChannels == 3)

local red, green, blue = image:getRGB(132, 132)
assert(red == 86)
assert(green == 7)
assert(blue == 52)

image:free()