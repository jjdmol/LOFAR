# $Id: CMakeLists.txt 32341 2015-08-28 11:59:26Z schaap $

set(sql_files add_notifications.sql
              add_resource_allocation_statics.sql
              add_virtual_instrument.sql
              create_database.sql
              create_and_populate_database.sql)

install_files(/share/radb/sql FILES ${sql_files})

