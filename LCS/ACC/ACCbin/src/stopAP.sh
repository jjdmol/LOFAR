echo -n "Killing process "; cat $1
kill -9 `cat $1`
rm -f $1
