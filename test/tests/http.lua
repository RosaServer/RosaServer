local res = assert(http.getSync('https://github.com', '/robots.txt', {}))

assert(res.status >= 200 and res.status <= 299)
assert(res.body:find('Disallow'))

local foundContentType = false
for k, v in pairs(res.headers) do
	if k:lower() == 'content-type' then
		foundContentType = true
		assert(v == 'text/plain')
		break
	end
end
assert(foundContentType)