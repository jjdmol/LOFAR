nodes="`cat nodes.txt`"
for node in ${nodes} ; do
    ssh ${node} -C "killall -u ${USER}" &
done
sleep 2
