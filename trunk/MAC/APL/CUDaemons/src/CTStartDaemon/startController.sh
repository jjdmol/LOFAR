# startController.sh executable taskname parenthostname parentService
#
# start the given executable
#

# start process
($1 $2 $3 $4 1 2>&1 | awk '{ print strftime("%Y-%m-%d %H:%M:%S"),$0; }' >>"/opt/lofar/var/log/$2.stdout") &

# get its pid
# echo $!

exit 0
