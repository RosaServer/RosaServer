local entries = os.listDirectory('tests')
assert(#entries > 0)

local thisFile
for _, entry in ipairs(entries) do
	if entry.stem == 'os' then
		thisFile = entry
		break
	end
end

assert(thisFile)
assert(not thisFile.isDirectory)
assert(thisFile.name == 'os.lua')
assert(thisFile.extension == '.lua')

os.execute('rm -rf ./hoodieStrings')
assert(os.createDirectory('hoodieStrings/subDirectory/yetAnother'))
assert(os.execute('rm -rf ./hoodieStrings'))