.. _imager_prepare-recipe:

================
imager_prepare
================

***Master Side of the recipe ***


.. autoclass:: lofarpipe.recipes.master.imager_prepare.imager_prepare
	:members: _create_input_map_for_sbgroup, _validate_input_map, go

***Node Side of the recipe***
   
.. autoclass:: lofarpipe.recipes.nodes.imager_prepare.imager_prepare
	:members: _copy_input_files, _run_dppp, _concat_timeslices, _run_rficonsole, _filter_bad_stations, run
