# $Id: CMakeLists.txt 32341 2015-08-28 11:59:26Z schaap $

set(sql_files SetupDatabase.sql)

install_files(/share/ssdb/sql FILES ${sql_files})

