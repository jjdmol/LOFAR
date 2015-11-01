# $Id$

lofar_package(LTAIngest 0.1) #DEPENDS MessageBus)

#by default use LTAINGEST_BUILD_TARGET="test" unless overridden on the command line, or by variants files
set(LTAINGEST_BUILD_TARGET "test" CACHE STRING "Specify for which specific destination host you want to build LTAIngest.")

message(STATUS "Using LTAINGEST_BUILD_TARGET \"${LTAINGEST_BUILD_TARGET}\".")

set(MOM3SCHEMA "lofar_mom3")


#switch with config variables for the various build targets.
if(LTAINGEST_BUILD_TARGET STREQUAL "lexar")
  set(LTAINGEST_LOG_ROOT_DIR "/log/ingest/")
  set(LTAINGEST_DATA_DIR "/data/ingest/")
elseif(LTAINGEST_BUILD_TARGET STREQUAL "lotar")
  set(LTAINGEST_LOG_ROOT_DIR "/home/lofarlocal/log/ingest/")
  set(LTAINGEST_DATA_DIR "/home/lofarlocal/.ingest/")
elseif(LTAINGEST_BUILD_TARGET STREQUAL "test")
  set(LTAINGEST_LOG_ROOT_DIR "/log/ingesttest/")
  set(LTAINGEST_DATA_DIR "/log/ingesttest/")
  set(MOM3SCHEMA "lofartest_sas099_mom3_two")
else()
  message(SEND_ERROR "Unknown LTAINGEST_BUILD_TARGET given: '${LTAINGEST_BUILD_TARGET}'.")
endif()

include(PythonInstall)
set(USE_PYTHON_COMPILATION Off)

python_install(
	ingestpipeline.py
	ClientForm-0.1.17/ClientForm-0.1.17/test.py
	ClientForm-0.1.17/ClientForm-0.1.17/ClientForm.py
	ClientForm-0.1.17/ClientForm-0.1.17/setup.py
	ClientForm-0.1.17/test.py
	ClientForm-0.1.17/ClientForm.py
	ClientForm-0.1.17/setup.py
	ingestpipeline_test.py
	mechanize-0.2.5/examples/hack21.py
	mechanize-0.2.5/examples/pypi.py
	mechanize-0.2.5/examples/forms/simple.py
	mechanize-0.2.5/examples/forms/example.py
	mechanize-0.2.5/mechanize/_debug.py
	mechanize-0.2.5/mechanize/_util.py
	mechanize-0.2.5/mechanize/_lwpcookiejar.py
	mechanize-0.2.5/mechanize/_mechanize.py
	mechanize-0.2.5/mechanize/_sockettimeout.py
	mechanize-0.2.5/mechanize/_sgmllib_copy.py
	mechanize-0.2.5/mechanize/_firefox3cookiejar.py
	mechanize-0.2.5/mechanize/_markupbase.py
	mechanize-0.2.5/mechanize/_urllib2_fork.py
	mechanize-0.2.5/mechanize/_beautifulsoup.py
	mechanize-0.2.5/mechanize/_http.py
	mechanize-0.2.5/mechanize/_version.py
	mechanize-0.2.5/mechanize/_request.py
	mechanize-0.2.5/mechanize/_pullparser.py
	mechanize-0.2.5/mechanize/__init__.py
	mechanize-0.2.5/mechanize/_mozillacookiejar.py
	mechanize-0.2.5/mechanize/_useragent.py
	mechanize-0.2.5/mechanize/_opener.py
	mechanize-0.2.5/mechanize/_rfc3986.py
	mechanize-0.2.5/mechanize/_form.py
	mechanize-0.2.5/mechanize/_auth.py
	mechanize-0.2.5/mechanize/_response.py
	mechanize-0.2.5/mechanize/_clientcookie.py
	mechanize-0.2.5/mechanize/_urllib2.py
	mechanize-0.2.5/mechanize/_gzip.py
	mechanize-0.2.5/mechanize/_msiecookiejar.py
	mechanize-0.2.5/mechanize/_html.py
	mechanize-0.2.5/mechanize/_testcase.py
	mechanize-0.2.5/mechanize/_headersutil.py
	mechanize-0.2.5/test.py
	mechanize-0.2.5/release.py
	mechanize-0.2.5/test/test_headers.py
	mechanize-0.2.5/test/test_browser.py
	mechanize-0.2.5/test/test_html.py
	mechanize-0.2.5/test/test_useragent.py
	mechanize-0.2.5/test/test_pickle.py
	mechanize-0.2.5/test/test_date.py
	mechanize-0.2.5/test/test_performance.py
	mechanize-0.2.5/test/test_api.py
	mechanize-0.2.5/test/test_import.py
	mechanize-0.2.5/test/test_functional.py
	mechanize-0.2.5/test/test_response.py
	mechanize-0.2.5/test/__init__.py
	mechanize-0.2.5/test/test_form.py
	mechanize-0.2.5/test/test_cookies.py
	mechanize-0.2.5/test/test_opener.py
	mechanize-0.2.5/test/test_urllib2.py
	mechanize-0.2.5/test/test_unittest.py
	mechanize-0.2.5/test/test_form_mutation.py
	mechanize-0.2.5/test/test_urllib2_localnet.py
	mechanize-0.2.5/test/test_cookie.py
	mechanize-0.2.5/test/test_pullparser.py
	mechanize-0.2.5/test-tools/doctest.py
	mechanize-0.2.5/test-tools/linecache_copy.py
	mechanize-0.2.5/test-tools/twisted-ftpserver.py
	mechanize-0.2.5/test-tools/testprogram.py
	mechanize-0.2.5/test-tools/functools_copy.py
	mechanize-0.2.5/test-tools/unittest/case.py
	mechanize-0.2.5/test-tools/unittest/loader.py
	mechanize-0.2.5/test-tools/unittest/__main__.py
	mechanize-0.2.5/test-tools/unittest/result.py
	mechanize-0.2.5/test-tools/unittest/suite.py
	mechanize-0.2.5/test-tools/unittest/__init__.py
	mechanize-0.2.5/test-tools/unittest/runner.py
	mechanize-0.2.5/test-tools/unittest/main.py
	mechanize-0.2.5/test-tools/unittest/util.py
	mechanize-0.2.5/test-tools/twisted-localserver.py
	mechanize-0.2.5/ez_setup.py
	mechanize-0.2.5/setup.py
	mechanize/_debug.py
	mechanize/_util.py
	mechanize/_lwpcookiejar.py
	mechanize/_mechanize.py
	mechanize/_sockettimeout.py
	mechanize/_sgmllib_copy.py
	mechanize/_firefox3cookiejar.py
	mechanize/_markupbase.py
	mechanize/_urllib2_fork.py
	mechanize/_beautifulsoup.py
	mechanize/_http.py
	mechanize/_version.py
	mechanize/_request.py
	mechanize/_pullparser.py
	mechanize/__init__.py
	mechanize/_mozillacookiejar.py
	mechanize/_useragent.py
	mechanize/_opener.py
	mechanize/_rfc3986.py
	mechanize/_form.py
	mechanize/_auth.py
	mechanize/_response.py
	mechanize/_clientcookie.py
	mechanize/_urllib2.py
	mechanize/_gzip.py
	mechanize/_msiecookiejar.py
	mechanize/_html.py
	mechanize/_testcase.py
	mechanize/_headersutil.py
	fpconst-0.7.0/fpconst-0.7.0/fpconst.py
	fpconst-0.7.0/fpconst-0.7.0/setup.py
	fpconst-0.7.0/fpconst.py
	fpconst-0.7.0/setup.py
	unspecifiedSIP.py
	h5_check.py
	mom_http.py
	dav/davlib_orig.py
	dav/davlib.py
	dav/httplib.py
	dav/webdav/WebdavRequests.py
	dav/webdav/WebdavClient.py
	dav/webdav/Constants.py
	dav/webdav/Utils.py
	dav/webdav/logger.py
	dav/webdav/__init__.py
	dav/webdav/Condition.py
	dav/webdav/Connection.py
	dav/webdav/VersionHandler.py
	dav/webdav/NameCheck.py
	dav/webdav/WebdavResponse.py
	dav/webdav/acp/GrantDeny.py
	dav/webdav/acp/Ace.py
	dav/webdav/acp/Acl.py
	dav/webdav/acp/Privilege.py
	dav/webdav/acp/__init__.py
	dav/webdav/acp/AceHandler.py
	dav/webdav/acp/Principal.py
	dav/wsrt_webdavlib.py
	dav/davhttplib.py
	dav/qp_xml.py
	job_parser.py
	find_files.py
	multiprocessing/reduction.py
	multiprocessing/process.py
	multiprocessing/queues.py
	multiprocessing/connection.py
	multiprocessing/dummy/connection.py
	multiprocessing/dummy/__init__.py
	multiprocessing/heap.py
	multiprocessing/sharedctypes.py
	multiprocessing/pool.py
	multiprocessing/__init__.py
	multiprocessing/forking.py
	multiprocessing/synchronize.py
	multiprocessing/managers.py
	multiprocessing/util.py
	slave.py
	sitecustomize.py
	SOAPpy-0.12.0/tools/interop2html.py
	SOAPpy-0.12.0/tests/xmethods.py
	SOAPpy-0.12.0/tests/echoClient.py
	SOAPpy-0.12.0/tests/excelTest.py
	SOAPpy-0.12.0/tests/translateTest.py
	SOAPpy-0.12.0/tests/simpleWSDL.py
	SOAPpy-0.12.0/tests/echoServer.py
	SOAPpy-0.12.0/tests/quoteTest.py
	SOAPpy-0.12.0/tests/echoHeader.py
	SOAPpy-0.12.0/tests/TCtest.py
	SOAPpy-0.12.0/tests/weatherTest.py
	SOAPpy-0.12.0/tests/esj_test_client.py
	SOAPpy-0.12.0/tests/BabelfishWSDLTest.py
	SOAPpy-0.12.0/tests/testWSDL.py
	SOAPpy-0.12.0/tests/Bug1001646.py
	SOAPpy-0.12.0/tests/ZeroLengthArray.py
	SOAPpy-0.12.0/tests/testleak.py
	SOAPpy-0.12.0/tests/ComplexTypes.py
	SOAPpy-0.12.0/tests/esj_test_server.py
	SOAPpy-0.12.0/tests/newsTest.py
	SOAPpy-0.12.0/tests/SOAPtest.py
	SOAPpy-0.12.0/tests/largeDataTest.py
	SOAPpy-0.12.0/tests/Bug916265.py
	SOAPpy-0.12.0/tests/cardServer.py
	SOAPpy-0.12.0/tests/storageTest.py
	SOAPpy-0.12.0/tests/alanbushTest.py
	SOAPpy-0.12.0/tests/whoisTest.py
	SOAPpy-0.12.0/tests/GoogleTest.py
	SOAPpy-0.12.0/tests/speedTest.py
	SOAPpy-0.12.0/tests/testClient1.py
	SOAPpy-0.12.0/tests/Bug918216.py
	SOAPpy-0.12.0/tests/cardClient.py
	SOAPpy-0.12.0/bid/inventoryClient.py
	SOAPpy-0.12.0/bid/monitorClient.py
	SOAPpy-0.12.0/bid/inventoryServer.py
	SOAPpy-0.12.0/SOAPpy/Errors.py
	SOAPpy-0.12.0/SOAPpy/Server.py
	SOAPpy-0.12.0/SOAPpy/NS.py
	SOAPpy-0.12.0/SOAPpy/SOAPBuilder.py
	SOAPpy-0.12.0/SOAPpy/Utilities.py
	SOAPpy-0.12.0/SOAPpy/version.py
	SOAPpy-0.12.0/SOAPpy/wstools/Namespaces.py
	SOAPpy-0.12.0/SOAPpy/wstools/Utility.py
	SOAPpy-0.12.0/SOAPpy/wstools/WSDLTools.py
	SOAPpy-0.12.0/SOAPpy/wstools/TimeoutSocket.py
	SOAPpy-0.12.0/SOAPpy/wstools/XMLSchema.py
	SOAPpy-0.12.0/SOAPpy/wstools/logging.py
	SOAPpy-0.12.0/SOAPpy/wstools/UserTuple.py
	SOAPpy-0.12.0/SOAPpy/wstools/__init__.py
	SOAPpy-0.12.0/SOAPpy/wstools/c14n.py
	SOAPpy-0.12.0/SOAPpy/wstools/XMLname.py
	SOAPpy-0.12.0/SOAPpy/GSIServer.py
	SOAPpy-0.12.0/SOAPpy/__init__.py
	SOAPpy-0.12.0/SOAPpy/Client.py
	SOAPpy-0.12.0/SOAPpy/URLopener.py
	SOAPpy-0.12.0/SOAPpy/Types.py
	SOAPpy-0.12.0/SOAPpy/WSDL.py
	SOAPpy-0.12.0/SOAPpy/Parser.py
	SOAPpy-0.12.0/SOAPpy/Config.py
	SOAPpy-0.12.0/SOAPpy/SOAP.py
	SOAPpy-0.12.0/contrib/soap_cli.py
	SOAPpy-0.12.0/contrib/soap_handler.py
	SOAPpy-0.12.0/setup.py
	SOAPpy-0.12.0/validate/silabclient.py
	SOAPpy-0.12.0/validate/soapware.py
	SOAPpy-0.12.0/validate/silabserver.py
	master.py
	user_ingest.py
	do_ltacp.py
	simple_server.py
	__init__.py
	doc/LTA-SIP.xsd #not a py file, but needed by ingest in python ingest lib dir
  DESTINATION LTAIngest)

#deploy startup scripts in same python ingest lib dir
#TODO: should be moved to LOFARROOT/bin. Currently probably not possible yet due to unknown dependencies.
set(STARTUP_SCRIPTS 
      startup.csh
      startup_slave.csh
      startup_target.csh
      startup_target_test.csh
      startup_test.csh)
install(PROGRAMS ${STARTUP_SCRIPTS} DESTINATION ${PYTHON_INSTALL_DIR}/LTAIngest)

#determine LTAINGEST_LOG_ROOT_DIR for various build hosts (used in ingest_config.py.in)
if(NOT DEFINED LTAINGEST_LOG_ROOT_DIR)
  set(LTAINGEST_LOG_ROOT_DIR "/tmp/ingest/")
endif()

#settings dir for ingest
set(LTAINGEST_CFG_DIR "${CMAKE_INSTALL_PREFIX}/etc/ltaingest")

#set various user/passwords from environment
#mail schaap@astron.nl for a shell script which sets up your build environment with these secret variables
if ("$ENV{MOMREADONLY_USER}" STREQUAL "")
  message(WARNING "MOMREADONLY_USER not found in environment. Did you source your build environment file?")
endif()
set(MOMREADONLY_USER $ENV{MOMREADONLY_USER})
set(MOMREADONLY_PASSWORD $ENV{MOMREADONLY_PASSWORD})
set(MOMCLIENT_USERNAME $ENV{MOMCLIENT_USERNAME})
set(MOMCLIENT_PASSWORD $ENV{MOMCLIENT_PASSWORD})
set(LTAURL_USERNAME $ENV{LTAURL_USERNAME})
set(LTAURL_PASSWORD $ENV{LTAURL_PASSWORD}) 
set(LTAURL_TEST_USERNAME $ENV{LTAURL_TEST_USERNAME})
set(LTAURL_TEST_PASSWORD $ENV{LTAURL_TEST_PASSWORD}) 

#configure ingest_config.py.in with the various host and environment specific variables.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/ingest_config.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/ingest_config.py @ONLY)

#include configured ingest_config.py in python install.
python_install(
  ${CMAKE_CURRENT_BINARY_DIR}/ingest_config.py
  DESTINATION LTAIngest)

#configure job_group.py.in with the various host and environment specific variables.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/job_group.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/job_group.py @ONLY)

#include configured job_group.py in python install.
python_install(
  ${CMAKE_CURRENT_BINARY_DIR}/job_group.py
  DESTINATION LTAIngest)
  
#configure ingest_config_test.py.in with the various host and environment specific variables.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/ingest_config_test.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/ingest_config_test.py @ONLY)

#include configured ingest_config_test.py in python install.
python_install(
  ${CMAKE_CURRENT_BINARY_DIR}/ingest_config_test.py
  DESTINATION LTAIngest)

#configure ingest_logrotate.conf.in with @LTAINGEST_LOG_ROOT_DIR@
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/ingest_logrotate.conf.in
  ${CMAKE_CURRENT_BINARY_DIR}/ingest_logrotate.conf @ONLY)

#install ingest_logrotate.conf in LTAINGEST_CFG_DIR
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ingest_logrotate.conf
  DESTINATION ${LTAINGEST_CFG_DIR})

#configure ingest_install_logrotate_crontab.sh.in with @LTAINGEST_CFG_DIR@
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/ingest_install_logrotate_crontab.sh.in
  ${CMAKE_CURRENT_BINARY_DIR}/ingest_install_logrotate_crontab.sh @ONLY)

#install ingest_install_logrotate_crontab.sh in bin
lofar_add_bin_scripts(
  ${CMAKE_CURRENT_BINARY_DIR}/ingest_install_logrotate_crontab.sh)

  
add_subdirectory(test)
  