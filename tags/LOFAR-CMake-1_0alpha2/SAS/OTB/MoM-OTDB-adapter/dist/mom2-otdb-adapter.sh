#!/bin/bash
export PATH=/usr/local/jdk1.5.0_04/bin:$PATH
export LD_LIBRARY_PATH=/home/verhoef/sharedlib:/lib:/usr/lib:/usr/local/lib
export CLASSPATH=/home/verhoef/jotdb.jar:$CLASSPATH
export CLASSPATH=/home/verhoef/javalib/astron-http-utils.jar:$CLASSPATH
export CLASSPATH=/home/verhoef/javalib/astron-utils.jar:$CLASSPATH
export CLASSPATH=/home/verhoef/javalib/commons-codec-1.3.jar:$CLASSPATH
export CLASSPATH=/home/verhoef/javalib/commons-httpclient-3.0.jar:$CLASSPATH
export CLASSPATH=/home/verhoef/javalib/commons-logging.jar:$CLASSPATH
export CLASSPATH=/home/verhoef/javalib/commons-validator.jar:$CLASSPATH
export CLASSPATH=/home/verhoef/javalib/log4j.1.2.8.jar:$CLASSPATH
export CLASSPATH=/home/verhoef/javalib/xercesImpl.jar:$CLASSPATH
export CLASSPATH=/home/verhoef/javalib/xml-apis.jar:$CLASSPATH
export CLASSPATH=/home/verhoef/mom-otdb-adapter.jar:$CLASSPATH
java nl.astron.lofar.odtb.mom2otdbadapter.MomOtdbAdapter -u odtbadapter -p otdbadapter -rmihost lofar17.astron.nl -rmiport 10099 -rmiseconds 5 -mom2url http://localhost:8080/mom2 -authurl http://localhost:8080/wsrtauth
