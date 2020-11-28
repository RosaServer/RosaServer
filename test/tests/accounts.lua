os.remove('server.srk')
accounts.save()
assert(os.remove('server.srk'))

assert(#accounts.getAll() == 0)
assert(accounts.getCount() == 0)
assert(#accounts == 0)

assert(not accounts.getByPhone(0))