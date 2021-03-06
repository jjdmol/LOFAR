#!/bin/bash

{
    timeout=$1 # in seconds
    interval=1
    delay=1
    shift ${argv}
    (
	((t = timeout))

	while ((t > 0)); do
	    sleep $interval
	    kill -0 $$ || exit 0
	    ((t -= interval))
	done

        # Be nice, post SIGTERM first.
        # The 'exit 0' below will be executed if any preceeding command fails.
	kill -s SIGTERM $$ && kill -0 $$ || exit 0
	sleep $delay
	kill -s SIGKILL $$
    ) 2> /dev/null &
    exec "$@"
}
