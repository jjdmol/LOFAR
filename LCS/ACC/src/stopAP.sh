echo -n "Killing process "; cat $1.pid
kill -9 `cat $1.pid`
rm -f $1.pid $1.ps
