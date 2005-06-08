# create table and add plpgsql to curent database
createdb otdbtest
createlang plpgsql -d testtest

create_base_tables.sql
create_security.sql
create_tree_table.sql
create_types.sql

create_log_system.sql

security_func.sql
classify_func.sql

misc_func.sql

# OTDBConnection
getTreeList_func.sql
newTree_func.sql

# PICadmin
create_PIC_tables.sql
hPICsearchParamID_func.sql
addPICparam_func.sql
getItemList_func.sql
addKVT_func.sql
searchInPeriod_func.sql

# Events and Actions
create_event_action.sql
