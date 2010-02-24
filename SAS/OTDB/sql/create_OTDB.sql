-- execute this file with:
-- psql -f create_OTDB.sql -h dop50 -U postgres otdbtest
--
\i create_base_tables.sql
\i create_security.sql
\i create_tree_table.sql
\i create_types.sql

\i create_log_system.sql

\i security_func.sql
\i misc_func.sql

-- State history
\i setTreeState_func.sql
\i addTreeState_func.sql
\i getStateList_func.sql

-- OTDBConnection
\i getTreeList_func.sql
\i getTreeInfo_func.sql
\i newTree_func.sql
\i getExecutableTrees_func.sql
\i getTreeGroup_func.sql
\i getTreesInPeriod_func.sql

-- PICtree
\i create_PIC_tables.sql
\i addPICparam_func.sql
\i getPICparamDef_func.sql
\i getPICitemList_func.sql
\i searchPICinPeriod_func.sql

-- Events and Actions
\i create_event_action.sql

-- VICcomponent
\i create_VIC_tables.sql
\i saveVCnode_func.sql
\i saveVICparamDef_func.sql
\i getVICnodeDef_func.sql
\i getVCNodeList_func.sql
\i getVCparams_func.sql
\i buildTemplateTree_func.sql
\i addComponentToVT_func.sql
\i isTopComponent_func.sql
\i deleteVCnode_func.sql

-- VICtemplate
\i hVICsearchParamID_func.sql
\i getVICparamDef_func.sql
\i dupVTnode_func.sql
\i updateVTnode_func.sql
\i deleteVTnode_func.sql
\i getVTitemList_func.sql
\i getVTchildren_func.sql
\i instanciateTree_func.sql

-- VIChierarchy
\i getVHitemList_func.sql
\i exportTree_func.sql
\i searchVHinPeriod_func.sql
\i setSchedule_func.sql

-- multi treetype
\i getTopNode_func.sql
\i getNode_func.sql
\i copyVHtree_func.sql
\i copyVTtree_func.sql
\i copyTree_func.sql
\i deleteTree_func.sql
\i addKVT_func.sql
\i classify_func.sql
\i setMomInfo_func.sql
\i setDescription_func.sql

-- campaign
\i campaignAPI.sql
