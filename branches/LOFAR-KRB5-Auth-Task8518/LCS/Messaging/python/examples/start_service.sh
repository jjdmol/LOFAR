#!/bin/bash

export KRB5CCNAME=/tmp/service_123456
export KRB5_KTNAME=/etc/qpidd.keytab

kinit -k -t $KRB5_KTNAME qpidd/robinhood.control.lofar
klist

./service_gssapi

