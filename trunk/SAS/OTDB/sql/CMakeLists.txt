# $Id$

lofar_package(OTDB_SQL 0.1)
install(PROGRAMS
  fresh_database
DESTINATION sbin)

file(GLOB otdb_sql_files
  *.sql)
install(FILES
  ${otdb_sql_files}
  DESTINATION sbin/sql)

