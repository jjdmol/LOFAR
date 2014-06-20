.. _gainoutliercorrection-recipe:

========================
gainoutliercorrection
========================

***Master Side of the recipe ***

.. autoclass:: lofarpipe.recipes.master.gainoutliercorrection.gainoutliercorrection

***Node Side of the recipe ***

.. autoclass:: lofarpipe.recipes.nodes.gainoutliercorrection.gainoutliercorrection
	:members: _filter_stations_parmdb, _read_polarisation_data_and_type_from_db, _convert_data_to_ComplexArray,	_swap_outliers_with_median, _write_corrected_data
