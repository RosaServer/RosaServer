local db = SQLite.new(':memory:')

do
	local _, err = db:query('hello')
	assert(err == 'near "hello": syntax error')
end

do
	local _, err = db:query('create table people (age integer, height real, name text, dna blob);')
	assert(not err, err)
end

do
	local _, err = db:query(
		'insert into people values (?, ?, ?, ?), (?, ?, ?, ?);',
		50, 180, 'John Smith', 'ACGT\0ACGT\1',
		25, 190.5, nil, nil
	)
	assert(not err, err)
end

do
	local rows = assert(db:query('select age, height, name, dna from people where age < 30 and name is not null;'))
	assert(#rows == 0)
end

do
	local rows = assert(db:query('select age, height, name, dna from people where age >= 30;'))
	assert(#rows == 1)

	local row = rows[1]
	assert(row[1] == 50)
	assert(row[2] == 180)
	assert(row[3] == 'John Smith')
	assert(row[4] == 'ACGT\0ACGT\1')
end

db:close()