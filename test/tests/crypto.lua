local testString = 'Good morning Dr. Chandra. This is Hal. I am ready for my first lesson.'

assert(crypto.sha256('') == 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855')
assert(crypto.sha256(testString) == '912ec5ff4fa90ca4b3fe0b3a65ea46d0c4a9e3c3b16171bd7380ce74424581f3')