set classpath=R:\MoM-OTDB-adapter\lib\astron-http-utils.jar;
set classpath=R:\MoM-OTDB-adapter\lib\astron-utils.jar;%classpath%
set classpath=R:\MoM-OTDB-adapter\lib\commons-codec-1.3.jar;%classpath%
set classpath=R:\MoM-OTDB-adapter\lib\commons-httpclient-3.0.jar;%classpath%
set classpath=R:\MoM-OTDB-adapter\lib\commons-logging.jar;%classpath%
set classpath=R:\MoM-OTDB-adapter\lib\commons-validator.jar;%classpath%
set classpath=R:\MoM-OTDB-adapter\lib\log4j.1.2.8.jar;%classpath%
set classpath=R:\MoM-OTDB-adapter\lib\xercesImpl.jar;%classpath%
set classpath=R:\MoM-OTDB-adapter\lib\xml-apis.jar;%classpath%
set classpath=R:\MoM-OTDB-adapter\sharedlib\jotdb.jar;%classpath%
set classpath=R:\MoM-OTDB-adapter\dist\mom-otdb-adapter.jar;%classpath%
java nl.astron.lofar.odtb.mom2otdbadapter.MomOtdbAdapter -u bastiaan -p bastiaan -rmihost lofar17.astron.nl -rmiport 10099 -mom2url http://localhost:8080/mom2 -authurl http://localhost:8080/wsrtauth