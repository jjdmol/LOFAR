-- execute this file with:
-- psql -f create_OTDB.sql -h dop50 -U postgres otdbtest
--
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
\i getTreeInfo_func.sql
\i newTree_func.sql

-- PICadmin
\i create_PIC_tables.sql
\i hPICsearchParamID_func.sql
\i addPICparam_func.sql
\i getPICitemList_func.sql
\i addKVT_func.sql
\i searchInPeriod_func.sql

-- Events and Actions
\i create_event_action.sql

-- VICadmin
\i create_VIC_tables.sql
\i dupVTnode_func.sql
\i updateVTnode_func.sql
\i deleteVTnode_func.sql
\i getVTnode_func.sql
\i getVTchildren_func.sql
\i instanciateTree_func.sql
\i exportTree_func.sql
