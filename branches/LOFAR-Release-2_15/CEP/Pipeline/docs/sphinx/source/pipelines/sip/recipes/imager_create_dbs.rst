.. _imager_create_dbs-recipe:

=================
imager_create_dbs
=================

***Master Side of the recipe ***


.. autoclass:: lofarpipe.recipes.master.imager_create_dbs.imager_create_dbs
	:members: _validate_input_data, _run_create_dbs_node, _collect_and_assign_outputs, go

***Node Side of the recipe***
   
.. autoclass:: lofarpipe.recipes.nodes.imager_create_dbs.imager_create_dbs
	:members: _create_source_list, _create_source_db, _field_of_view, _create_parmdb, _create_parmdb_for_timeslices, _create_monet_db_connection, _get_ra_and_decl_from_ms, _get_soucelist_from_gsm, run

	