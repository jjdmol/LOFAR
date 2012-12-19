.. module:: lofarpipe.cuisine
   :synopsis: The Cuisine system.

************************************
The :mod:`lofarpipe.cuisine` package
************************************

The LOFAR pipeline system developed partly from the `WSRT Cuisine
<http://www.astron.nl/~renting/pipeline_frame.html>`_, developed by Adriaan
Renting for use at the Westerbork Synthesis Radio Telescope. 

A small part of this cuisine framework is currently part of the lofar pipeline
framework, mainly needed for running a recipe in stand alone mode: 
Some basic parset functionality and ingredient behavior. The largest 
part of the cuisine has been superseded by the functionality in pipeline/support 

Very little of the original Cuisine code is currently used in the LOFAR
framework, although the :class:`lofarpipe.cuisine.WSRTrecipe.WSRTrecipe` is
still used as the basis for all LOFAR recipes. The recipe author, however, is
*never* expected to directly interact with Cuisine code: all LOFAR pipeline
recipes inherit from :class:`lofarpipe.support.baserecipe.BaseRecipe`, which
entirely wraps all relevant Cuisine functionality. The
:mod:`lofarpipe.support` inheritance diagrams show exactly how these packages
are related. The following API documentation covers only those routines
directly used by the rest of the pipeline system, not Cuisine as a whole.

.. module:: lofarpipe.cuisine.WSRTrecipe
   :synopsis: Base module for all Cuisine recipe functionality.

:mod:`lofarpipe.cuisine.WSRTrecipe`
-----------------------------------

.. autoclass:: lofarpipe.cuisine.WSRTrecipe.WSRTrecipe
   :members: help, run,  prepare_run, finalize_run, main, go, cook_recipe

.. module:: lofarpipe.cuisine.cook
   :synopsis: Cuisine cooks.

:mod:`lofarpipe.cuisine.cook`
-----------------------------

.. autoclass:: lofarpipe.cuisine.cook.PipelineCook
   :members: copy_inputs, copy_outputs, spawn, try_running
