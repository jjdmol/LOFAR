createTunnel() {
    /usr/bin/ssh -f -N -L2010:lexar002:2010 -L2022:lexar002:22 momingest@portal.lofar.eu
    if [[ $? -eq 0 ]]; then
        echo Tunnel to lexar002 created successfully
    else
        echo An error occurred creating a tunnel to lexar002 RC was $?
    fi
}
## Run the 'ls' command remotely.  If it returns non-zero, then create a new connection
/usr/bin/ssh -p 2022 momingest@localhost ls
if [[ $? -ne 0 ]]; then
    echo Creating new tunnel connection
    createTunnel
fi
