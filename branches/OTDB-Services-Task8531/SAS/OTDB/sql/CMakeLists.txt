# $Id$

lofar_package(OTDB_SQL 1.0)

lofar_add_sbin_scripts(fresh_database)

file(GLOB otdb_sql_files
  *.sql)
install(FILES
  ${otdb_sql_files}
  update_all_functions.sh
  DESTINATION sbin/sql)

