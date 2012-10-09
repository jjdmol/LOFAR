# startController.sh executable taskname parenthostname parentService
#
# start the given executable
#

# start process
($1 $2 $3 $4 1>/dev/null 2>&1 ) &

# get its pid
# echo $!

exit 0
