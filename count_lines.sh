#!/bin/bash

find . -type f |
egrep -v '/(\.git)/' |
egrep '\.(h|c)$' |
egrep -v '/(gonzo|repl)\.(c)$' |
egrep -v '/(siod|tinyscheme.*|libtommath.*)/' |
while read f
do
    echo -n "$f "
    cat $f |
    sed -e 's+//.*++g' |
    egrep -v '^[ \t]*$' |
    wc -l
done |
awk '
{
    t += $2;
    printf("%8d %s\n", $2, $1);
}
END {
    printf("%8d %s\n", t, "TOTAL");
}'

