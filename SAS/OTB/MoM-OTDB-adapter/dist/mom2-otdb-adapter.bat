set classpath=R:\MoM-OTDB-adapter\lib\commons-logging.jar
set classpath=R:\MoM-OTDB-adapter\dist\mom-otdb-adapter.jar;%classpath%
java nl.astron.lofar.odtb.mom2otdbadapter.MomOtdbAdapter -u bastiaan -p bastiaan -rmihost lofar17.astron.nl -rmiport 10099 -mom2url http://localhost:8080/mom2 -authurl http://localhost:8080/wsrtauth