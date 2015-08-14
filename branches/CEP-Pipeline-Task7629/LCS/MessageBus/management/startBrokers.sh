#!/bin/bash 

# Script for starting and stopping a set of brokers

# mkdir -p ~/.qpidd_broker2 && qpidd -d -t -p 9876 --federation-tag `hostname --fqdn` --data-dir ~/.qpidd_broker2 --log-to-file ~/.qpidd_broker2/qpid.log
# -d daemon
# -t verbose logging
# --data-dir waar staat de 'data' config/persistent stuff
# --log-to-file plaats van log file

# ik zou -t vervangen door --log-enable info+
# dus:
# mkdir -p ~/.qpidd_broker2 && qpidd -d --log-enable info+ -p 9876 --federation-tag `hostname --fqdn` --data-dir ~/.qpidd_broker2 --log-to-file ~/.qpidd_broker2/qpid.log