# create table and add plpgsql to curent database
createdb test
createlang plpgsql -d test

create_base_tables.sql
create_security.sql
create_tree_table.sql
create_types.sql

create_log_system.sql

security_func.sql

getTreeList_func.sql

