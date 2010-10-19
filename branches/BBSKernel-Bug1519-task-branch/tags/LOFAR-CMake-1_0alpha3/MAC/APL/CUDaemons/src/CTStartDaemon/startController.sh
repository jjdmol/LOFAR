# startController.sh executable taskname parenthostname parentService
#
# start the given executable
#

# start process
($1 $2 $3 $4 2>&1 | tee $2.log ) & 

# get its pid
# echo $!

exit 0
