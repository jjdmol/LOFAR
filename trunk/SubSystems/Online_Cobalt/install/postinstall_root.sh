#!/bin/bash -eu

echo "Removing system-supplied /opt/casacore..."
rm -rf /opt/casacore

echo "Giving /opt to lofarbuild..."
chown lofarbuild.lofarbuild /opt
