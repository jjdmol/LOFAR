-- create table and add plpgsql to curent database
-- The follwing two command must be given on the prompt of a shell.
-- createdb -h dop50 -U postgres otdbtest
-- createlang -h dop50 -U postgres plpgsql otdbtest

-- The rest of the file can be executed with:
-- psql -f create_database.sh -h dop50 -U postgres otdbtest
\i create_base_tables.sql
\i create_security.sql
\i create_tree_table.sql
\i create_types.sql

\i create_log_system.sql

\i security_func.sql
\i classify_func.sql

\i misc_func.sql

-- OTDBConnection
\i getTreeList_func.sql
\i newTree_func.sql

-- PICadmin
\i create_PIC_tables.sql
\i hPICsearchParamID_func.sql
\i addPICparam_func.sql
\i getItemList_func.sql
\i addKVT_func.sql
\i searchInPeriod_func.sql

-- Events and Actions
\i create_event_action.sql
