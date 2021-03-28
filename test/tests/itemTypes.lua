local expectedNum = 46

assert(#itemTypes.getAll() == expectedNum)
assert(itemTypes.getCount() == expectedNum)
assert(#itemTypes == expectedNum)
assert(itemTypes[0])

itemTypes[0].price = 420
assert(itemTypes[0].price == 420)